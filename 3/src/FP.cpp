#include "FP.h"

extern double alpha;
double avgDelta;
double P, T1, T, cool;
double preCost, bestCost, deltaCost, convergeCost;
double convergedRatio;
double OutlineRatio, OLCostWeight, AWCostWeight; // for penalty of W/H ratio
double acceptProb;
int c, k;
int iterations;
int success, failure;
double avgAWCost, avgOLCost;   // normalize
short maxX, maxY; // check feasible and normalize
double Area, WireLength, OutlineCost; // normolize
time_t timer;
bool bestFeasible;
Solution sol;

// for restrore previous
vector<Module> changedModules;
short chagedRoot;

void FP::floorplan ()
{
    cout << "random place\n";
    //printTree();
    // -----------------------------init-----------------------------
    P = 0.8;
    cool = 0.000001;
    sol.cost = preCost = bestCost = numeric_limits<double>::max();
    avgAWCost = avgOLCost = 1;
    OutlineRatio = double( outlineHeight ) / double( outlineWidth );
    convergedRatio = 0.99;
    OLCostWeight = 0.5;
    AWCostWeight = 1 - OLCostWeight;
    c = max( 100-numBLocks, 10 );
    k = max( 2, numBLocks/11 );
    iterations = numBLocks * 2 + 20;
    success = failure = 0;
    coords.resize( numBLocks, Coordinate(0, 0) );
    contourIts.resize( numBLocks );
    bestFeasible = false;
    avgDelta = randomPlace();
    T1 = abs( avgDelta / log( 1/P ) );
    // --------------------------------------------------------------

    cout << "avgDelta " << avgDelta << endl;
    cout << "avgAWCost: " << avgAWCost << " avgOLCost " << avgOLCost << endl;

    // first stageF
    cout << "first stage\n";
    T = T1;
    anneal( 1, 1 );

    cout << "second stage\n";
    // second stage
    for (int i = 2; i <= k; i++)
    {
        //T = abs( T1 * deltaCost / i / c );
        T = abs( avgDelta / log( 1/0.3 ) );
        avgDelta = 0;
        anneal( i, 2 );
    }

    cout << "third stage\n";
    // third stage
    for (int i = k + 1; ; i++)
    {
        convergeCost = bestCost;
        //T = abs( T1 * avgDelta / i );
        T = 0.85 * T;
        if ( !bestFeasible ) T = abs(avgDelta / log( 1*i/P ));
        avgDelta = 0;
        anneal( i, 3 );
        if ( bestFeasible && ( T < cool || TLE() || converged()) ) return;
    }
    
}

void FP::anneal ( int r, short stage )
{
    for (int i = 0; i < iterations; i++)
    {
        perturb();
        pack();
        sol.cost = getCost();
        deltaCost = sol.cost - preCost;
        avgDelta = ( avgDelta * i + deltaCost ) / ( i + 1 );
        acceptProb = 1 / ( exp( deltaCost / T ) );
        acceptProb = acceptProb > 1 ? 1 : acceptProb;
        if ( sol.cost < bestCost && bestFeasible != 1 ) keepBest();
        if ( isFeasible() )
        {
            success ++;
            if ( sol.cost < bestCost ) keepBest();
            if ( !bestFeasible ) cout << "FEASIBLE!!!\n";
            bestFeasible = true;            
        } else failure++;
        if ( deltaCost < 0 || rand_01() < acceptProb ) 
        {
            preCost = sol.cost;
            cout << "Take acceptProb = " << acceptProb << " cost = " << sol.cost << endl;
        }
        else 
        {
            restorePrev();
            cout << "Discard acceptProb = " << acceptProb << " cost = " << sol.cost << endl;
        }
        modifyWeight();
        //if ( stage == 2 ) T = abs( T * 0.9 );
        //else if ( stage == 3 ) {
        //    //T = abs( T * 0.9 );
        //    if ( !bestFeasible ) T = abs(avgDelta / log( 1*r/P ));
        //}
    }
    restoreBest();
    pack();
    cout << "Restore best, cost = " << sol.cost << " Outline = (" << maxX << ", " << maxY << ")\n";
    //cout << "avgDelta: " << avgDelta << " Temparature: " << setprecision(5) << fixed << T << endl;
}

double FP::randomPlace ()
{
    double _sumDelta = 0;
    pack();
    preCost = sol.cost = getCost();
    keepBest();

    // for normalize
    double _sumArea = 0, _sumWirelength = 0;
    double _sumRatio = 0;
    for (int i = 0; i < iterations * 2; i++)
    {
        perturb();
        //printTree();
        pack();
        sol.cost = getCost();
        _sumArea += Area;
        _sumWirelength += WireLength;
        _sumRatio += OutlineCost;
        if ( sol.cost < bestCost ) keepBest();
    }
    avgAWCost = ( alpha * _sumArea + (1 - alpha) * _sumWirelength ) / ( iterations * 2 );
    avgOLCost = _sumRatio / ( iterations * 2 );
    //restoreBest();

    // for avgdelta
    preCost = 1;
    for (int i = 0; i < iterations; i++)
    {
        perturb();
        pack();
        sol.cost = getCost();
        deltaCost = sol.cost - preCost;
        preCost = sol.cost;
        _sumDelta += deltaCost;
        if ( sol.cost < bestCost ) keepBest();
    }
    restoreBest();
    return _sumDelta / ( iterations );
}

void FP::perturb ()
{
    int _choice = rand() % 3;
    switch ( _choice )
    {
        case 0:
            rotate();
            break;
        case 1:
            delete_insert();
            break;
        case 2:
            swap2nodes();
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
        list<short>::iterator _it;
        if ( m.parent != NIL )
        {
            _it = contourIts[m.parent];
            if ( sol.Btree[ m.parent ].left == m.id )
            {
                coords[ m.id ].x = coords[ m.parent ].x + sol.Btree[ m.parent ].width;
                coords[ m.id ].y = modifyContour( ++_it, m.id );     
            } else
            {
                coords[ m.id ].x = coords[ m.parent ].x;
                coords[ m.id ].y = modifyContour( _it, m.id );     
            }
        } else
        {
            coords[ m.id ].x = 0;
            coords[ m.id ].y = 0;
            contours.clear();
            contours.push_back( m.id );
            contourIts[m.id] = contours.begin();
        }
        //cout << m.id << " " << coords[m.id].x << " " << coords[m.id].y << " " << coords[m.id].x + sol.Btree[m.id].width << " " << coords[m.id].y + sol.Btree[m.id].height << endl;
        if ( m.right != NIL ) nodeStack.push( m.right );
        if ( m.left != NIL ) nodeStack.push( m.left );  
    }
    
}

double FP::getCost ()
{
    maxX = maxY = 0;
    for ( short _id = 0; _id < numBLocks; _id++ )
    {
        short _contourX, _contourY;
        _contourX = coords[_id].x + sol.Btree[_id].width;
        _contourY = coords[_id].y + sol.Btree[_id].height;
        if ( _contourX > maxX ) maxX = _contourX;
        if ( _contourY > maxY ) maxY = _contourY;
    }
    Area = maxX * maxY;
    
    double _left, _right, _bottom, _top, _x, _y;
    WireLength = 0;
    for ( auto _n = nets.begin(); _n != nets.end(); _n++ )
    {
        _left = _bottom = numeric_limits<double>::max();
        _right = _top = 0;
        for (auto _m = (*_n)->moduleIDs.begin(); _m != (*_n)->moduleIDs.end(); _m++)
        {
            _x = coords[*_m].x + double( sol.Btree[*_m].width ) / 2;
            _y = coords[*_m].y + double( sol.Btree[*_m].height ) / 2;
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
        WireLength += ( _right - _left ) + ( _top - _bottom );
    }

    OutlineCost = double( maxY ) / double(  maxX ) - OutlineRatio;
    //OutlineCost = abs(OutlineCost);
    OutlineCost *= OutlineCost;
    OutlineCost /= avgOLCost;
    double _awcost = ( alpha * Area + ( 1 - alpha ) * WireLength ) / avgAWCost; 
    cout << double( maxY ) / double(  maxX ) << " " << OutlineRatio << endl;
    //cout << _awcost << " " << OutlineCost << endl;
    return AWCostWeight * _awcost + OLCostWeight * OutlineCost;
}

bool FP::isFeasible ()
{
    return maxX <= outlineWidth && maxY <= outlineHeight;
}

void FP::modifyWeight ()
{
    // need further tuned
    if ( failure == 0 ) OLCostWeight *= 0.99;
    else OLCostWeight *= double(failure) / double( failure+success );
}

void FP::rotate ()
{
    short _id;
    changedModules.clear();
    chagedRoot = NIL; 
    do
    {
        _id = rand() % numBLocks;
    } while ( sol.Btree[_id].width > outlineHeight || sol.Btree[_id].height > outlineWidth );
    sol.Btree[_id].rotate();
    //cout << "rotate module " << _id << endl;
    changedModules.push_back( sol.Btree[_id] );

}

void FP::delete_insert ()
{
    short _id1 = rand() % numBLocks; 
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
            if ( _parent == NIL ) sol.root = _child;
            break;
        }

        _dir = rand_bool();
        if ( _dir == LEFT ) _child = _lchild;
        else _child = _rchild;
        connect( _parent, _child, _from );
        if ( _parent == NIL ) sol.root = _child;

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
    short _id2;
    _dir = rand_bool();
    do
    {
        _id2 = rand() % numBLocks;
    } while ( _id2 == _id1 );

    //cout << "delete module " << _id1 << " ,insert at " << _id2 << endl;

    if ( _dir == LEFT ) _child = sol.Btree[_id2].left;
    else _child = sol.Btree[_id2].right;
    connect( _id2, _id1, _dir );
    connect( _id1, _child, _dir );
}

void FP::swap2nodes ()
{
    short _id1, _id2;
    do
    {
        _id1 = rand() % numBLocks;
        _id2 = rand() % numBLocks;
    } while ( _id1 == _id2 );
    
    //cout << "swap " << _id1 << " and " << _id2 << endl;

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
    
    // 2 connected nodes
    if ( _p1 == _id2 )
    {
        _p1 = _id1;
        if ( _dir1 == LEFT ) _l2 = _id2;
        else _r2 = _id2;
    } else if ( _p2 == _id1 )
    {
        _p2 = _id2;
        if ( _dir2 == LEFT ) _l1 = _id1;
        else _r1 = _id1;
    } 

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

short FP::modifyContour ( list<short>::iterator _startIt, short _id )
{
    auto _left = _startIt, _right = _startIt;
    short _maxHeight = 0, _x1 = coords[_id].x, _x2 = coords[_id].x+sol.Btree[_id].width;

    // find range
    for ( auto it = _startIt; it != contours.end(); it++ )
    {
        short _contourId = *it;
        if ( coords[_contourId].y + sol.Btree[_contourId].height > _maxHeight ) _maxHeight = coords[_contourId].y + sol.Btree[_contourId].height;
        if ( !( coords[_contourId].x + sol.Btree[_contourId].width <= _x2 ) ) 
        {
            _right = it;
            break;
        }       
    }
    // update
    contours.erase( _left, _right );
    contourIts[_id] = contours.insert(_right, _id);
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
    return time(NULL) - timer > 3300;
}

bool FP::converged ()
{
    return bestCost / convergeCost < convergedRatio;
}

FP::~FP ()
{
    for ( auto it = nets.cbegin(); it != nets.cend(); ++it ) delete *it; // net contains module and terminal
    for ( auto it = moduleMap.cbegin(); it != moduleMap.cend(); ++it ) delete it->second;
    for ( auto it = terminalMap.cbegin(); it != terminalMap.cend(); ++it ) delete it->second;
}

void FP::parseModule ( char *_filename )
{
    timer = time(NULL);
    
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
        _b->set( i, _num1, _num2 );
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
        names.push_back(_s);
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

void FP::writeFile ( char *_filename )
{
    ofstream _of;
    restoreBest();
    pack();
    sol.cost = getCost();

    _of.open( _filename );

    _of << setprecision(5) << fixed << alpha * Area + ( 1 - alpha ) * WireLength << endl;
    _of << setprecision(5) << fixed << WireLength << endl;
    _of << setprecision(5) << fixed << Area << endl;
    _of << maxX << " " << maxY << endl;
    _of << time(NULL) - timer << endl;
    for ( short i = 0; i < numBLocks; ++i )
    {
        _of << names[i] << " " << coords[i].x << " " << coords[i].y << " " << coords[i].x + sol.Btree[i].width << " " << coords[i].y + sol.Btree[i].height << endl;
    }
} 

bool rand_bool ()
{
	return bool( rand()%2 == 0 );
}

double rand_01 ()
{
  return double( rand()%10000 ) / 10000;
}

void FP::printTree ()
{
    stack<short> nodeStack;

    nodeStack.push( sol.root );

    while ( !nodeStack.empty() )
    {
        Module &m = sol.Btree[ nodeStack.top() ];
        nodeStack.pop();

        cout << " Node: " << m.id;        
        cout << " parent: " << m.parent;        
        cout << " left: " << m.left;        
        cout << " right: " << m.right;
        cout << endl;        
        if ( m.right != NIL ) nodeStack.push( m.right );
        if ( m.left != NIL ) nodeStack.push( m.left );  
    }
}

void FP::printPacking ()
{
    for ( short i = 0; i < numBLocks; ++i )
    {
        cout << names[i] << " " << coords[i].x << " " << coords[i].y << " " << coords[i].x + sol.Btree[i].width << " " << coords[i].y + sol.Btree[i].height << endl;
    }
}