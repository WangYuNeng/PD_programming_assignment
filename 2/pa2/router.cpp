#include "router.h"

//======================Grid======================

Grid::Grid ( RoutingDB &db )
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
        pair<Pos, bool> _boundInfo = tile2Boundary( make_pair( _ca.GetGx1(), _ca.GetGy1() ), make_pair( _ca.GetGx2(), _ca.GetGy2() ) );
        if ( _boundInfo.second == LAYER::HORI ) hCapacity[_boundInfo.first.first][_boundInfo.first.second] = _ca.GetReduceCapacity() >> 1;
        else vCapacity[_boundInfo.first.first][_boundInfo.first.second] = _ca.GetReduceCapacity() >> 1;
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

pair<Pos, bool> Grid::tile2Boundary( const Pos &_pin1, const Pos &_pin2 )
{
    bool layer;
    if ( _pin1.first == _pin2.first ) 
    {
        layer = LAYER::VERTI;
        if ( _pin1.second < _pin2.second ) return make_pair(_pin1, layer);
        else return make_pair(_pin2, layer);
    }
    else
    {
        layer = LAYER::HORI;
        if ( _pin1.first < _pin2.first ) return make_pair(_pin1, layer);
        else return make_pair(_pin2, layer);
    }

}

