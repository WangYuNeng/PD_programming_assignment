#ifndef CIRCUIT_HEADER_
#define CIRCUIT_HEADER_

#include <cstdlib>
#include <vector>
#include <list>
#include <array>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <map>

class Net;
class Cell;
class Group;
class Circuit;
#define G0 0
#define G1 1
#define EMPTY_GROUP -2147483648
typedef std::vector<Net*> Nets;
typedef std::vector<Cell*> Cells;
typedef std::list<Cell*> CellList;
typedef std::list<Cell*>::iterator CellListIt;

class Cell
{
friend class Group;
friend class Circuit;
private:
    Nets terminals;
    CellListIt gainPos;
    int gain;
    Group* belong;
public:
    Cell () { gain = 0; }
    ~Cell () {}

    char* name;
    // update function
    void addTerminal ( Net *n ) { terminals.push_back(n); }
    void incrGain ();
    void decrGain ();
    void incrGain ( int g);
    void assignGroup ( Group* g ) { belong = g; }
    void setGainPos ( CellListIt it ) { gainPos = it; }

    // access function
    size_t getNumTerminal () { return terminals.size(); }
    Net* getTermianl ( int i ) { return terminals.at(i); }
    Group* getGroup () { return belong; }
    CellListIt getGainPos () { return gainPos; }

    // print function
    void printTerminal ();
};

class Net
{
private:
    Cells terminals;
    int gCount[2];
public:
    Net () {}
    ~Net () {}

    char* name;
    void addTerminal ( Cell *n ) { terminals.push_back(n); }
    void resetGCount () { gCount[0] = gCount[1] = 0; }
    void incrGCount ( int i ) { ++gCount[i]; }
    void decrGCount ( int i ) { --gCount[i]; }

    size_t getNumTerminal () { return terminals.size(); }
    Cell* getTerminal ( int i ) { return terminals.at(i); }
    char* getName () { return name; }
    int getGCount ( int i ) { return gCount[i]; }

    void printTerminal ();
};

class Group
{
private:
    CellList *GainArray;
    int Pmax, max, gsize;
public:
    Group () {}
    ~Group() { delete [] GainArray; }

    int getSize () { return gsize; }
    Cell* getMaxCell () { return GainArray[max+Pmax].front(); }
    int getMax () { return max; }

    void initialize ( int pm ) { GainArray = new CellList [2*pm + 1]; Pmax = pm; max = -pm; gsize = 0; }
    void refresh () { max = -Pmax; gsize = 0; }
    void deleteCell ( Cell* c );
    void addCell ( Cell* c );
    void addLockedCell () { ++gsize; }

    void printGroup();
    void writeCells( FILE* fp );
};

class Circuit
{
private:
    Cells cells;
    Nets nets;
    Group* groups;
    double ratio;

    // FM function
    inline void initialPartition ();
    inline void initialGain();
    inline Cell* getMaxGainCell();
    inline void updateGain( Cell* );
    inline void updateSum( Cell*, int&, int&, int&, int&, int );
    inline void bestMoves( Cell**, bool*, int );

public:
    Circuit () {}
    ~Circuit () {}

    void FM();

    // set function
    void setRatio ( double r ) { ratio = r; }
    void setGroups ( Group* g ) { groups = g; }

    // get function
    Cells* getCells () { return &cells; }
    Nets* getNets () { return &nets; }
    Group* getGroups ( bool i ) { return groups+int(i); }

    // free memory
    void cleanCircuit ();

    // print funtction
    void printCells ();
    void printNets ();
    void printGroups ();
    void writePartition ( char* );
};

#endif