#include "FP.h"

double alpha;

int main( int args, char ** argv )
{
    FP p;
    alpha = atof( argv[1] );
    p.parseModule( argv[2] );
    p.parseNets( argv[3] );
    p.floorplan();
    p.writeFile( argv[4] );
    return 0;
}