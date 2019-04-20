#ifndef __ROUTER_H__
#define __ROUTER_H__
#include "routingdb.h"
#include <cassert>
#include <queue>
#include <limits>
#include <cmath>

class Grid;
class Router;

class Vertice
{
    public:
    Vertice () {} 
    Vertice ( short _x, short _y ) { x = _x; y =_y; }
    Vertice ( short _x, short _y, short _distance ) { x = _x; y =_y; distance = _distance; }
    ~Vertice() {} 

     // check if vertice move out-of-range
    bool isValid () { return (x >= 0 && y >= 0 && x < db.GetHoriGlobalTileNo() && y < db.GetVertiGlobalTileNo() ) ; }
    
    short x;
    short y;
    double distance;
};

class Vertice_Compare{
public:
    bool operator() (Vertice* v1, Vertice* v2){
        if (v1->distance > v2->distance) return true;
        return false;
    }
};

enum DIRECTION {UP, DOWN, LEFT, RIGHT, NONE};
enum LAYER : bool { HORI=true, VERTI=false };
typedef priority_queue<Vertice*, vector<Vertice*>, Vertice_Compare> minHeap;

class Grid
{
    public:
    Grid ();
    ~Grid ();

    // get function
    int getCapacity ( const Vertice* _pin1, const Vertice* _pin2 );
    int getLoad ( const Vertice* _pin1, const Vertice* _pin2 );

    // modify function
    void incrLoad ( const Vertice* _pin1, const Vertice* _pin2 );

    // print function
    void showInfo ();

    private:
    int vtiles, htiles;
    int **vCapacity, **hCapacity, **vLoad, **hLoad; 
    pair<const Vertice*, bool> tile2Boundary ( const Vertice* _pin1, const Vertice* _pin2 );
};

class DijstraMap
{
    public:
    DijstraMap ();
    ~DijstraMap ();

    bool isConnected ( Vertice* _p ) { return connected[_p->x][_p->y]; }
    bool isMinDistance ( Vertice* _p ) { return _p->distance == distance[_p->x][_p->y]; }
    void BackTrace ( Vertice* _p );
    void refreshConnection ();
    void refreshDistance ();

    void connect ( Vertice* _p ) { connected[_p->x][_p->y] = true; }
    void setParent ( Vertice* _p, int _direction ) { parent[_p->x][_p->y] = _direction; };
    void setDistance ( Vertice* _p ) { distance[_p->x][_p->y] = _p->distance; };

    private:
    int **parent; // dirction is in _p's parent's view
    bool **connected;
    double **distance; // substitute for decrease key

};

class Router
{
public:
    Router();
    ~Router();

    void routeAll();

private:
    map<int, Net*> orderedNets;
    Grid *grid;
    DijstraMap *dMap;

    // Dijstra function
    void runDijstra ( SubNet* _sn );
    void relax( Vertice* _p, minHeap& _mheap );
    Vertice* toVertice( Vertice* _p, int _direction );
    double getWeight( Vertice* _pFrom, Vertice* _pTo );
    void update ( Vertice* _pFrom, Vertice* _pTo, double _newDist, minHeap& _mheap );


    // Ordering function
    void netOrdering ();
    int getNetOrder ( Net &_n );
    int getSubnetOrder( SubNet &_sn );

    // util function
    int getManhattanDist ( short _x1, short _y1, short _x2, short _y2 );
    
};

#endif