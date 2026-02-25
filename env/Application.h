#ifndef __NETWORKSIMULATOR_APPLICATION_H_
#define __NETWORKSIMULATOR_APPLICATION_H_

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include "DataPacket_m.h"
#include "TimerNextPacket_m.h"
#include "ControlPacket_m.h"
#include "Statistic.h"


using namespace std;

/**
 * TODO - Generated class
 */
class Application : public cSimpleModule
{
  private:
    TimerNextPacket *interArrival;
    int id;
    int NumPackets;
    int genT;
    double lambdaFactor;
    double numRx;
    int ttlInitial;
    double lambda;
    double MAXSIM;
    int dest;
    string folderName;
    //string queueFileName;



  public:
    Application();
    virtual ~Application();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void logCurrentQueueState();

  private:
    double nextPacket(int i);
    int nextDest();
    int extractId(string name, int pos);
    void initSignals();

};

#endif
