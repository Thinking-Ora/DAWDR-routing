// ========================================================================
// NodeQueue.cc -- 【CPU性能优化版】
// ========================================================================

#include "NodeQueue.h"
#include <fstream>
#include <string>
#include <cstdio>

Define_Module(NodeQueue);

struct PacketData {
    int bitLength;
    int ttl;
    int srcNode;
    int dstNode;
    double lastTS;
    int lastRouter;
    int l2, l3, l4;
    int lastQueue;
    int q2, q3, q4, q5;
    double t2, t3, t4, t5;
    int routing;
};

NodeQueue::NodeQueue() {
    endTxMsg = nullptr;
}

NodeQueue::~NodeQueue() {
    cancelAndDelete(endTxMsg);
    if (portQueue.empty()) {
        return;
    }
    std::string filename = folderName + "/QueueData-" + std::to_string(this->getId()) + ".bin";
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
//        EV << "Cannot open file for writing: " << filename << endl;
        while (!portQueue.empty()) {
             cMessage *msg = portQueue.front();
             portQueue.pop();
             delete msg;
        }
        return;
    }
    while (!portQueue.empty()) {
        cMessage* msg = portQueue.front();
        portQueue.pop();
        DataPacket* dataPacket = dynamic_cast<DataPacket*>(msg);
        if (dataPacket) {
            PacketData data_to_write;
            data_to_write.bitLength = dataPacket->getBitLength();
            data_to_write.ttl = dataPacket->getTtl();
            data_to_write.srcNode = dataPacket->getSrcNode();
            data_to_write.dstNode = dataPacket->getDstNode();
            data_to_write.lastTS = dataPacket->getLastTS();
            data_to_write.lastRouter = dataPacket->getLastRouter();
            data_to_write.l2 = dataPacket->getL2();
            data_to_write.l3 = dataPacket->getL3();
            data_to_write.l4 = dataPacket->getL4();
            data_to_write.lastQueue = dataPacket->getLastQueue();
            data_to_write.q2 = dataPacket->getQ2();
            data_to_write.q3 = dataPacket->getQ3();
            data_to_write.q4 = dataPacket->getQ4();
            data_to_write.q5 = dataPacket->getQ5();
            data_to_write.t2 = dataPacket->getT2();
            data_to_write.t3 = dataPacket->getT3();
            data_to_write.t4 = dataPacket->getT4();
            data_to_write.t5 = dataPacket->getT5();
            data_to_write.routing = dataPacket->getRouting();
            outFile.write(reinterpret_cast<const char*>(&data_to_write), sizeof(PacketData));
        }
        delete msg;
    }
    outFile.close();
}

// <-- 修改: 在 initialize() 中一次性计算并缓存ID
void NodeQueue::initialize() {
    deleted = 0;
    endTxMsg = new cMessage("endTxMsg");
    folderName = par("folderName").stdstringValue();
    loadQueueFromFile();
    if (!portQueue.empty()) {
        scheduleAt(simTime(), endTxMsg);
    }

    // <-- 新增代码块: 在初始化时计算一次ID并存入成员变量
    string fullPath = this->getFullPath();
    size_t nodePos = fullPath.find("node") + 4;
    size_t queuePos = fullPath.find("queue[") + 6;
    size_t queueEndPos = fullPath.find("]", queuePos);

    // 使用 try-catch 块增加代码健壮性
    try {
        myNodeID = std::stoi(fullPath.substr(nodePos, fullPath.find('.', nodePos) - nodePos));
        myQueueID = std::stoi(fullPath.substr(queuePos, queueEndPos - queuePos));
    } catch (const std::exception& e) {
//        EV << "Error parsing NodeID/QueueID for " << fullPath << ": " << e.what() << endl;
        myNodeID = -1; // 设置为无效值
        myQueueID = -1;
    }
}

void NodeQueue::loadQueueFromFile() {
    std::string filename = folderName + "/QueueData-" + std::to_string(this->getId()) + ".bin";
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile.is_open()) {
        return;
    }
    PacketData data_read;
    while (inFile.read(reinterpret_cast<char*>(&data_read), sizeof(PacketData))) {
        DataPacket* packet = new DataPacket("dataPacketFromFile");
        packet->setBitLength(data_read.bitLength);
        packet->setTtl(data_read.ttl);
        packet->setSrcNode(data_read.srcNode);
        packet->setDstNode(data_read.dstNode);
        packet->setLastTS(data_read.lastTS);
        packet->setLastRouter(data_read.lastRouter);
        packet->setL2(data_read.l2);
        packet->setL3(data_read.l3);
        packet->setL4(data_read.l4);
        packet->setLastQueue(data_read.lastQueue);
        packet->setQ2(data_read.q2);
        packet->setQ3(data_read.q3);
        packet->setQ4(data_read.q4);
        packet->setQ5(data_read.q5);
        packet->setT2(data_read.t2);
        packet->setT3(data_read.t3);
        packet->setT4(data_read.t4);
        packet->setT5(data_read.t5);
        packet->setRouting(data_read.routing);
        portQueue.push(packet);
    }
    inFile.close();
    std::remove(filename.c_str());
}

// <-- 修改: 在 handleMessage() 中使用缓存的ID
void NodeQueue::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        cMessage *packet = portQueue.front();
        portQueue.pop();
        send(packet, "line$o");
        cModule *parent = getParentModule();
        int currentMaxQueueSize = parent->par("maxQueueSize").longValue();
        parent->par("maxQueueSize").setLongValue(currentMaxQueueSize + 1);
//        ev << "maxQueueSize " << this->getFullPath() << ": " << (currentMaxQueueSize + 1) << endl;

        if (not portQueue.empty()) {
            cChannel *txChannel = gate("line$o")->getTransmissionChannel();
            simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
            scheduleAt(txFinishTime, endTxMsg);
        }
//        ev << "QUEUE INFO  " << this->getFullPath() << "-->  Queue elements: " << portQueue.size() << endl;
    }
    else if (msg->arrivedOn("in")) {
        cChannel *txChannel = gate("line$o")->getTransmissionChannel();
        simtime_t txFinishTime = txChannel->getTransmissionFinishTime();

        if (txFinishTime <= simTime()) {
            send(msg, "line$o");
        }
        else {
            cModule *parent = getParentModule();
            int currentMaxQueueSize = parent->par("maxQueueSize").longValue();

            if (portQueue.empty())
                scheduleAt(txFinishTime, endTxMsg);
            if (portQueue.size() < static_cast<size_t>(currentMaxQueueSize)) {
                portQueue.push(msg);
                if (currentMaxQueueSize > 1){
                    parent->par("maxQueueSize").setLongValue(currentMaxQueueSize - 1);
//                    ev << "maxQueueSize " << this->getFullPath() << ": " << (currentMaxQueueSize - 1) << endl;
                }
            }
            else {
                deleted++;
                DataPacket *data = check_and_cast<DataPacket *>(msg);
                Statistic::instance()->setLost(simTime(), data->getSrcNode(), data->getDstNode());
                delete msg;
            }
        }
//        ev << "QUEUE INFO  " << this->getFullPath() << "-->  Queue elements: " << portQueue.size() << endl;

        // <-- 【关键修改】: 删除昂贵的字符串解析，直接使用缓存的成员变量
        Statistic::instance()->logQueueState(myNodeID, myQueueID, portQueue.size());

    }
    else {
        send(msg,"out");
    }
}