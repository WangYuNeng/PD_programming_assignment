#include "FP.h"

extern double alpha;
double avgDelta;
double P, T1, T, cool;
double cost, preCost, bestCost, deltaCost;
double maxOriginCost;   // for normalize
double costRatio; // for penalty of W/H ratio
double acceptProb;
int c, k;
int iterations;
int success, failure;
short maxX, maxY; // check feasible
Solution sol;

// for restrore previous
vector<Module> changedModules;
short chagedRoot;

void FP::floorplan ()
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

void FP::anneal ( int r, short stage )
{
    for (int i = 0; i < iterations; i++)
    {
        perturb();
        pack();
        cost = getCost();
        deltaCost = cost - preCost;
        avgDelta = ( avgDelta * i + deltaCost ) / ( i + 1 );
        acceptProb = 1 / ( exp( deltaCost / T ) );
        acceptProb = acceptProb > 1 ? 1 : acceptProb;
        if ( deltaCost < 0 || rand_01() < acceptProb ) preCost = cost;
        else restorePrev();
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

double FP::randomPlace ()
{
    double _max = 0;
    double _sumDelta = 0;
    pack();
    _max = cost = getCost();
    keepBest();

    for (int i = 0; i < iterations; i++)
    {
        perturb();
        pack();
        cost = getCost();
        deltaCost = cost - preCost;
        _sumDelta += deltaCost;
        if ( cost > _max ) _max = cost;
        if ( cost < bestCost ) keepBest();
    }
    restoreBest();
    _sumDelta /= _max;
    maxOriginCost = _max;
    return _sumDelta / iterations;
}

void FP::perturb ()
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

void FP::pack ()
{
    stack<short> nodeStack;

    nodeStack.push( sol.root );

    while ( !nodeStack.empty() )
    {
        Module &m = sol.Btree[ nodeStack.top() ];
        nodeStack.pop();

        short _x, _y, _parentID;
        if ( m.parent != NIL )
        {
            if ( sol.Btree[ m.parent ].left = m.id )
            {
                _x = coords[ m.parent ].x + sol.Btree[ m.parent ].width;
                _y = modifyContour( _x, _x+m.width, m.height );     
            } else
            {
                _x = coords[ m.parent ].x;
                _y = modifyContour( _x, _x+m.width, m.height );     
            }
            coords[ m.id ].x = _x;
            coords[ m.id ].y = _y;
        } else
        {
            coords[ m.id ].x = 0;
            coords[ m.id ].y = 0;
            contours.clear();
            contours.push_back( Coordinate( 0, 0 ) );
            contours.push_back( Coordinate( 0, m.height ) );
            contours.push_back( Coordinate( m.width, m.height ) );
            contours.push_back( Coordinate( m.width, 0 ) );
            contours.push_back( Coordinate( numeric_limits<short>::max(), 0 ) );
        }
        
        if ( m.right != NIL ) nodeStack.push( m.right );
        if ( m.left != NIL ) nodeStack.push( m.left );  
    }
    
}

double FP::getCost ()
{
    maxX = maxY = 0;
    for ( auto it = contours.begin(); it != contours.end(); it++ )
    {
        if ( it->x > maxX && it->x != numeric_limits<short>::max() ) maxX = it->x;
        if ( it->y > maxY ) maxY = it->y;
    }
    int _area = maxX * maxY;
    
    short _left, _right, _bottom, _top, _x, _y;
    int _wireLength = 0;
    for ( auto _n = nets.begin(); _n != nets.end(); _n++ )
    {
        _left = _bottom = numeric_limits<short>::max();
        _right = _bottom = 0;
        for (auto _m = (*_n)->moduleIDs.begin(); _m != (*_n)->moduleIDs.end(); _m++)
        {
            _x = coords[*_m].x;
            _y = coords[*_m].y;
            if ( _x > _right ) _right = _x;
            if ( _x < _left ) _left = _x;
            if ( _y > _top ) _top = _y;
            if ( _y < _bottom ) _bottom = _y;
        }
        for (auto _t = (*_n)->terminals.begin(); _t != (*_n)->terminals.end(); _t++)
        {
            _x = (*_t)->x_coord;
            _y = (*_t)->y_coord;
            if ( _x > _right ) _right = _x;
            if ( _x < _left ) _left = _x;
            if ( _y > _top ) _top = _y;
            if ( _y < _bottom ) _bottom = _y;
        }
        _wireLength += ( _right - _left ) + ( _top - _bottom );
    }

    double _cost = alpha * _area + ( 1 - alpha )*_wireLength; // how to normalize???

}

bool FP::isFeasible ()
{
    return maxX <= outlineWidth && maxY <= outlineHeight;
}

void FP::modifyWeight ()
{
    // need further tuned
    costRatio *= failure*1.0 / ( failure+success ) * 1.0;
}

void FP::rotate ( short _id )
{
    changedModules.clear();
    chagedRoot = NIL; 
    changedModules.push_back( sol.Btree[_id] );
    sol.Btree[_id].rotate();
}

void FP::delete_insert ( short _id1 )
{
    changedModules.clear();
    chagedRoot = sol.root;
    for (auto i = 0; i < sol.Btree.size(); i++)
    {
        changedModules.push_back( sol.Btree[i] );
    }
    

    bool _dir, _from;
    short _child, _parent;
    short _lchild, _rchild, _newlchild, _newrchild;

    _parent = sol.Btree[_id1].parent;
    _lchild = sol.Btree[_id1].left;
    _rchild = sol.Btree[_id1].right;
    _from = ( sol.Btree[_parent].right == _id1 );
    while (1)
    {
        if ( _lchild == NIL || _rchild == NIL )
        {
            if ( _lchild == NIL && _rchild == NIL )
            {
                if ( _from == LEFT ) sol.Btree[_parent].left = NIL;
                else sol.Btree[_parent].right = NIL;
            } else
            {
                if ( _lchild != NIL ) _child = _lchild;
                else _child = _rchild;
                connect( _parent, _child, _from );
            }
            break;
        }

        _dir = rand_bool();
        if ( _dir == LEFT ) _child = _lchild;
        else _child = _rchild;
        connect( _parent, _child, _from );

        _newlchild = sol.Btree[_child].left;
        _newrchild = sol.Btree[_child].right;

        if ( _dir == LEFT ) connect( _lchild, _rchild, RIGHT );
        else connect( _rchild, _lchild, LEFT );
        
        _from = _dir;
        _parent = _child;
        _lchild = _newlchild;
        _rchild = _newrchild;
    }      
    
    // insert
    sol.Btree[_id1].left = NIL;
    sol.Btree[_id1].right = NIL;
    short _id2 = rand() % numBLocks;
    _dir = rand_bool();
    while( _id2 == _id1 )
    {
        _id2 = rand() % numBLocks;
    }

    if ( _dir == LEFT ) _child = sol.Btree[_id2].left;
    else _child = sol.Btree[_id2].right;
    connect( _id2, _id1, _dir );
    connect( _id1, _child, _dir );
}

void FP::swap2nodes ( short _id1, short _id2 )
{
    changedModules.clear();
    if ( sol.root == _id1 ) {
        chagedRoot = sol.root;
        sol.root = _id2;    
    } else if ( sol.root == _id2 )
    {
        chagedRoot = sol.root;
        sol.root = _id1; 
    }
    else chagedRoot = NIL;

    // check all six node
    changedModules.push_back( sol.Btree[_id1] );
    changedModules.push_back( sol.Btree[_id2] );

    short _l1, _r1, _p1, _l2, _r2, _p2;
    bool _dir1, _dir2;
    _l2 = changedModules[1].left;
    _r2 = changedModules[1].right;
    _p2 = changedModules[1].parent;
    _dir2 = ( sol.Btree[sol.Btree[_id2].parent].right == _id2 );
    _l1 = changedModules[0].left;
    _r1 = changedModules[0].right;
    _p1 = changedModules[0].parent;
    _dir1 = ( sol.Btree[sol.Btree[_id1].parent].right == _id1 );

    if ( _l2 != NIL ) changedModules.push_back( sol.Btree[_l2] );
    if ( _r2 != NIL ) changedModules.push_back( sol.Btree[_r2] );
    if ( _p2 != NIL ) changedModules.push_back( sol.Btree[_p2] );
    if ( _l1 != NIL ) changedModules.push_back( sol.Btree[_l1] );
    if ( _r1 != NIL ) changedModules.push_back( sol.Btree[_r1] );
    if ( _p1 != NIL ) changedModules.push_back( sol.Btree[_p1] );

    connect( _id1, _l2, LEFT );
    connect( _id1, _r2, RIGHT );
    connect( _p2, _id1, _dir2 );

    connect( _id2, _l1, LEFT );
    connect( _id2, _r1, RIGHT );
    connect( _p1, _id2, _dir1 );
}

void FP::connect ( short _parent, short _child, bool _dir )
{
    if ( _parent != NIL )
    {
        if ( _dir == LEFT ) sol.Btree[_parent].left = _child;
        else sol.Btree[_parent].right = _child;
    }
    if ( _child != NIL ) sol.Btree[_child].parent = _parent;
}

short FP::modifyContour ( short _x1, short _x2, short _h )
{
    auto _left = contours.begin(), _right = contours.end();
    short _maxHeight = 0;

    // find range
    for ( auto it = contours.begin(); it != contours.end(); it++ )
    {
        if ( it->x <= _x1 ) _left = it;
        else if ( it->x > _x1 && it->x < _x2 )
        {
            if ( it->y > _maxHeight ) _maxHeight = it->y;
        }
        else if ( it->x == _x2 )
        {
            _right = ++it;
            break;
        }
        else
        {
            _right = it;
            break;
        }        
    }
    
    // update
    short _newHeight = _maxHeight + _h;
    contours.erase( _left, _right );
    _left = --_right;
    ++_right;
    if ( _left->x != _x1 || _left->y != _newHeight ) contours.insert( _right, Coordinate( _x1, _newHeight ) );
    if ( _right->x != _x2 || _right->y != _newHeight )contours.insert( _right, Coordinate( _x2, _newHeight ) );
    if ( _right->x != _x2 || _right->y < _maxHeight ) contours.insert( _right, Coordinate( _x2, _maxHeight ) );
    return _maxHeight;
}

void FP::keepBest ()
{
    bestSol.Btree = sol.Btree;
    bestSol.cost = preCost = bestCost = sol.cost;
    bestSol.root = sol.root;
}

void FP::restorePrev ()
{
    if ( chagedRoot != NIL ) sol.root = chagedRoot;
    for ( short i = 0; i < changedModules.size(); i++ )
    {
        sol.Btree[changedModules[i].id] = changedModules[i];
    }
}

void FP::restoreBest ()
{
    sol.Btree = bestSol.Btree;
    sol.cost = preCost = bestCost = bestSol.cost;
    sol.root = bestSol.root;
}

bool FP::TLE ()
{

}

bool FP::converged ()
{

}

FP::~FP ()
{
    for ( auto it = nets.cbegin(); it != nets.cend(); ++it ) delete *it; // net contains module and terminal
    for ( auto it = moduleMap.cbegin(); it != moduleMap.cend(); ++it ) delete it->second;
    for ( auto it = terminalMap.cbegin(); it != terminalMap.cend(); ++it ) delete it->second;
}

void FP::parseModule ( char *_filename )
{
    ifstream _f;
    string _s, _s2;
    short _num1, _num2, _current_node = 0;
    Module* _b;
    Terminal* _t;

    _f.open( _filename );
    sol.root = 0;

    _f >> _s >> outlineWidth >> outlineHeight;
    _f >> _s >> numBLocks;
    _f >> _s >> numTerminals;

    for ( short i = 0; i < numBLocks; i++ )
    {
        _f >> _s >> _num1 >> _num2;
        _b = new Module;
        _b->set( _s, i, _num1, _num2 );
        moduleMap.insert( make_pair( _s, _b ) );
        if ( i != 0 ) 
        {
            _b->parent = _current_node;
            if ( sol.Btree[_current_node].left == NIL ) sol.Btree[_current_node].left = i;
            else
            {
                sol.Btree[_current_node].right = i;
                _current_node++;
            }
        } else _b->parent = NIL;
        sol.Btree.push_back( *_b );
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

void FP::parseNets ( char *_filename )
{
    ifstream _f;
    string _s;
    short _num;
    Net* _n;
    map<string, Module*>::const_iterator _moduleIt;
    map<string, Terminal*>::const_iterator _terminalIt;

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
            if ( _moduleIt != moduleMap.end() ) _n->connect( _moduleIt->second->id );
            else if ( _terminalIt != terminalMap.end() ) _n->connect( _terminalIt->second );
            else cout << "error\n";
        }
        nets.push_back( _n );
    }
    
    _f.close();
}

bool rand_bool()
{
	return bool( rand()%2 == 0 );
}

double rand_01()
{
  return double( rand()%10000 ) / 10000;
}