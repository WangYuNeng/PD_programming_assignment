#include "router.h"

extern RoutingDB db;

//======================DijstraMap======================

DijstraMap::DijstraMap ()
{
    int _vtiles = db.GetVertiGlobalTileNo();
    int _htiles = db.GetHoriGlobalTileNo();
    int _vcap = db.GetLayerVertiCapacity(1), _hcap = db.GetLayerHoriCapacity(0);

    parent = new int* [_htiles];
    vConnected = new bool* [_htiles];
    hConnected = new bool* [_htiles];
    viaConnected = new bool* [_htiles];
    distance = new double* [_htiles];

    // size not exactly fit
    // capacity of boundary for wire = _cap / 2
    vCapacity = new int* [_htiles];
    hCapacity = new int* [_htiles];
    vLoad = new int* [_htiles];
    hLoad = new int* [_htiles];

    for (int i = 0; i < _htiles; i++)
    {
        parent[i] = new int [_vtiles];
        vConnected[i] = new bool [_vtiles];
        hConnected[i] = new bool [_vtiles];
        viaConnected[i] = new bool [_vtiles];
        distance[i] = new double [_vtiles];

        vCapacity[i] = new int [_vtiles];
        hCapacity[i] = new int [_vtiles];
        vLoad[i] = new int [_vtiles];
        hLoad[i] = new int [_vtiles];

        for (int j = 0; j < _vtiles; j++)
        {
            parent[i][j] = 0;
            vConnected[i][j] = 0;
            hConnected[i][j] = 0;
            viaConnected[i][j] = 0;
            distance[i][j] = 0;

            vCapacity[i][j] = _vcap >> 1;
            hCapacity[i][j] = _hcap >> 1;
            vLoad[i][j] = 0;
            hLoad[i][j] = 0;
        }
    }
    
    for (int i = 0; i < db.GetCapacityAdjustNo(); i++)
    {
        CapacityAdjust& _ca = db.GetCapacityAdjust(i);
        Vertice _v1 = Vertice( _ca.GetGx1(), _ca.GetGy1() ), _v2 = Vertice( _ca.GetGx2(), _ca.GetGy2() );

        pair<const Vertice*, bool> _boundInfo = tile2Boundary( &_v1, &_v2 );
        if ( _boundInfo.second == LAYER::HORI ) hCapacity[_boundInfo.first->x][_boundInfo.first->y] = _ca.GetReduceCapacity() >> 1;
        else vCapacity[_boundInfo.first->x][_boundInfo.first->y] = _ca.GetReduceCapacity() >> 1;
    }
    
}

DijstraMap::~DijstraMap ()
{
    for (int i = 0; i < db.GetHoriGlobalTileNo(); i++)
    {
        delete [] parent[i];
        delete [] vConnected[i];
        delete [] hConnected[i];
        delete [] viaConnected[i];
        delete [] distance[i];

        delete [] vCapacity[i];
        delete [] hCapacity[i];
        delete [] vLoad[i];
        delete [] hLoad[i];
    }
    delete [] parent;
    delete [] vConnected;
    delete [] hConnected;
    delete [] viaConnected;
    delete [] distance;

    delete [] vCapacity;
    delete [] hCapacity;
    delete [] vLoad;
    delete [] hLoad;
}

int DijstraMap::getCapacity ( const Vertice* _pin1, const Vertice* _pin2 )
{
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) return hCapacity[_boundInfo.first->x][_boundInfo.first->y];
    else return vCapacity[_boundInfo.first->x][_boundInfo.first->y];
}

int DijstraMap::getLoad ( const Vertice* _pin1, const Vertice* _pin2 )
{
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) return hLoad[_boundInfo.first->x][_boundInfo.first->y];
    else return vLoad[_boundInfo.first->x][_boundInfo.first->y];
}

bool DijstraMap::isConnected ( const Vertice* _pin1, const Vertice* _pin2 )
{
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) return hConnected[_boundInfo.first->x][_boundInfo.first->y];
    else return vConnected[_boundInfo.first->x][_boundInfo.first->y];
}

void DijstraMap::refreshConnection ()
{ 
    for (int i = 0; i < db.GetHoriGlobalTileNo(); i++)
    {
        for (int j = 0; j < db.GetVertiGlobalTileNo(); j++)
        {
            vConnected[i][j] = false;
            hConnected[i][j] = false;
            viaConnected[i][j] = false;
        }
    }
}

void DijstraMap::refreshDistance ()
{
    for (int i = 0; i < db.GetHoriGlobalTileNo(); i++)
    {
        for (int j = 0; j < db.GetVertiGlobalTileNo(); j++)
        {
            distance[i][j] = numeric_limits<double>::max();
        }
    }
}

void DijstraMap::connect ( const Vertice* _pin1, const Vertice* _pin2 )
{
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) 
    {
        hConnected[_boundInfo.first->x][_boundInfo.first->y] = true;
        ++hLoad[_boundInfo.first->x][_boundInfo.first->y];
    }
    else 
    {
        vConnected[_boundInfo.first->x][_boundInfo.first->y] = true;
        ++vLoad[_boundInfo.first->x][_boundInfo.first->y];
    }
}

void DijstraMap::incrLoad ( const Vertice* _pin1, const Vertice* _pin2 )
{
    // embedded  in connect
    pair<const Vertice*, bool> _boundInfo = tile2Boundary( _pin1, _pin2 );
    if ( _boundInfo.second == LAYER::HORI ) ++hLoad[_boundInfo.first->x][_boundInfo.first->y];
    else ++vLoad[_boundInfo.first->x][_boundInfo.first->y];
}

int DijstraMap::BackTrace ( Vertice* _p, vector<string>& _output )
{
    bool _halfOutput;
    short _prevDirection, _nowDirection; // 1 for horizontal, 2 for vertital
    int _routeCount;
    int _xCoord, _yCoord, _lowX, _lowY, _tileW, _tileH;
    Vertice _v = Vertice( _p->x, _p->y );
    Vertice _parent = Vertice();
    string _tmp;

    _halfOutput = false;
    _prevDirection = 1;
    _routeCount = 0;
    _lowX = db.GetLowerLeftX();
    _lowY = db.GetLowerLeftY();
    _tileW = db.GetTileWidth();
    _tileH = db.GetTileHeight();
    

    while ( true )
    {
        _xCoord = _lowX + _v.x*_tileW + _tileW/2;
        _yCoord = _lowY + _v.y*_tileH + _tileH/2;
        if ( parent[_v.x][_v.y] == NONE ) 
        {
            if ( _halfOutput )
            {
                _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + to_string(_prevDirection);
                _output.push_back( _tmp );
                _halfOutput = false;
                ++_routeCount;
            }
            if ( _prevDirection == 2 && !isViaConnected( &_v ) )
            {
                connectVia( &_v );
                _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + "1";
                _output.push_back( _tmp );
                _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + "2";
                _output.push_back( _tmp );
                _halfOutput = false;
                ++_routeCount;
            }
            break;
        }
        _parent.x = _v.x;
        _parent.y = _v.y;
        switch (parent[_v.x][_v.y])
        {
            case UP:
                _parent.y--;
                _nowDirection = 2;
                break;
            case DOWN:
                _parent.y++;
                _nowDirection = 2;
                break;
            case LEFT:
                _parent.x++;
                _nowDirection = 1;
                break;
            case RIGHT:
                _parent.x--;
                _nowDirection = 1;
                break;
        }
        if ( !isConnected( &_v, &_parent ) )
        {
            connect(  &_v, &_parent );
            if ( _nowDirection != _prevDirection )
            {
                if ( _halfOutput )
                {
                    _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + to_string(_prevDirection);
                    _output.push_back(_tmp);
                    _halfOutput = false;
                    ++_routeCount;
                }
                if ( !isViaConnected( &_v ) )
                {
                    connectVia( &_v );
                    _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + "1";
                    _output.push_back( _tmp );
                    _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + "2";
                    _output.push_back( _tmp );
                    _halfOutput = false;
                    ++_routeCount;
                }
                _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + to_string(_nowDirection);
                _output.push_back( _tmp );
                _halfOutput = true;
            }
            if ( !_halfOutput )
            {
                _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + to_string(_prevDirection);
                _output.push_back( _tmp );
                _halfOutput = true;
            }
        }
        else
        {
            if ( _halfOutput )
            {
                _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + to_string(_prevDirection);
                _output.push_back( _tmp );
                _halfOutput = false;
                ++_routeCount;
            }
            if ( _nowDirection != _prevDirection )
            {
                if ( !isViaConnected( &_v ) )
                {
                    connectVia( &_v );
                    _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + "1";
                    _output.push_back( _tmp );
                    _tmp = to_string(_xCoord)+","+to_string(_yCoord)+"," + "2";
                    _output.push_back( _tmp );
                    _halfOutput = false;
                    ++_routeCount;
                }
            }
        }
        _v.x = _parent.x;
        _v.y = _parent.y;
        _prevDirection = _nowDirection;
    }
    return _routeCount;
}

void DijstraMap::showInfo ()
{
    int vtiles = db.GetVertiGlobalTileNo();
    int htiles = db.GetHoriGlobalTileNo();
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

pair<const Vertice*, bool> DijstraMap::tile2Boundary( const Vertice* _pin1, const Vertice* _pin2 )
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
        if ( _pin1->x < _pin2->x ) return make_pair(_pin1, layer);
        else return make_pair(_pin2, layer);
    }

}

//======================Router======================

Router::Router ()
{
    dMap = new DijstraMap;
}

Router::~Router ()
{
    delete dMap;
}

void Router::routeAll ( ofstream& _of )
{
    netOrdering();
    int _count = 0;
    for (multimap<int, Net*>::iterator it = orderedNets.begin(); it != orderedNets.end(); ++it)
    {
        multimap<int, SubNet*> _orderedSubnet;
        Net* _n = it->second;
        vector<string> _output;
        int _routeCount;
        _output.empty();
        _routeCount = 0;

        ++_count;
        cout << "Routing Net " << setw(8) << _n->GetUid() << " (" << setw(8) << _count << "/" << setw(8) << db.GetNetNo() << ")" << flush;

        for (int i = 0; i < _n->GetSubNetNo(); i++)
        {
            SubNet& _sn = _n->GetSubNet(i);
            _orderedSubnet.insert( make_pair( getSubnetOrder( _sn ), &_sn ) );
        }
        
        for (multimap<int, SubNet*>::iterator subIt = _orderedSubnet.begin(); subIt != _orderedSubnet.end(); ++subIt)
        {
            SubNet* _sn = subIt->second;
            if ( _sn->GetSourcePinGx() == _sn->GetTargetPinGx() && _sn->GetSourcePinGy() == _sn->GetTargetPinGy() ) continue;
            Vertice _v = Vertice(  _sn->GetTargetPinGx(), _sn->GetTargetPinGy() );

            runDijstra( _sn );
            _routeCount += dMap->BackTrace( &_v, _output );
        }
        cout << char(13) << setw(60) << ' ' << char(13);   
        printNet( _n, _output, _routeCount, _of );
        dMap->refreshConnection();
        _orderedSubnet.clear();
    }
    
}

void Router::runDijstra ( SubNet* _sn )
{
    minHeap _distanceHeap;
    Vertice* _v;
    short _targetX = _sn->GetTargetPinGx(), _targetY = _sn->GetTargetPinGy();

    // initialize single source
    dMap->refreshDistance();
    _v = new Vertice( _sn->GetSourcePinGx(), _sn->GetSourcePinGy(), 0 );
    dMap->setDistance( _v );
    dMap->setParent( _v, NONE );
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
        
        if ( !dMap->isMinDistance( _v ) ) { // old vertice(substitute for Decrease-Key)
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
            if ( _newDistance < dMap->getDistance( _toVertice ) )
            {
                dMap->setParent( _toVertice, d );
                update( _p, _toVertice, _newDistance, _mheap);
            }
            else delete _toVertice;
        }
    }
}

Vertice* Router::toVertice( Vertice* _p, int _direction )
{
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
    default:
        _x = _p->x;
        _y = _p->y;
        assert(0);
    }
    if ( _x >= 0 && _y >= 0 && _x < db.GetHoriGlobalTileNo() && _y < db.GetVertiGlobalTileNo() ) return new Vertice( _x, _y );
    else return NULL;
}

double Router::getWeight( Vertice* _pFrom, Vertice* _pTo )
{
    // reuse walked path
    if ( dMap->isConnected( _pFrom, _pTo ) ) return 0;

    int _cap = dMap->getCapacity( _pFrom, _pTo );
    int _load = dMap->getLoad( _pFrom, _pTo );
    double ratio = _load*1.0 / _cap*1.0;
    
    int _viacost = 0;
    bool _d1, _d2;
    if ( dMap->getParent( _pFrom ) == UP || dMap->getParent( _pFrom ) == DOWN ) _d1 = 1;
    else _d1 = 0;
    if ( dMap->getParent( _pTo ) == UP || dMap->getParent( _pTo ) == DOWN ) _d2 = 1;
    else _d2 = 0;
    _viacost = int( !(_d1 == _d2) );
    return pow( 2, ratio ) + _viacost;
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

void Router::printNet( Net* _n, vector<string>& _output, int _routeCount, ofstream& _of )
{
    bool _iseven = true;
    _of << _n->GetName() << " " << _n->GetUid() << " " << _routeCount << endl;
    for (size_t i = 0; i < _output.size(); i++)
    {
        _of << "(" << _output[i] << ")";
        if ( _iseven ) _of << "-";
        else _of << "\n";
        _iseven = !_iseven;
    }
    _of << "!" << endl;   
}