#include "Application.h"

Define_Module(Application);


Application::Application() {

     interArrival = NULL;

}

Application::~Application() {
     //cancelAndDelete(interArrival);
     if (interArrival) {
        cancelAndDelete(interArrival);
     }
}


void Application::initialize()
{
    //id = extractId(this->getFullPath(), 12);
    id = par("id");
    genT = par("generation");
    lambdaFactor = par("lambda");
    dest = par("dest");
    MAXSIM = par("simulationDuration");
    numRx = par("numNodes");
    ttlInitial = par("ttlInitial");
    NumPackets = 0;
    folderName = par("folderName").stdstringValue();


    Statistic::instance()->setGeneration(genT);
    Statistic::instance()->setMaxSim(MAXSIM);
    Statistic::instance()->setLambda(lambdaFactor);

}




void Application::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage()) {
        DataPacket *data = new DataPacket("dataPacket");

        int size;
        switch (genT) {
            case 0: // Poisson
//                size = exponential(1000);
                size = exponential(5000);
                if (size > 50000) size = 50000;
                break;
            case 1: // Deterministic
                size = 10000;   // 1000
                break;
            case 2: // Uniform
                size = uniform(0,2000);
                break;
            case 3: // Binomial
                if (dblrand() < 0.5) size = 300;
                else size = 1700;
                break;
            default:
                break;
        }


        data->setBitLength(size);
//        data->setTtl(numRx);
        data->setTtl(ttlInitial);
        data->setDstNode(dest);
        data->setSrcNode(id);
        data->setLastTS(simTime().dbl());

        send(data, "out");

        NumPackets++;


        Statistic::instance()->setTraffic(simTime(), id, dest, size);
        Statistic::instance()->infoTS(simTime());


        // 计划下一次数据包生成时间
        if (simTime() < MAXSIM) {
//            simtime_t etime= exponential(1.0/lambda);
            simtime_t etime= 1.0/lambda;
//            EV << "Lambda value: " << lambda << endl;
//            EV << "NumPackets value: " << NumPackets << endl;

            scheduleAt(simTime() + etime, msg);
        }
        else {
//            EV << "END simulation" << endl;
        }
        // logCurrentQueueState(); // Log the queue state

    }

    else {
        ControlPacket *data = check_and_cast<ControlPacket *>(msg);
        double flowRatio = data->getData();
        lambda = lambdaFactor * flowRatio;
        //lambda = lambdaMax/numRx;   //我觉得statistic:lambdaMax   /numRx

        interArrival = new TimerNextPacket("timer");
        interArrival->setLambda(1.0/lambda);
        if (dest != id)
            scheduleAt(simTime() + 1.0/lambda, interArrival);
//        ev << "Ratio: " << flowRatio << "   lambda: " << lambda << endl;


        delete data;


    }


}
