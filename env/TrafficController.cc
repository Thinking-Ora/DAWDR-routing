#include "TrafficController.h"
//#include "GlobalPause.h"

Define_Module(TrafficController);

void TrafficController::initialize()
{
    id = par("id");
    nodeRatio = par("nodeRatio");
    numNodes = par("numNodes");
    folderName = par("folderName").stdstringValue();
    Statistic::instance()->initializeVectors(numNodes);

    //
    int TC_RunNumber = 0;
    int MODE = 3;
    if (MODE == 1) {
        // UNIFORM TRAFFIC PER FLOW
        for (int i = 0; i < numNodes; i++) {
            double aux;
            if (i == id) aux = 0;
            else aux = uniform(0.1,1);

            ControlPacket *data = new ControlPacket("trafficInfo");
            data->setData(aux/numNodes);
            send(data, "out", i);
        }
    }
    else if (MODE == 2) {
        // UNIFOR TRAFFIC PER NODE
        double flowRatio[numNodes];
        double sumVal = 0;
        for (int i = 0; i < numNodes; i++) {
            double aux;

            if (i == id) aux = 0;
            else aux = uniform(0.1,1);

            flowRatio[i] = aux;
            sumVal += aux;
        }

        //ev << "NODE Ratio: " << nodeRatio << endl;
        for (int i = 0; i < numNodes; i++) {
            ControlPacket *data = new ControlPacket("trafficInfo");
            data->setData(nodeRatio*flowRatio[i]/sumVal);
            send(data, "out", i);
        }
    }
    else {
        // READED FROM FILE
        // TC_RunNumber = getTrafficInfo(id, flowRatio, TC_RunNumber);
        flowRatio.resize(numNodes);
        getTrafficInfo(id, flowRatio, TC_RunNumber);
        for (int i = 0; i < numNodes; i++) {
            ControlPacket *data = new ControlPacket("trafficInfo");
            data->setData(flowRatio[i]);
            send(data, "out", i);
        }
    }

}

void TrafficController::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

int TrafficController::getTrafficInfo(int id, std::vector<double>& rData, int TC_RunNumber) {
     string line;
     ifstream myfile (folderName + "/Traffic.txt");
     double val;

     //TC_RunNumber += 1;

     if (myfile.is_open())
     {
         int i = 0;
         while (id != i) {
             for(int k = 0; k < numNodes; k++) {
                 string aux;
                 getline(myfile, aux, ',');
             }
             //myfile >> val;
             i++;
         }

         //ofstream infoFile (folderName + "/OutputInfo.txt", std::ios::out | std::ios::app);
         for(int k = 0; k < numNodes; k++) {
             string aux;
             getline(myfile, aux, ',');
             val = stod(aux);
             rData[k] = val;
             //infoFile<<"TC_RunNum:"<< TC_RunNumber << "\t"<<"k:"<< k << endl;
         }
         //infoFile<<"test TC_RunNumber:"<< TC_RunNumber+1 << endl;
         //infoFile.close();

         myfile.close();
     }
     return TC_RunNumber;
}
