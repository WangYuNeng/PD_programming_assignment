#ifndef _FP_H_
#define _FP_H_

#include <cstdio>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <list>
#include <limits>

using namespace std;
const short NIL = -1;
const bool LEFT = 0, RIGHT = 1;

struct Module // maybe can add nets to module
{
    string name;
    short id, width, height;
    short parent, left, right;
    void set ( string &_n, short _id, short _w, short _h ) { name = _n; id = _id; width = _w, height = _h; parent = NIL; left = NIL; right = NIL; }
    bool isleaf () { return left == NIL && right == NIL; }
    bool isroot () { return parent == NIL; }
    void rotate () { short tmp = width; width = height; height = tmp; }
    void print() { cout << name << " " << width << " " << height << endl; }
};

struct Terminal
{
    string name;
    short x_coord, y_coord;
    void set ( string &_n, short _x, short _y ) { name = _n; x_coord = _x, y_coord = _y; }
    void print() { cout << name << " " << x_coord << " " << y_coord << endl; }
};

struct Net
{
    vector<short> moduleIDs;
    vector<Terminal*> terminals;
    void connect ( short _id ) { moduleIDs.push_back( _id ); }
    void connect ( Terminal* _b ) { terminals.push_back( _b ); }
    void print() { 
        for ( short i = 0; i < moduleIDs.size(); i++ ) cout << moduleIDs[i] << endl;
        for ( short i = 0; i < terminals.size(); i++ ) cout << terminals[i]->name << endl;  
        cout << endl;      
    }
};

struct Solution
{
    short root;
    vector<Module> Btree;
    double cost;
    Solution () { cost = 1; }
};

struct Coordinate
{
    short x, y;
    Coordinate ( short _x, short _y ) { x = _x, y = _y; }
};

class FP
{
public:
    FP () {}
    ~FP ();

    // parse
    void parseModule ( char* _filename );
    void parseNets ( char* _filename );

    // floorplam
    void floorplan ();

    // print
    void printAll ()  {
        cout << "Modules\n";
        for ( auto it = moduleMap.cbegin(); it != moduleMap.cend(); ++it ) it->second->print();
        cout << "Terminals\n";
        for ( auto it = terminalMap.cbegin(); it != terminalMap.cend(); ++it ) it->second->print();
        cout << "Nets\n";
        for ( auto it = nets.cbegin(); it != nets.cend(); ++it ) (*it)->print();
    }

private:
    short outlineWidth, outlineHeight;
    short numBLocks, numTerminals, numNets; 
    map<string, Module*> moduleMap;
    map<string, Terminal*> terminalMap;
    vector<Net*> nets;
    list<Coordinate> contours;
    vector<Coordinate> coords;
    Solution bestSol;

    // pre-place
    double randomPlace ();

    // annealing
    void anneal ( int r, short stage );
    void perturb ();
    void pack ();
    double getCost ();
    bool isFeasible ();
    void modifyWeight ();

    // preturb
    void rotate ( short _id );
    void delete_insert ( short _id1 );
    void swap2nodes ( short _id1, short _id2 );
    
    void connect ( short _parent, short _child, bool _dir );

    // pack
    short modifyContour ( short _x1, short _x2, short _h );

    // solution
    void keepBest ();
    void restorePrev ();
    void restoreBest ();

    // terminate conition
    bool TLE ();
    bool converged ();
};

bool rand_bool ();
double rand_01 ();

#endif
