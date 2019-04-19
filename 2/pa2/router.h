#ifndef __ROUTER_H__
#define __ROUTER_H__
#include "routingdb.h"
#include <cassert>

class Grid;
class router;

enum DIRECTION {UP, DOWN, LEFT, RIGHT, NONE};
enum LAYER : bool { HORI=true, VERTI=false };
typedef pair<int, int> Pos;

class Grid
{
    public:
    Grid ();
    ~Grid ();

    // get function
    int getCapacity ( const Pos &_pin1, const Pos &_pin2 );
    int getLoad ( const Pos &_pin1, const Pos &_pin2 );

    // modify function
    void incrLoad ( const Pos &_pin1, const Pos &_pin2 );

    // print function
    void showInfo ();

    private:
    int vtiles, htiles;
    int **vCapacity, **hCapacity, **vLoad, **hLoad; 
    pair<const Pos*, bool> tile2Boundary ( const Pos &_pin1, const Pos &_pin2 );
};

class router
{
public:
    router();
    ~router();

    void runDijstra();

private:
    Grid *grid;
    map<int, Net*> orderedNets;

    // Ordering function
    void netOrdering();
    int getNetOrder( Net &n );
};

#endif