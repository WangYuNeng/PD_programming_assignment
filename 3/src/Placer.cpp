#include "Placer.h"

double avgDelta;
double P, T1, T, cool;
double cost, preCost, bestCost, deltaCost;
double maxOriginCost;   // for normalize
double costRatio; // for penalty of W/H ratio
double acceptProb;
int c, k;
int iterations;
int success, failure;
default_random_engine generator;
uniform_real_distribution<double> distribution(0.0,1.0);
Solution sol, prevSol;

void Placer::place ()
{
    // -----------------------------init-----------------------------
    avgDelta = randomPlace();
    P = 0.99;
    cool = 0.1;
    preCost = bestCost = 2;
    maxOriginCost = 1;
    costRatio = 0.5;
    c = 100;
    k = 7;
    iterations = numBLocks * 13;
    success = failure = 0;
    T1 = avgDelta / log( P );
    // --------------------------------------------------------------

    // first stage
    T = T1;
    anneal( 1, 1 );
    restoreBest();

    // second stage
    for (int i = 2; i <= k; i++)
    {
        T = T1 * deltaCost / i / c;
        avgDelta = 0;
        anneal( i, 2 );
        restoreBest();
    }

    // third stage
    for (int i = k + 1; ; i++)
    {
        T = T1 * avgDelta / i;
        avgDelta = 0;
        anneal( i, 3 );
        restoreBest();
        if ( T < cool || TLE() || converged() ) return;
    }
    
    
}

void Placer::anneal ( int r, short stage )
{
    for (int i = 0; i < iterations; i++)
    {
        perturb();
        pack();
        cost = getCost();
        deltaCost = cost - prevSol.cost;
        avgDelta = ( avgDelta * i + deltaCost ) / ( i + 1 );
        acceptProb = 1 / ( exp( deltaCost / T ) );
        acceptProb = acceptProb > 1 ? 1 : acceptProb;
        if ( deltaCost < 0 || distribution(generator) < acceptProb ) keepSol();
        if ( isFeasible() )
        {
            success ++;
            if ( cost < bestCost ) keepBest();            
        } else failure++;
        modifyWeight();
        if ( stage == 2 ) T = T1 * avgDelta / r / c;
        else if ( stage == 3 ) T = T1 * avgDelta / r;
    }
}

double Placer::randomPlace ()
{
    double _max = 0;
    double _sumDelta = 0;
    pack();
    _max = cost = getCost();
    keepSol();
    keepBest();

    for (int i = 0; i < iterations; i++)
    {
        perturb();
        pack();
        cost = getCost();
        deltaCost = cost - prevSol.cost;
        _sumDelta += deltaCost;
        if ( cost > _max ) _max = cost;
        keepSol();
        if ( cost < bestCost ) keepBest();
    }
    restoreBest();
    _sumDelta /= _max;
    maxOriginCost = _max;
    return _sumDelta / iterations;
}

void Placer::perturb ()
{
    int _choice = rand() % 3;
    switch ( _choice )
    {
        case 0:
            rotate( rand() % numBLocks );
            break;
        case 1:
            delete_insert( rand() % numBLocks );
            break;
        case 2:
            swap2nodes( rand() % numBLocks, rand() % numBLocks );
            break;
        default:
            break;
    }
}

void Placer::pack ()
{
    stack<short> nodeStack;
    nodeStack.push( sol.root );

    while ( !nodeStack.empty() )
    {
        Module m = sol.Btree[ nodeStack.top() ];
        nodeStack.pop();
    }
    
}

double Placer::getCost ()
{

}

bool Placer::isFeasible ()
{

}

void Placer::modifyWeight ()
{

}

void Placer::rotate ( short _id )
{

}

void Placer::delete_insert ( short _id1 )
{

}

void Placer::swap2nodes ( short _id1, short _id2 )
{

}

void Placer::keepSol ()
{

}

void Placer::keepBest ()
{

}

void Placer::restoreBest ()
{

}

bool Placer::TLE ()
{

}

bool Placer::converged ()
{

}

Placer::~Placer ()
{
    for ( auto it = nets.cbegin(); it != nets.cend(); ++it ) delete *it; // net contains module and terminal
    for ( auto it = moduleMap.cbegin(); it != moduleMap.cend(); ++it ) delete it->second;
    for ( auto it = terminalMap.cbegin(); it != terminalMap.cend(); ++it ) delete it->second;
}

void Placer::parseModule ( char *_filename )
{
    ifstream _f;
    string _s, _s2;
    short _num1, _num2, _current_node = 0;
    Module* _b;
    Terminal* _t;

    _f.open( _filename );
    root = 0;

    _f >> _s >> outlineWidth >> outlineHeight;
    _f >> _s >> numBLocks;
    _f >> _s >> numTerminals;

    for ( short i = 0; i < numBLocks; i++ )
    {
        _f >> _s >> _num1 >> _num2;
        _b = new Module;
        _b->set( _s, i, _num1, _num2 );
        moduleMap.insert( make_pair( _s, _b ) );
        Btree.push_back( _b );
        if ( i != 0 ) 
        {
            _b->parent = _current_node;
            if ( Btree[_current_node]->left == NIL ) Btree[_current_node]->left = i;
            else
            {
                Btree[_current_node]->right = i;
                _current_node++;
            }
        }
    }
    
    for ( short i = 0; i < numTerminals; i++ )
    {
        _f >> _s >> _s2 >> _num1 >> _num2;
        _t = new Terminal;
        _t->set( _s, _num1, _num2 );
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
    unordered_map<string, Module*>::const_iterator _moduleIt;
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
            _moduleIt = moduleMap.find( _s );
            _terminalIt = terminalMap.find( _s );
            if ( _moduleIt != moduleMap.end() ) _n->connect( _moduleIt->second );
            else if ( _terminalIt != terminalMap.end() ) _n->connect( _terminalIt->second );
            else cout << "error\n";
        }
        nets.push_back( _n );
    }
    
    _f.close();
}