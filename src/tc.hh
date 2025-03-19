#ifndef TC_HH_
#define TC_HH_

#include <iostream>
#include "graph.hh"

namespace simple {
class TC  {
    private:
        int triangle_count;
    protected:
        int min(int a, int b);
        int intersectCount(Graph *G, int start, int end, int len1, int len2);
    public:
        TC(Graph *G);
};
}
#endif