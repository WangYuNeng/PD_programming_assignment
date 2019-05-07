#include "Placer.h"

int main( int args, char ** argv )
{
    Placer p;
    p.parseBlock( argv[2] );
    p.parseNets( argv[3] );
    return 0;
}