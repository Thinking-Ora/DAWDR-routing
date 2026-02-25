package networksimulator;

//
// TODO auto-generated module
//
simple Application
{

    parameters:
        int id;
        double lambda;
        int dest;
        int generation;
        int numNodes;
        string folderName;
        int ttlInitial;
        double simulationDuration;

        @display("i=block/browser");


    gates:
        output out;
        input tcontrol;
}
