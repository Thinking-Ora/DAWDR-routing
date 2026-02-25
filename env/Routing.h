#ifndef __NETWORKSIMULATOR_NROUTING_H_
#define __NETWORKSIMULATOR_NROUTING_H_

#include <omnetpp.h>
#include "DataPacket_m.h"
#include "Statistic.h"

using namespace std;
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <string>


/**
 * TODO - Generated class
 */
class Routing : public cSimpleModule
{
  private:
    int numPorts;
    int id;
    int numTx;
    int numNodes;
//    int outPort[200];
    std::vector<int> outPort;
    string folderName;



  public:
    Routing();
    virtual ~Routing();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void getRoutingInfo(int id, int rData[]);

};

#endif
