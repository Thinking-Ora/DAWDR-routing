package networksimulator;

//
// TODO auto-generated module
//
simple NodeQueue
{
    parameters:
        int capacity @unit(byte) = default (1024B);
        @display("i=block/queue");
        string folderName;


    gates:
        input in;
        output out;
        inout line;
}
