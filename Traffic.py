

import numpy as np
from os import listdir
from re import split
from helper import softmax
import random


def natural_key(string_):
    return [int(s) if s.isdigit() else s for s in split(r'(\d+)', string_)]


class Traffic():

    def __init__(self, nodes_num, type, capacity):
        self.nodes_num = nodes_num
        self.prev_traffic = None
        self.type = type
        self.base_capacity = capacity
        self.capacity = self._get_scaled_capacity(capacity, nodes_num)
        self.dictionary = {}

        self.dictionary['UNI'] = self.uniform_traffic

        if self.type.startswith('DIR:'):
            self.dir = sorted(listdir(self.type.split('DIR:')[-1]), key=lambda x: natural_key((x)))
        self.static = None

    def _get_scaled_capacity(self, base_capacity, nodes_num):
        scale_factors = {
            20: 1,
            40: 1,
            60: 1,
            80: 1,
            100: 1
        }

        if nodes_num in scale_factors:
            return base_capacity * scale_factors[nodes_num]
        else:

            known_sizes = sorted(scale_factors.keys())
            if nodes_num <= known_sizes[0]:
                return base_capacity * scale_factors[known_sizes[0]]
            elif nodes_num >= known_sizes[-1]:
                return base_capacity * scale_factors[known_sizes[-1]]
            else:
                for i in range(len(known_sizes) - 1):
                    if known_sizes[i] <= nodes_num <= known_sizes[i + 1]:
                        low_size, high_size = known_sizes[i], known_sizes[i + 1]
                        low_scale, high_scale = scale_factors[low_size], scale_factors[high_size]
                        # 线性插值
                        ratio = (nodes_num - low_size) / (high_size - low_size)
                        scale = low_scale + ratio * (high_scale - low_scale)
                        return base_capacity * scale

        return base_capacity


        t = np.random.uniform(0.8 * self.capacity * 1, 0.9 * self.capacity * 1)
        return np.asarray(t * softmax(np.random.uniform(0, 1, size=[self.nodes_num] * 2))).clip(min=0.00001)

    def generate_scaled_traffic(self):

        traffic_matrix = np.full((self.nodes_num, self.nodes_num), 0.001)
        np.fill_diagonal(traffic_matrix, -1)

        grid_configs = {
            20: (4, 5),
            40: (5, 8),
            60: (6, 10),
            80: (8, 10),
            100: (10, 10)
        }

        if self.nodes_num in grid_configs:
            rows, cols = grid_configs[self.nodes_num]
        else:

            rows = int(np.sqrt(self.nodes_num))
            cols = self.nodes_num // rows

        HIGH_TRAFFIC = 0.6 * self.capacity
        MEDIUM_TRAFFIC = 0.3 * self.capacity
        LOW_TRAFFIC = 0.1 * self.capacity
        VERY_LOW_TRAFFIC = 0.02 * self.capacity



        center_row_start = rows // 3
        center_row_end = 2 * rows // 3
        center_col_start = cols // 3
        center_col_end = 2 * cols // 3

        center_nodes = []
        for i in range(center_row_start, center_row_end + 1):
            for j in range(center_col_start, center_col_end + 1):
                node = i * cols + j
                if node < self.nodes_num:
                    center_nodes.append(node)


        edge_nodes = []

        for j in range(cols):
            if 0 * cols + j < self.nodes_num:
                edge_nodes.append(0 * cols + j)
            if (rows - 1) * cols + j < self.nodes_num:
                edge_nodes.append((rows - 1) * cols + j)

        for i in range(1, rows - 1):
            if i * cols + 0 < self.nodes_num:
                edge_nodes.append(i * cols + 0)
            if i * cols + (cols - 1) < self.nodes_num:
                edge_nodes.append(i * cols + (cols - 1))


        key_corners = [
            (0, self.nodes_num - 1),
            (cols - 1, (rows - 1) * cols)
        ]

        for src, dst in key_corners:
            if src < self.nodes_num and dst < self.nodes_num:
                traffic_matrix[src, dst] = traffic_matrix[dst, src] = HIGH_TRAFFIC


        center_high_flows = max(2, len(center_nodes) // 5)
        center_pairs = []


        if len(center_nodes) >= 2:
            for _ in range(center_high_flows):
                src, dst = np.random.choice(center_nodes, 2, replace=False)
                center_pairs.append((src, dst))

            for src, dst in center_pairs:
                traffic_matrix[src, dst] = traffic_matrix[dst, src] = np.random.uniform(
                    MEDIUM_TRAFFIC, HIGH_TRAFFIC)


        vertical_flow_count = max(2, rows // 2)

        top_row_nodes = list(range(cols))
        bottom_row_nodes = list(range((rows - 1) * cols, min(rows * cols, self.nodes_num)))

        for _ in range(vertical_flow_count):
            if top_row_nodes and bottom_row_nodes:
                src = np.random.choice(top_row_nodes)
                dst = np.random.choice(bottom_row_nodes)
                traffic_matrix[src, dst] = traffic_matrix[dst, src] = np.random.uniform(
                    MEDIUM_TRAFFIC, HIGH_TRAFFIC)


        horizontal_flow_count = max(2, cols // 2)

        left_col_nodes = [i * cols for i in range(rows) if i * cols < self.nodes_num]
        right_col_nodes = [min(i * cols + (cols - 1), self.nodes_num - 1) for i in range(rows)]

        for _ in range(horizontal_flow_count):
            if left_col_nodes and right_col_nodes:
                src = np.random.choice(left_col_nodes)
                dst = np.random.choice(right_col_nodes)
                traffic_matrix[src, dst] = traffic_matrix[dst, src] = np.random.uniform(
                    MEDIUM_TRAFFIC, HIGH_TRAFFIC)

        if self.nodes_num <= 40:
            num_bottlenecks = 2
        else:
            num_bottlenecks = min(4, max(3, len(center_nodes) // 6))

        if center_nodes and len(center_nodes) >= num_bottlenecks:
            bottlenecks = np.random.choice(center_nodes, num_bottlenecks, replace=False)

            for bottleneck in bottlenecks:
                num_inputs = min(6, max(3, self.nodes_num // 10))
                all_nodes = [n for n in range(self.nodes_num) if n != bottleneck]

                if len(all_nodes) >= num_inputs:
                    sources = np.random.choice(all_nodes, num_inputs, replace=False)
                    for src in sources:
                        traffic_matrix[src, bottleneck] = traffic_matrix[bottleneck, src] = np.random.uniform(
                            MEDIUM_TRAFFIC, HIGH_TRAFFIC)


        for i in range(len(edge_nodes)):
            for j in range(i + 1, min(i + 10, len(edge_nodes))):
                src, dst = edge_nodes[i], edge_nodes[j]
                if traffic_matrix[src, dst] <= 0.001:
                    traffic_matrix[src, dst] = traffic_matrix[dst, src] = np.random.uniform(
                        0.01, LOW_TRAFFIC)

        remaining_pairs = 0
        for i in range(self.nodes_num):
            for j in range(i + 1, self.nodes_num):
                if traffic_matrix[i, j] <= 0.001:
                    remaining_pairs += 1

        background_flows = min(remaining_pairs, self.nodes_num * 3)

        for _ in range(background_flows):
            i, j = np.random.randint(0, self.nodes_num, 2)
            if i != j and traffic_matrix[i, j] <= 0.001:
                traffic_matrix[i, j] = traffic_matrix[j, i] = np.random.uniform(
                    0.001, VERY_LOW_TRAFFIC)

        if hasattr(self, 'topo_matrix'):
            traffic_matrix = traffic_matrix * self.topo_matrix
            np.fill_diagonal(traffic_matrix, -1)

        return traffic_matrix

    def generate(self):
        if self.type in self.dictionary:
            return self.dictionary[self.type]()
        else:
            return self.generate_scaled_traffic()