#ifndef _PLACER_H_
#define _PLACER_H_

#include <cstdio>
#include <cmath>
#include <random>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>

using namespace std;
const short NIL = -1;

struct Module // maybe can add nets to module
{
    string name;
    short id, width, height;
    short parent, left, right;
    void set ( string &_n, short _id, short _w, short _h ) { name = _n; id = _id; width = _w, height = _h; parent = NIL; left = NIL; right = NIL; }
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
    vector<Module*> modules;
    vector<Terminal*> terminals;
    void connect ( Module* _b ) { modules.push_back( _b ); }
    void connect ( Terminal* _b ) { terminals.push_back( _b ); }
    void print() { 
        for ( short i = 0; i < modules.size(); i++ ) cout << modules[i]->name << endl;
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

struct Contour
{
    short front, back;
};

class Placer
{
public:
    Placer () {}
    ~Placer ();

    // parse
    void parseModule ( char* _filename );
    void parseNets ( char* _filename );

    // place
    void place ();

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
    short root; 
    unordered_map<string, Module*> moduleMap;
    unordered_map<string, Terminal*> terminalMap;
    vector<Net*> nets;
    vector<Module*> Btree;
    vector<Contour> contours;
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

    // solution
    void keepSol ();
    void keepBest ();
    void restoreBest ();

    // terminate conition
    bool TLE ();
    bool converged ();
};

#endif
