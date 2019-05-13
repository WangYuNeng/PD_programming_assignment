#include "Placer.h"

int main( int args, char ** argv )
{
    Placer p;
    p.parseModule( argv[2] );
    p.parseNets( argv[3] );
    p.printAll();
    return 0;
}