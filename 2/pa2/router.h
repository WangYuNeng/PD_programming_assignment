#ifndef __ROUTER_H__
#define __ROUTER_H__
#include "routingdb.h"
#include <cassert>
#include <queue>
#include <limits>
#include <cmath>

class DijstraMap;
class Router;

class Vertice
{
    public:
    Vertice () {} 
    Vertice ( short _x, short _y ) { x = _x; y =_y; }
    Vertice ( short _x, short _y, short _distance ) { x = _x; y =_y; distance = _distance; }
    ~Vertice() {} 
    
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

class DijstraMap
{
    public:
    DijstraMap ();
    ~DijstraMap ();

    // get function
    int getCapacity ( const Vertice* _pin1, const Vertice* _pin2 );
    int getLoad ( const Vertice* _pin1, const Vertice* _pin2 );
    bool isConnected ( const Vertice* _p1, const Vertice* _p2 );
    bool isViaConnected ( const Vertice* _p ) { return viaConnected[_p->x][_p->y]; };
    bool isMinDistance ( const Vertice* _p ) { return _p->distance == distance[_p->x][_p->y]; }
    bool unDiscovered ( short _x, short _y ) { return distance[_x][_y] == numeric_limits<double>::max(); }
    
    // refresh function
    void refreshConnection ();
    void refreshDistance ();
    
    // set function
    void connect ( const Vertice* _p1, const Vertice* _p2 );
    void connectVia ( const Vertice* _p ) { viaConnected[_p->x][_p->y] = true; };
    void setParent ( const Vertice* _p, const int _direction ) { parent[_p->x][_p->y] = _direction; };
    void setDistance ( const Vertice* _p ) { distance[_p->x][_p->y] = _p->distance; };
    void incrLoad ( const Vertice* _pin1, const Vertice* _pin2 );

    // print function
    int BackTrace ( Vertice* _p, vector<string>& _output );
    void showInfo ();

    private:
    int **parent; // dirction is in _p's parent's view
    double **distance; // substitute for decrease key
    bool **hConnected, **vConnected, **viaConnected;
    int **vCapacity, **hCapacity, **vLoad, **hLoad; 
    
    pair<const Vertice*, bool> tile2Boundary ( const Vertice* _pin1, const Vertice* _pin2 );

};

class Router
{
public:
    Router();
    ~Router();

    void routeAll( ofstream& _of );

private:
    map<int, Net*> orderedNets;
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

    // print function
    void printNet( Net* _n, vector<string>& _output, int _routeCount, ofstream& _of );
    
};

#endif