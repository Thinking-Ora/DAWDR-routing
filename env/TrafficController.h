#ifndef __SCALEFREE_TRAFFICCONTROLLER_H_
#define __SCALEFREE_TRAFFICCONTROLLER_H_

#include <omnetpp.h>
#include "ControlPacket_m.h"
#include "Statistic.h"
using namespace std;
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>

/**
 * TODO - Generated class
 */
class TrafficController : public cSimpleModule
{
    private:
        double nodeRatio;
        int numNodes;
        int id;
//        double flowRatio[500];
        std::vector<double> flowRatio;
        string  folderName;

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
//        int getTrafficInfo(int id, double rData[], int TC_RunNumber);
        virtual int getTrafficInfo(int id, std::vector<double>& rData, int TC_RunNumber); // <-- 修改
};

#endif
