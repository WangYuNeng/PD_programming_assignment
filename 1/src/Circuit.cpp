#include "Circuit.h"

void Circuit::FM ()
{
    Cell** lockedCells = new Cell* [cells.size()];
    bool* lockedGroup = new bool [cells.size()]; 
    Cell* lockedCell;
    int gainSum, maxGainSum, terminateStep, balance;

    initialPartition();
    int ite = 0;
    while (1)
    {
        initialGain();  
        ++ite;
        //printf("%d %d\n", groups[0].getMax(), groups[1].getMax());
        //printGroups();
        gainSum = 0;
        maxGainSum = EMPTY_GROUP;
        terminateStep = -1;
        balance = 0;
        for(int i = 0; i < cells.size(); i++)
        {
            lockedCell = getMaxGainCell();
            if ( lockedCell == 0 )
            { 
                printf("ERROR");
                break;
            }
            lockedCells[i] = lockedCell;
            lockedGroup[i] = ( lockedCell->belong == &groups[1] );
            updateGain( lockedCell );
            updateSum( lockedCell, gainSum, maxGainSum, terminateStep, balance, i );
        }
        if ( maxGainSum > 0 ) bestMoves( lockedCells, lockedGroup, terminateStep );
        else break;
    }
    printf("%d\n", ite);
    bestMoves( lockedCells, lockedGroup, cells.size() );
    delete [] lockedCells;
    delete [] lockedGroup;
}

inline void Circuit::initialPartition ()
{
    int Pmax = 0;
    srand(100);
    for(Cells::iterator it = cells.begin(); it != cells.end(); ++it)
    {
        int num = (*it)->getNumTerminal();
        if ( num > Pmax ) Pmax = num;
    }
    groups = new Group [2];
    groups[0].initialize(Pmax);
    groups[1].initialize(Pmax);
    for(int i = 0; i < cells.size(); ++i)
    {
        if ( (i << 1) < cells.size() )
        //if ( rand() % 2 == 0 && groups[0].getSize() < (cells.size() >> 1) )
        {
            groups[0].addCell( cells.at(i) );
            cells.at(i)->assignGroup( &groups[0] );
        }
        else
        {
            groups[1].addCell( cells.at(i) );            
            cells.at(i)->assignGroup( &groups[1] );
        }
    }
}

inline void Circuit::initialGain ()
{
    Net * n;
    Cell* c;
    bool gFlag;
    for(Nets::iterator itNet = nets.begin(); itNet != nets.end(); ++itNet)
    {
        n = *itNet; 

        n->resetGCount(); 
        for(int i = 0; i < n->getNumTerminal(); ++i)
        {
            c = n->getTerminal(i);
            n->incrGCount( int(c->getGroup() == &groups[1]) );
        }
        if ( n->getNumTerminal() == 1 ) continue;
        if ( !( n->getGCount(0) > 1 && n->getGCount(1) > 1 ) )
        {
            for(int i = 0; i < n->getNumTerminal(); ++i)
            {
                c = n->getTerminal(i);
                if ( n->getGCount(0) == 0 ) c->decrGain();
                else if ( n->getGCount(0) == 1 ) if ( c->getGroup() == &groups[0] ) c->incrGain();
                if ( n->getGCount(1) == 0 ) c->decrGain();
                else if ( n->getGCount(1) == 1 ) if ( c->getGroup() == &groups[1] ) c->incrGain();
            }
        }
    }
    
}

inline Cell* Circuit::getMaxGainCell ()
{
    double top = (1.0 + ratio)*cells.size() / 2.0, down = (1.0 - ratio)*cells.size() / 2.0;
    double gsize[2] = {double(groups[0].getSize()), double(groups[1].getSize())};
    Group *gFrom;
    Cell *c;
    if ( gsize[0]+1.0 > top || gsize[1]-1.0 < down ) gFrom = &groups[0];
    else if ( gsize[1]+1.0 > top || gsize[0]-1.0 < down ) gFrom = &groups[1];
    else 
    {
        if ( groups[0].getMax() > groups[1].getMax() ) gFrom = &groups[0];
        else if ( groups[0].getMax() < groups[1].getMax() ) gFrom = &groups[1];
        else gFrom = &groups[int( groups[0].getSize() < groups[1].getSize() )]; // consider balance
    } 
    if ( gFrom->getMax() == EMPTY_GROUP )
    {
        return 0;
    }
    c = gFrom->getMaxCell();
    gFrom->deleteCell( c );
    return c;
}

inline void Circuit::updateGain ( Cell* c )
{
    Net* n;
    Cell* t;
    int fromIncr, toIncr;
    bool from = (&groups[1] == c->getGroup()), to = !from;
    c->assignGroup( 0 );
    groups[to].addLockedCell();
    for(int i = 0; i < c->getNumTerminal(); i++)
    {
        n = c->getTermianl(i);
        fromIncr = toIncr = 0;
        if ( n->getGCount(from) == 1 ) --toIncr;
        else if ( n->getGCount(from) == 2 ) ++fromIncr;
        if ( n->getGCount(to) == 0 ) ++fromIncr;
        else if ( n->getGCount(to) == 1 ) --toIncr;
        if ( toIncr != 0 or fromIncr !=0 )
        {
            for(int j = 0; j < n->getNumTerminal(); j++)
            {
                t = n->getTerminal(j);
                if ( t->getGroup() == 0 ) continue;
                if ( t->getGroup() == &groups[from] )
                {
                    if ( fromIncr != 0 ) t->incrGain( fromIncr );
                }
                else
                {
                    if ( toIncr != 0 ) t->incrGain( toIncr );
                }
            }
        }
        n->decrGCount(from);
        n->incrGCount(to);
    }
}

inline void Circuit::updateSum ( Cell* c, int &sum, int &maxsum, int &termainate, int &balance, int step )
{
    sum += c->gain;
    if ( sum > maxsum ) 
    {
        maxsum = sum;
        termainate = step;
        balance = groups[0].getSize() - groups[1].getSize();
        if ( balance < 0 ) balance *= -1;
    }
    else if ( sum == maxsum )
    {
        int b = groups[0].getSize() - groups[1].getSize();
        if ( b < 0 ) b *= -1;
        if ( b < balance )
        {
            maxsum = sum;
            termainate = step;
            balance = b;
        }
    }
    
}

inline void Circuit::bestMoves( Cell **cs, bool* gs, int terminate )
{
    Cell* c;

    groups[0].refresh();
    groups[1].refresh();
    for(int i = 0; i < cells.size(); i++)
    {
        c = cs[i];
        c->gain = 0; // refresh gain
        if ( i <= terminate )
        {
            c->belong = &groups[int( !gs[i] )];
            c->belong->addCell( c );
        }
        else 
        {
            c->belong = &groups[int( gs[i] )];
            c->belong->addCell( c );
        }
    }
    
}

void Cell::incrGain () 
{
    belong->deleteCell( this );
    ++gain;
    belong->addCell( this );
}

void Cell::decrGain () 
{
    belong->deleteCell( this );
    --gain;
    belong->addCell( this );
}

void Cell::incrGain ( int g ) 
{
    belong->deleteCell( this );
    gain += g;
    belong->addCell( this );
}


void Cell::printTerminal ()
{
    printf("Cell %s:", name);
    for(Nets::iterator it = terminals.begin(); it != terminals.end(); ++it)
    {
        printf(" %s", (*it)->name);
    }
    printf("\n");
}

void Net::printTerminal ()
{
    printf("Net %s:", name);
    for(Cells::iterator it = terminals.begin(); it != terminals.end(); ++it)
    {
        printf(" %s", (*it)->name);
    }
    printf("\n");
}

void Group::deleteCell ( Cell* c )
{
    CellListIt it = c->gainPos;
    int gain = c->gain, pos = gain + Pmax;
    GainArray[pos].erase(it);   
    if ( max == gain && GainArray[pos].empty() )
    {
        for(int i = max+Pmax-1; i >= 0; --i)
        {
            if ( !GainArray[i].empty() ) 
            {
                max = i-Pmax;
                break;
            }
        }
        if ( max == gain )
        {
            max = EMPTY_GROUP;
        }

    }
    --gsize;
}

void Group::addCell ( Cell* c )
{
    int gain = c->gain, pos = gain + Pmax;
    if ( max < gain ) max = gain;
    GainArray[pos].push_front(c);
    ++gsize;
    c->gainPos = GainArray[pos].begin();
}

void Group::printGroup ()
{
    for(int i = 0; i-1 < 2*Pmax; i++)
    {
        printf("gain = %d:", i-Pmax);
        for(CellListIt it = GainArray[i].begin(); it != GainArray[i].end(); ++it)
        {
            printf("%s->%d ", (*it)->name, (*it)->gain);
        }
        printf("\n");        
    }
    
}

void Group::writeCells ( FILE* fp )
{
    for(int i = 0; i <= 2*Pmax; ++i)
    {
        for(CellListIt it = GainArray[i].begin(); it != GainArray[i].end(); ++it)
        {
            fprintf( fp, "%s ",  (*it)->name);
        }
    }
    fprintf( fp, ";\n");
}

void Circuit::cleanCircuit ()
{
    for(int i = 0; i < cells.size(); i++)
    {
        free(cells[i]->name);        
        delete cells[i];
    }
    for(int i = 0; i < nets.size(); i++)
    {
        free(nets[i]->name);
        delete nets[i];
    }
    delete [] groups;
}

void Circuit::printCells ()
{
    for(Cells::iterator it = cells.begin(); it != cells.end(); ++it)
    {
        (*it)->printTerminal();
    }
}

void Circuit::printNets ()
{
    for(Nets::iterator it = nets.begin(); it != nets.end(); ++it)
    {
        (*it)->printTerminal();
    }
}

void Circuit::printGroups ()
{
    printf("G0\n");
    groups[0].printGroup();
    printf("G1\n");
    groups[1].printGroup();
}

void Circuit::writePartition ( char* filename )
{
    FILE *fp = fopen(filename, "w");
    Net* n;
    int cutsize = 0;

    for(Nets::iterator itNet = nets.begin(); itNet != nets.end(); ++itNet)
    {
        n = *itNet; 
        if ( n->getGCount(0) > 0 && n->getGCount(1) > 0 ) ++cutsize;
    }
    fprintf( fp, "Cutsize = %d\n", cutsize );
    fprintf( fp, "G1 %d\n", groups[0].getSize());
    groups[0].writeCells( fp );
    fprintf( fp, "G2 %d\n", groups[1].getSize());
    groups[1].writeCells( fp );
    fclose(fp);
}