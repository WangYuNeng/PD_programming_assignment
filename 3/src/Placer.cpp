#include "Placer.h"

Placer::~Placer ()
{
    for ( auto it = nets.cbegin(); it != nets.cend(); ++it ) delete *it; // net contains block and terminal
    for ( auto it = blockMap.cbegin(); it != blockMap.cend(); ++it ) delete it->second;
    for ( auto it = terminalMap.cbegin(); it != terminalMap.cend(); ++it ) delete it->second;
}

void Placer::parseBlock ( char *_filename )
{
    ifstream _f;
    string _s, _s2;
    short _num1, _num2;
    Block* _b;
    Terminal* _t;

    _f.open( _filename );

    _f >> _s >> outlineWidth >> outlineHeight;
    _f >> _s >> numBLocks;
    _f >> _s >> numTerminals;

    for ( short i = 0; i < numBLocks; i++ )
    {
        _f >> _s >> _num1 >> _num2;
        _b = new Block( _s, _num1, _num2 );
        blockMap.insert( make_pair( _s, _b ) );
    }
    
    for ( short i = 0; i < numTerminals; i++ )
    {
        _f >> _s >> _s2 >> _num1 >> _num2;
        _t = new Terminal( _s, _num1, _num2 );
        terminalMap.insert( make_pair( _s, _t ) );
    }

    _f.close();
}

void Placer::parseNets ( char *_filename )
{
    ifstream _f;
    string _s;
    short _num;
    Net* _n;
    unordered_map<string, Block*>::const_iterator _blockIt;
    unordered_map<string, Terminal*>::const_iterator _terminalIt;

    _f.open( _filename );

    _f >> _s >> numNets;

    for ( short i = 0; i < numNets; i++ )
    {
        _n = new Net;
        _f >> _s >> _num;
        for ( short j = 0; j < _num; j++)
        {
            _f >> _s;
            _blockIt = blockMap.find( _s );
            _terminalIt = terminalMap.find( _s );
            if ( _blockIt != blockMap.end() ) _n->connect( _blockIt->second );
            else if ( _terminalIt != terminalMap.end() ) _n->connect( _terminalIt->second );
            else cout << "error\n";
        }
        nets.push_back( _n );
    }
    
    _f.close();
}