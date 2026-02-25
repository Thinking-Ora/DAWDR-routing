#ifndef __NETWORKSIMULATOR_QUEUE_H_
#define __NETWORKSIMULATOR_QUEUE_H_

#include <iostream>
#include <omnetpp.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <queue>
#include "DataPacket_m.h"
#include "Statistic.h"
#include <fstream>
#include <sstream>
#include <limits>


using namespace std;


/**
 * TODO - Generated class
 */
class NodeQueue : public cSimpleModule
{

  public:
    NodeQueue();
    virtual ~NodeQueue();
    void loadQueueFromFile();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    int nodeID;
    int queueID;


  private:
    queue<cMessage*> portQueue;
    cMessage* endTxMsg;
    int deleted;
    string folderName;
    int myNodeID;
    int myQueueID;

};

#endif
