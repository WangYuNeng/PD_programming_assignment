#include "router.h"

extern RoutingDB db;

//======================Grid======================

Grid::Grid ()
{
    vtiles = db.GetVertiGlobalTileNo();
    htiles = db.GetHoriGlobalTileNo();

    int _vcap = db.GetLayerVertiCapacity(1), _hcap = db.GetLayerHoriCapacity(0);
    // size not exactly fit
    // capacity of boundary for wire = _cap / 2
    vCapacity = new int* [htiles];
    hCapacity = new int* [htiles];
    vLoad = new int* [htiles];
    hLoad = new int* [htiles];
    for (int i = 0; i < htiles; i++)
    {
        vCapacity[i] = new int [vtiles];
        hCapacity[i] = new int [vtiles];
        vLoad[i] = new int [vtiles];
        hLoad[i] = new int [vtiles];
        for (int j = 0; j < vtiles; j++)
        {
            vCapacity[i][j] = _vcap >> 1;
            hCapacity[i][j] = _hcap >> 1;
            vLoad[i][j] = 0;
            hLoad[i][j] = 0;
        }
    }
    
    for (int i = 0; i < db.GetCapacityAdjustNo(); i++)
    {
        CapacityAdjust& _ca = db.GetCapacityAdjust(i);
        pair<const Vertice*, bool> _boundInfo = tile2Boundary( &Vertice( _ca.GetGx1(), _ca.GetGy1() ), &Vertice( _ca.GetGx2(), _ca.GetGy2() ) );
        if ( _boundInfo.second == LAYER::HORI ) hCapacity[_boundInfo.first->x][_boundInfo.first->y] = _ca.GetReduceCapacity() >> 1;
        else vCapacity[_boundInfo.first->x][_boundInfo.first->y] = _ca.GetReduceCapacity() >> 1;
    }
    
}

Grid::~Grid()
{
    for (int i = 0; i < htiles; i++)
    {
        delete [] vCapacity[i];
        delete [] hCapacity[i];
        delete [] vLoad[i];
        delete [] hLoad[i];
    }
    delete [] vCapacity;
    delete [] hCapacity;
    delete [] vLoad;
    delete [] hLoad;
}

void Grid::showInfo ()
{
    cout << "\nvertical capacity:";
    for (int i = vtiles-2; i >=0 ; i--)
    {
        cout << "\n";
        for (int j = 0; j < htiles; j++)
        {
            cout << vCapacity[j][i] << " ";
        }
    }
    cout << "\nvertical load:";
    for (int i = vtiles-2; i >=0 ; i--)
    {
        cout << "\n";
        for (int j = 0; j < htiles; j++)
        {
            cout << vLoad[j][i] << " ";
        }
    }
    cout << "\nhorizontal capacity:";
    for (int i = vtiles-1; i >=0 ; i--)
    {
        cout << "\n";
        for (int j = 0; j < htiles-1; j++)
        {
            cout << hCapacity[j][i] << " ";
        }
    }
    cout << "\nhorizontal load:";
    for (int i = vtiles-1; i >=0 ; i--)
    {
        cout << "\n";
        for (int j = 0; j < htiles-1; j++)
        {
            cout << hLoad[j][i] << " ";
        }
    }
    cout << endl;
}

int Grid::getCapacity ( const Vertice* _pin1, const Vertice* _pin2 )
{
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) return hCapacity[_boundInfo.first->x][_boundInfo.first->y];
    else return vCapacity[_boundInfo.first->x][_boundInfo.first->y];
}

int Grid::getLoad ( const Vertice* _pin1, const Vertice* _pin2 )
{
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) return hLoad[_boundInfo.first->x][_boundInfo.first->y];
    else return vLoad[_boundInfo.first->x][_boundInfo.first->y];
}

void Grid::incrLoad ( const Vertice* _pin1, const Vertice* _pin2 )
{
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) ++hCapacity[_boundInfo.first->x][_boundInfo.first->y];
    else ++vCapacity[_boundInfo.first->x][_boundInfo.first->y];
}


pair<const Vertice*, bool> Grid::tile2Boundary( const Vertice* _pin1, const Vertice* _pin2 )
{
    bool layer;
    if ( _pin1->x == _pin2->x ) 
    {
        layer = LAYER::VERTI;
        if ( _pin1->y < _pin2->y ) return make_pair(_pin1, layer);
        else return make_pair(_pin2, layer);
    }
    else
    {
        layer = LAYER::HORI;
        if ( _pin1->x < _pin2->y ) return make_pair(_pin1, layer);
        else return make_pair(_pin2, layer);
    }

}

//======================DijstraMap======================

DijstraMap::DijstraMap ()
{
    int _vtiles = db.GetVertiGlobalTileNo();
    int _htiles = db.GetHoriGlobalTileNo();

    parent = new int* [_htiles];
    connected = new bool* [_htiles];
    distance = new double* [_htiles];
    for (int i = 0; i < _htiles; i++)
    {
        parent[i] = new int [_vtiles];
        connected[i] = new bool [_vtiles];
        distance[i] = new double [_vtiles];
        for (int j = 0; j < _vtiles; j++)
        {
            parent[i][j] = 0;
            connected[i][j] = 0;
            distance[i][j] = 0;
        }
    }
}

DijstraMap::~DijstraMap ()
{
    for (int i = 0; i < db.GetHoriGlobalTileNo(); i++)
    {
        delete [] parent[i];
        delete [] connected[i];
        delete [] distance[i];
    }
    delete [] parent;
    delete [] connected;
    delete [] distance;
}

void DijstraMap::BackTrace ( Vertice* _p )
{
    short _x = _p->x, _y = _p->y;
    
    while ( true )
    {
        connected[_x][_y] = true;
        cout << _x << " " << _y << endl;
        if ( distance[_x][_y] == 0 ) break;
        switch (parent[_x][_y])
        {
            case UP:
                _y--;
                break;
            case DOWN:
                _y++;
                break;
            case LEFT:
                _x++;
                break;
            case RIGHT:
                _x--;
                break;
        }
    }
}

void DijstraMap::refreshConnection ()
{ 
    for (int i = 0; i < db.GetHoriGlobalTileNo(); i++)
    {
        for (int j = 0; i < db.GetVertiGlobalTileNo(); j++)
        {
            connected[i][j] = false;
        }
    }
}

void DijstraMap::refreshDistance ()
{
    for (int i = 0; i < db.GetHoriGlobalTileNo(); i++)
    {
        for (int j = 0; i < db.GetVertiGlobalTileNo(); j++)
        {
            distance[i][j] = numeric_limits<double>::max();
        }
    }
}

//======================Router======================

Router::Router ()
{
    grid = new Grid;
    dMap = new DijstraMap;
}

Router::~Router ()
{
    delete grid;
    delete dMap;
}

void Router::routeAll ()
{
    netOrdering();
    for (map<int, Net*>::iterator it = orderedNets.begin(); it != orderedNets.end(); ++it)
    {
        map<int, SubNet*> _orderedSubnet;
        Net* _n = it->second;
        for (int i = 0; i < _n->GetSubNetNo(); i++)
        {
            SubNet& _sn = _n->GetSubNet(i);
            _orderedSubnet.insert( make_pair( getSubnetOrder( _sn ), &_sn ) );
        }
        
        for (map<int, SubNet*>::iterator subIt = _orderedSubnet.begin(); subIt != _orderedSubnet.end(); ++subIt)
        {
            SubNet* _sn = subIt->second;
            runDijstra( _sn );
            dMap->BackTrace( &Vertice( _sn->GetTargetPinGx(), _sn->GetTargetPinGy() ) );
        }

        dMap->refreshConnection();
        _orderedSubnet.clear();
    }
    
}

void Router::runDijstra ( SubNet* _sn )
{
    minHeap _distanceHeap;
    Vertice* _v;
    short _targetX = _sn->GetTargetPinGx(), _targetY = _sn->GetSourcePinGy();

    // initialize single source
    dMap->refreshDistance();
    _v = new Vertice( _sn->GetSourcePinGx(), _sn->GetSourcePinGy(), 0 );
    dMap->setDistance( _v );
    relax( _v , _distanceHeap );

    while(true){
        _v = _distanceHeap.top();
        _distanceHeap.pop();

        if ( _v->x == _targetX && _v->y == _targetY ){
            while ( !_distanceHeap.empty() )
            {
                _v = _distanceHeap.top();
                delete _v;
                _distanceHeap.pop();
            }
            break;
        }
        
        if ( dMap->isMinDistance( _v ) ) { // old vertice(substitute for Decrease-Key)
            continue;
        }
        
        relax( _v, _distanceHeap );
        delete _v;
    }
}

void Router::relax ( Vertice* _p, minHeap& _mheap )
{
    double _newDistance;
    Vertice* _toVertice;

    for(int d = UP; d < NONE; ++d)
    {
        _toVertice = toVertice( _p, d );
        if ( _toVertice != NULL ) {
            _newDistance = _p->distance + getWeight( _p, _toVertice );
            update( _p, _toVertice, _newDistance, _mheap);
            dMap->setParent( _toVertice, d );
        }
    }
}

Vertice* Router::toVertice( Vertice* _p, int _direction )
{
    Vertice* _v;
    int _x, _y;
    switch (_direction)
    {
    case UP:
        _x = _p->x;
        _y = _p->y + 1;
        break;
    case DOWN:
        _x = _p->x;
        _y = _p->y - 1;
        break;
    case LEFT:
        _x = _p->x - 1;
        _y = _p->y;
        break;
    case RIGHT:
        _x = _p->x + 1;
        _y = _p->y;
        break;
    }
    if ( _x >= 0 && _y >= 0 && _x < db.GetHoriGlobalTileNo() && _y < db.GetVertiGlobalTileNo() ) return new Vertice( _x, _y );
    else return NULL;
}

double Router::getWeight( Vertice* _pFrom, Vertice* _pTo )
{
    // reuse walked path
    if ( dMap->isConnected( _pTo ) ) return 0;

    int _cap = grid->getCapacity( _pFrom, _pTo );
    int _load = grid->getLoad( _pFrom, _pTo );
    double ratio = _load*1.0 / _cap*1.0;
    return pow( 2, ratio );
}

void Router::update ( Vertice* _pFrom, Vertice* _pTo, double _newDist, minHeap& _mheap )
{
    _pTo->distance = _newDist;
    _mheap.push(_pTo);
    dMap->setDistance( _pTo );
}

void Router::netOrdering ()
{
    for (int i = 0; i < db.GetNetNo(); i++)
    {
	    Net& _n = db.GetNetByPosition(i);
        orderedNets.insert( make_pair( getNetOrder(_n), &_n ) );
    }
}

int Router::getNetOrder ( Net &_n )
{
    int _x1, _y1, _x2, _y2;
    int _sum = 0;
    for (int i = 0; i < _n.GetSubNetNo(); i++)
    {
        SubNet& _sn = _n.GetSubNet(i);
        _sum += getSubnetOrder( _sn );
    }
    return _sum / _n.GetSubNetNo();    
}

int Router::getSubnetOrder ( SubNet &_sn )
{
    int _x1, _y1, _x2, _y2;
    _x1 = _sn.GetSourcePinGx();
    _y1 = _sn.GetSourcePinGy();
    _x2 = _sn.GetTargetPinGx();
    _y2 = _sn.GetTargetPinGy();
    return getManhattanDist( _x1, _y1, _x2, _y2 );
}

int Router::getManhattanDist ( short _x1, short _y1, short _x2, short _y2 )
{
    int _deltaX, _deltaY;
    _deltaX = ( _x1 > _x2 ) ? _x1 - _x2 : _x2 - _x1;
    _deltaY = ( _y1 > _y2 ) ? _y1 - _y2 : _y2 - _y1;
    return _deltaX + _deltaY;
}