#include "tc.hh"

namespace simple {
TC::TC(Graph *G) {
    triangle_count = 0;
    for (int u = 0; u < G->getV(); u++) {
        int row_start_u = G->row_pointer[u];
        int row_end_u = G->row_pointer[u + 1];

        for (int idx = row_start_u; idx < row_end_u; idx++) {
            int v = G->column_index[idx];

            if (u < v) {
                int row_start_v = G->row_pointer[v];
                int row_end_v = G->row_pointer[v + 1];

                triangle_count += intersectCount(G, row_start_u, row_start_v,
                            row_end_u - row_start_u, row_end_v - row_start_v);
            }
        }
    }
    if (G->doDisplay()) {
        std::cout << "  Triange Count = " << triangle_count;
        std::cout << std::endl;
    }
}

int TC::min(int a, int b) {
    return (a < b) ? a : b;
}

int TC::intersectCount(
        Graph *G, int start1, int start2, int len1, int len2) {
    int i = 0, j = 0, count = 0;
    while (i < len1 && j < len2) {
        if (G->column_index[start1 + i] < G->column_index[start2 + j]) {
            i++;
        }
        else if (G->column_index[start1 + i] > G->column_index[start2 + j]) {
            j++;
        }
        else {
            count++;
            i++;
            j++;
        }
    }
    return count;
} 
}