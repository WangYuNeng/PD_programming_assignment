#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

using namespace std;

class Placer;
class Block;
class Terminal;
class Net;

class Block
{
public:
    Block () {}
    Block ( string &_n, short _w, short _h ) { name = _n; width = _w, height = _h; }
    ~Block () {}

    // get
    string& getName () { return name; }

    // print
    void print() { cout << name << " " << width << " " << height << endl; }

private:
    string name;
    short width, height;
};

class Terminal
{
public:
    Terminal () {}
    Terminal ( string &_n, short _x, short _y ) { name = _n; x_coord = _x, y_coord = _y; }
    ~Terminal () {}

    // get
    string& getName () { return name; }

    // print
    void print() { cout << name << " " << x_coord << " " << y_coord << endl; }

private:
    string name;
    short x_coord, y_coord;
};

class Net
{
public:
    Net () {}
    ~Net () {}

    // initialize
    void connect ( Block* _b ) { blocks.push_back( _b ); }
    void connect ( Terminal* _b ) { terminals.push_back( _b ); }

    // print
    void print() { 
        for ( short i = 0; i < blocks.size(); i++ ) cout << blocks[i]->getName() << endl;
        for ( short i = 0; i < terminals.size(); i++ ) cout << terminals[i]->getName() << endl;  
        cout << endl;      
    }

private:
    vector<Block*> blocks;
    vector<Terminal*> terminals;
};

class Placer
{
public:
    Placer () {}
    ~Placer ();

    // parse
    void parseBlock ( char* _filename );
    void parseNets ( char* _filename );

    // print
    void printAll ()  {
        cout << "Blocks\n";
        for ( auto it = blockMap.cbegin(); it != blockMap.cend(); ++it ) it->second->print();
        cout << "Terminals\n";
        for ( auto it = terminalMap.cbegin(); it != terminalMap.cend(); ++it ) it->second->print();
        cout << "Nets\n";
        for ( auto it = nets.cbegin(); it != nets.cend(); ++it ) (*it)->print();
    }

private:
    int outlineWidth, outlineHeight;
    int numBLocks, numTerminals, numNets; 
    unordered_map<string, Block*> blockMap;
    unordered_map<string, Terminal*> terminalMap;
    vector<Net*> nets;
};


