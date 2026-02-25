package networksimulator;

//
// TODO auto-generated module
//
simple Routing
{
    parameters:
        int id;
        int numPorts = default(sizeof(in));
        int numTx;
        int numNodes;
        string folderName;

    gates:
        input localIn[];
        output localOut[];
        input in[];
        output out[];

}
