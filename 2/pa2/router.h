#ifndef __ROUTER_H__
#define __ROUTER_H__
#include "routingdb.h"
#include <cassert>

class Grid;

enum DIRECTION {UP, DOWN, LEFT, RIGHT, NONE};
enum LAYER : bool { HORI=true, VERTI=false };
typedef pair<int, int> Pos;

class Grid
{
    public:
    Grid ( RoutingDB &db );
    ~Grid ();

    // get function
    int capacity ( const Pos &_pin1, const Pos &_pin2 );
    int load ( const Pos &_pin1, const Pos &_pin2 );

    // modify function
    void incrLoad ( const Pos &_pin1, const Pos &_pin2 );

    // print function
    void showInfo ();

    private:
    int vtiles, htiles;
    int **vCapacity, **hCapacity, **vLoad, **hLoad; 
    pair<Pos, bool> tile2Boundary ( const Pos &_pin1, const Pos &_pin2 );
};

#endif