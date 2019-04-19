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
        pair<const Pos*, bool> _boundInfo = tile2Boundary( make_pair( _ca.GetGx1(), _ca.GetGy1() ), make_pair( _ca.GetGx2(), _ca.GetGy2() ) );
        if ( _boundInfo.second == LAYER::HORI ) hCapacity[_boundInfo.first->first][_boundInfo.first->second] = _ca.GetReduceCapacity() >> 1;
        else vCapacity[_boundInfo.first->first][_boundInfo.first->second] = _ca.GetReduceCapacity() >> 1;
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

pair<const Pos*, bool> Grid::tile2Boundary( const Pos &_pin1, const Pos &_pin2 )
{
    bool layer;
    if ( _pin1.first == _pin2.first ) 
    {
        layer = LAYER::VERTI;
        if ( _pin1.second < _pin2.second ) return make_pair(&_pin1, layer);
        else return make_pair(&_pin2, layer);
    }
    else
    {
        layer = LAYER::HORI;
        if ( _pin1.first < _pin2.first ) return make_pair(&_pin1, layer);
        else return make_pair(&_pin2, layer);
    }

}

//======================router======================

router::router ()
{
    grid = new Grid;
}

router::~router ()
{
    delete grid;
}

void router::runDijstra ()
{
    netOrdering();
}

void router::netOrdering ()
{
    for (int i = 0; i < db.GetNetNo(); i++)
    {
	    Net& n = db.GetNetByPosition(i);
        orderedNets.insert( make_pair( getNetOrder(n), &n ) );
    }
}

int router::getNetOrder ( Net &n )
{
    // left right top bottom
    int _left, _right, _top, _bottom;
    int _x1, _y1, _x2, _y2;
    _right = _top = 0;
    _left = db.GetHoriGlobalTileNo();
    _bottom = db.GetVertiGlobalTileNo();
    for (int i = 0; i < n.GetSubNetNo(); i++)
    {
        SubNet& _sn = n.GetSubNet(i);
        _x1 = _sn.GetSourcePinGx();
        _y1 = _sn.GetSourcePinGy();
        _x2 = _sn.GetTargetPinGx();
        _y2 = _sn.GetTargetPinGy();
        
        if ( _x1 < _left ) _left = _x1;
        if ( _x1 > _right ) _right = _x1;
        if ( _x2 < _left ) _left = _x2;
        if ( _x2 > _right ) _right = _x2;
        if ( _y1 < _bottom ) _bottom = _y1;
        if ( _y1 > _top ) _top = _y1;
        if ( _y2 < _bottom ) _bottom = _y2;
        if ( _y2 > _top ) _top = _y2;
    }

    return ( _top - _bottom ) + ( _right - _left );    
}