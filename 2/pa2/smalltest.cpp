#include <cstdio>
#include <iostream>
#include <map>

using namespace std;

class tmp{
    public:
        tmp() {num = 0;}
        ~tmp() {};

        int getnum() {return num;}
        void setnum(int i) {num = i;}
    private:
        int num;
};

int main()
{
    map<int, tmp*> m;
    tmp t[10];
    for (size_t i = 0; i < 10; i++)
    {
        t[i].setnum(i);
        m.insert( make_pair(i, &t[i]) );
        t[i].setnum(i+20);
    }
    for (std::map<int,tmp*>::iterator it=m.begin(); it!=m.end(); ++it)
    {
        printf("%d ", it->second->getnum());
    }
    printf("\n");
    
}