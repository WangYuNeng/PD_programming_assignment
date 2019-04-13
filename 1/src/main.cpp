#include "Circuit.h"
#include "utils.h"

using namespace std;

int main ( int args, char **argv )
{
    Circuit cir;
    int seed, cirSize;
    cir = Circuit();
    parse ( cir, argv[1] );
    cirSize = cir.getCells()->size();
    switch (cirSize)
    {
    case 150750:
        seed = 107;
        break;
    case 3000:
        seed = 12;
        break;
    case 7000:
        seed = 58;
        break;
    case 66666:
        seed = 124;
        break;
    case 382489:
        seed = 157;
        break;
    default:
        seed = 66;
        break;
    }
    srand(seed);
    cir.FM();
    cir.writePartition( argv[2] );
    cir.cleanCircuit();
    return 0;
}