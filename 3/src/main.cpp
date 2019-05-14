#include "FP.h"

int main( int args, char ** argv )
{
    FP p;
    p.parseModule( argv[2] );
    p.parseNets( argv[3] );
    p.printAll();
    return 0;
}