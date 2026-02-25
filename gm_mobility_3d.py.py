import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import rcParams


class GM_Simulator:
    def __init__(self, bounds, params):
        self.bounds = bounds
        self.params = params
        self.center = np.array([(bounds[0] + bounds[1]) / 2,
                                (bounds[2] + bounds[3]) / 2,
                                (bounds[4] + bounds[5]) / 2])
        self.dt = params['dt']
        self.alpha = params['alpha']
        self.buffer = params['buffer_dist']

    def init_state(self, start_pos):
        state = {
            'pos': np.array(start_pos, dtype=float),
            's': self.params['mean_speed'],
            'theta': np.random.uniform(-np.pi, np.pi),
            'phi': 0.0
        }
        return state

    def get_noise(self):
        coeff = np.sqrt(1 - self.alpha ** 2)
        ws = np.random.normal(0, self.params['std_speed']) * coeff
        wt = np.random.normal(0, self.params['std_angle']) * coeff
        wp = np.random.normal(0, self.params['std_angle']) * coeff
        return ws, wt, wp

    def angle_diff(self, target, current):
        diff = target - current
        return (diff + np.pi) % (2 * np.pi) - np.pi

    def step_improved(self, state):
        pos = state['pos']
        d_x = min(pos[0] - self.bounds[0], self.bounds[1] - pos[0])
        d_y = min(pos[1] - self.bounds[2], self.bounds[3] - pos[1])
        d_z = min(pos[2] - self.bounds[4], self.bounds[5] - pos[2])
        dist = min(d_x, d_y, d_z)

        target_theta_mean = state['theta']
        target_phi_mean = state['phi']

        if dist < self.buffer:
            weight = (1 - dist / self.buffer) ** 2
            vec = self.center - pos
            center_theta = np.arctan2(vec[1], vec[0])
            xy_dist = np.linalg.norm(vec[:2])
            center_phi = np.arctan2(vec[2], xy_dist)

            target_theta_mean += weight * self.angle_diff(center_theta, state['theta'])
            target_phi_mean += weight * (center_phi - state['phi'])

        ws, wt, wp = self.get_noise()
        state['s'] = self.alpha * state['s'] + (1 - self.alpha) * self.params['mean_speed'] + ws

        theta_drift = (1 - self.alpha) * self.angle_diff(target_theta_mean, state['theta'])
        state['theta'] += theta_drift + wt

        phi_drift = (1 - self.alpha) * (target_phi_mean - state['phi'])
        state['phi'] += phi_drift + wp
        state['phi'] = np.clip(state['phi'], -1.4, 1.4)

        vx = state['s'] * np.cos(state['phi']) * np.cos(state['theta'])
        vy = state['s'] * np.cos(state['phi']) * np.sin(state['theta'])
        vz = state['s'] * np.sin(state['phi'])

        state['pos'] += np.array([vx, vy, vz]) * self.dt

        state['pos'][0] = np.clip(state['pos'][0], self.bounds[0], self.bounds[1])
        state['pos'][1] = np.clip(state['pos'][1], self.bounds[2], self.bounds[3])
        state['pos'][2] = np.clip(state['pos'][2], self.bounds[4], self.bounds[5])

        return state['pos'].copy()



    def step_traditional(self, state):
        ws, wt, wp = self.get_noise()
        state['s'] = self.alpha * state['s'] + (1 - self.alpha) * self.params['mean_speed'] + ws
        state['theta'] += wt
        target_phi = 0.0  # 期望是平飞
        phi_drift = (1 - self.alpha) * (target_phi - state['phi'])
        state['phi'] = state['phi'] + phi_drift + w
        state['phi'] = np.clip(state['phi'], -0.5, 0.5)

        vx = state['s'] * np.cos(state['phi']) * np.cos(state['theta'])
        vy = state['s'] * np.cos(state['phi']) * np.sin(state['theta'])
        vz = state['s'] * np.sin(state['phi'])

        next_pos = state['pos'] + np.array([vx, vy, vz]) * self.dt

        hit = False

        if next_pos[0] < self.bounds[0]:
            next_pos[0] = self.bounds[0] + (self.bounds[0] - next_pos[0])
            vx = -vx
            hit = True
        elif next_pos[0] > self.bounds[1]:
            next_pos[0] = self.bounds[1] - (next_pos[0] - self.bounds[1])
            vx = -vx
            hit = True

        if next_pos[1] < self.bounds[2]:
            next_pos[1] = self.bounds[2] + (self.bounds[2] - next_pos[1])
            vy = -vy
            hit = True
        elif next_pos[1] > self.bounds[3]:
            next_pos[1] = self.bounds[3] - (next_pos[1] - self.bounds[3])
            vy = -vy
            hit = True

        if next_pos[2] < self.bounds[4]:
            next_pos[2] = self.bounds[4] + (self.bounds[4] - next_pos[2])
            vz = -vz
            hit = True
        elif next_pos[2] > self.bounds[5]:
            next_pos[2] = self.bounds[5] - (next_pos[2] - self.bounds[5])
            vz = -vz
            hit = True

        if hit:
            xy_dist = np.sqrt(vx ** 2 + vy ** 2)
            state['theta'] = np.arctan2(vy, vx)
            state['phi'] = np.arctan2(vz, xy_dist)

        state['pos'] = next_pos
        return state['pos'].copy()



def plot_cube(ax, b, color='gray', alpha=0.1):
    x = [b[0] / 1000, b[1] / 1000]
    y = [b[2] / 1000, b[3] / 1000]
    z = [b[4], b[5]]
    for z_plane in z:
        ax.plot([x[0], x[1], x[1], x[0], x[0]],
                [y[0], y[0], y[1], y[1], y[0]],
                [z_plane] * 5, color=color, alpha=alpha, linewidth=linewidth_axis)
    for i in x:
        for j in y:
            ax.plot([i, i], [j, j], z, color=color, alpha=alpha, linewidth=linewidth_axis)


def draw_buffer_box(ax, b, buff):
    x = [(b[0] + buff) / 1000, (b[1] - buff) / 1000]
    y = [(b[2] + buff) / 1000, (b[3] - buff) / 1000]
    z = [b[4], b[5]]
    for z_plane in z:
        ax.plot([x[0], x[1], x[1], x[0], x[0]],
                [y[0], y[0], y[1], y[1], y[0]],
                [z_plane] * 5, color='green', alpha=0.3, linestyle='--', linewidth=linewidth_axis)


def draw_single_plot(traj, bounds, filename, buffer_dist=None):
    fig = plt.figure(figsize=(fig_width, fig_height))
    fig.subplots_adjust(left=0.25, right=0.95, bottom=0.05, top=0.90)

    ax = fig.add_subplot(111, projection='3d')

    plot_x = traj[:, 0] / 1000.0
    plot_y = traj[:, 1] / 1000.0
    plot_z = traj[:, 2]

    line, = ax.plot(plot_x, plot_y, plot_z, color='#00008B', alpha=0.9, linewidth=linewidth_traj, label='Trajectory')
    start = ax.scatter(plot_x[0], plot_y[0], plot_z[0], c='green', marker='*', s=marker_size * 2.5,
                       label='Initial point')
    end = ax.scatter(plot_x[-1], plot_y[-1], plot_z[-1], c='orange', marker='o', s=marker_size * 2, label='Terminus')

    plot_cube(ax, bounds, color='gray', alpha=0.1)
    if buffer_dist:
        draw_buffer_box(ax, bounds, buffer_dist)

    pad_val = 3.5 * scaling
    ax.set_xlabel('X/km', fontsize=fontsize_label, labelpad=pad_val)
    ax.set_ylabel('Y/km', fontsize=fontsize_label, labelpad=pad_val)

    ax.set_zlabel('', fontsize=fontsize_label)

    ax.text2D(-0.1, 0.78, 'Z/m', transform=ax.transAxes,
              fontsize=fontsize_label, fontweight='normal', family='serif')

    # 刻度设置
    ax.tick_params(axis='both', which='major', labelsize=fontsize_tick, pad=-0.4 * scaling)
    ax.set_xticks(np.arange(0, 5, 1))
    ax.set_yticks(np.arange(0, 5, 1))
    ax.set_zticks([0, 60, 120])

    ax.legend(handles=[line, start, end],
              fontsize=fontsize_legend,
              loc='lower center',
              bbox_to_anchor=(0.5, 0.92),
              ncol=1,  # 改为横排更节省垂直空间
              edgecolor='black', framealpha=1.0, facecolor='white',
              handletextpad=0.3, borderpad=0.4, columnspacing=1.0)

    ax.set_xlim(0, 4)
    ax.set_ylim(0, 4)
    ax.set_zlim(0, bounds[5])
    ax.set_box_aspect((4, 4, 2.5))
    ax.view_init(elev=28, azim=-130)

    ax.xaxis.pane.fill = ax.yaxis.pane.fill = ax.zaxis.pane.fill = False

    plt.savefig(filename, format='png', dpi=1200, bbox_inches='tight', pad_inches=0.02)
    plt.show()


if __name__ == "__main__":
    bounds = [0, 4000, 0, 4000, 0, 120]

    params = {
        'alpha': 0.90,
        'mean_speed': 80.0,
        'std_speed': 5.0,
        'std_angle': 0.05,
        'dt': 3,
        'buffer_dist': 400.0
    }

    sim = GM_Simulator(bounds, params)
    steps = 100
    start_pos = [3500, 3500, 60]

    state_trad = sim.init_state(start_pos)
    state_trad['theta'] = np.pi / 4
    traj_trad = [state_trad['pos'].copy()]
    for _ in range(steps):
        traj_trad.append(sim.step_traditional(state_trad))
    traj_trad = np.array(traj_trad)

    draw_single_plot(traj_trad, bounds, "traditional_gm_final.png")

    state_imp = sim.init_state(start_pos)
    state_imp['theta'] = np.pi / 4
    traj_imp = [state_imp['pos'].copy()]
    for _ in range(steps):
        traj_imp.append(sim.step_improved(state_imp))
    traj_imp = np.array(traj_imp)

    draw_single_plot(traj_imp, bounds, "improved_gm_final.png", buffer_dist=params['buffer_dist'])