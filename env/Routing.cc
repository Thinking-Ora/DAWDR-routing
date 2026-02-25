#include "Routing.h"


Define_Module(Routing);


Routing::Routing() {
}

Routing::~Routing() {
}

// -12, number of nodes
void Routing::initialize()
{
    numPorts = gateSize("in");
    id = par("id");
    numTx = par("numTx");
    numNodes = par("numNodes");
    folderName = par("folderName").stdstringValue();

    outPort.resize(numNodes);

    int diff = numNodes - numTx;
//    getRoutingInfo(id-diff, outPort);
    getRoutingInfo(id-diff, &outPort[0]);

    Statistic::instance()->setNumNodes(numNodes);
    Statistic::instance()->setNumTx(numTx);
    Statistic::instance()->setFolder(folderName);
}

void Routing::handleMessage(cMessage *msg)
{
//    if (isPaused)
//    {
//        // 仿真暂停时，不处理消息，重新调度事件
//        scheduleAt(simTime() + 5, msg);
//        return;
//    }

    DataPacket *data = check_and_cast<DataPacket *>(msg);

    if (id == data->getDstNode()) {
//        ev << this->getFullPath() << "  Message received" << endl;
        simtime_t delayPaquet= simTime() - data->getCreationTime();
        Statistic::instance()->setDelay(simTime(), data->getSrcNode(), id, delayPaquet.dbl());
        delete msg;
    }
    else if (data->getTtl() == 0) {
//        ev << this->getFullPath() << "  TTL = 0. Msg deleted" << endl;
        Statistic::instance()->setLost(simTime(), data->getSrcNode(), data->getDstNode());
        delete msg;
    }
    else { // Tant in com out
        int destPort = outPort[data->getDstNode()];
        if (destPort != -2){
            data->setTtl(data->getTtl()-1);
             send(msg, "out", destPort);

//            ev << "Routing: " << this->getFullPath() << "  Source: " << data->getSrcNode() << " Dest: " << data->getDstNode()
//                << " using port: "<< destPort << endl;
        }
        else{
//            ev << this->getFullPath() << " Handling Isolated Nodes " << endl;
            Statistic::instance()->setLost(simTime(), data->getSrcNode(), data->getDstNode());
            delete msg;
        }

    }
    //if (msg->arrivedOn("localIn")) {

}

void Routing::getRoutingInfo(int id, int rData[]) {

     ifstream myfile (folderName + "/Routing.txt");
     double val;

     if (myfile.is_open()) {
         int i = 0;
         while (id != i) {
             for(int k = 0; k < numTx; k++) {
                 string aux;
                 getline(myfile, aux, ',');
             }
             //myfile >> val;
             i++;
         }

         for(int k = 0; k < numTx; k++) {
             string aux;
             getline(myfile, aux, ',');
             val = stod(aux);
             rData[k] = val;
         }

         myfile.close();
     }


}
