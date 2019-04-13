#ifndef UTILS_HEADER_
#define UTILS_HEADER_
#include "Circuit.h"

void parse ( Circuit&, const char* );

#define MAX_NAME 512

struct cmp_str
{
    bool operator()(char const *a, char const *b)
    {
        return std::strcmp(a, b) < 0;
    }
};

void parse ( Circuit& cir, const char* filename )
{
    FILE *fp = fopen(filename, "r");
    Cells* cells = cir.getCells();
    Nets*  nets = cir.getNets();
    std::map<const char*, Cell*, cmp_str> cellmap;
    std::map<const char*, Cell*, cmp_str> netcellmap;
    double r;

    fscanf( fp, "%lf", &r );
    cir.setRatio( r );
    char token[MAX_NAME] = "";
    while( fscanf( fp, "%s", token ) > 0 )
    {
        if ( token[0] == '\0' || token[0] == '\n' || token[0] == ';' ) continue;
        if ( strcmp( token, "NET" ) == 0 )
        {
            fscanf( fp, "%s", token );
            Net* n = new Net;
            n->name = (char *)malloc( sizeof(char) * (strlen(token)+1));
            strcpy( n->name, token );
            nets->push_back(n);

            netcellmap.clear();
        }
        else
        {
            Cell* c;
            std::map<const char*, Cell*, cmp_str>::iterator it = cellmap.find(token);
            std::map<const char*, Cell*, cmp_str>::iterator netit = netcellmap.find(token);            
            if ( it == cellmap.end() )
            {
                c = new Cell();
                c->name = (char *)malloc( sizeof(char) * (strlen(token)+1));
                strcpy( c->name, token );
                //c->addTerminal(nets->back());
                //nets->back()->addTerminal(c);
                cells->push_back(c);
                cellmap.insert( std::pair<const char*, Cell*>(c->name, c) );
            }
            else
            {
                c = it->second;
                //c->addTerminal(nets->back());
                //nets->back()->addTerminal(c);
            }
            if ( netit == netcellmap.end() )
            {
                c->addTerminal(nets->back());
                nets->back()->addTerminal(c);
                netcellmap.insert( std::pair<const char*, Cell*>(c->name, c) );
            }
        }
    }
    fclose(fp);
}

#endif