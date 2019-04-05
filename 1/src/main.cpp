#include "Circuit.h"
#include "utils.h"

using namespace std;

int main ( int args, char **argv )
{
    Circuit cir;
    parse ( cir, argv[1] );
    cir.FM();
    cir.writePartition( argv[2] );
    cir.cleanCircuit();
    return 0;
}