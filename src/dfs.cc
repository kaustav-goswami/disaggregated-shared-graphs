#include "dfs.hh"

namespace simple {
DFS::DFS() {
    // someone wants to use DFS as an algorithm.
}
DFS::DFS(Graph *G, int starting_vertex) {
    // Need to do something? 
    // The algo user shoud not be doing addressing.
    // volatile char* starting_addr = G->getStartingAddr();
    // XXX: can overflow
    int *visited = (int *)calloc(G->getV(), sizeof(int));
    
    // visited for the starting node must be filled up no?
    visited[starting_vertex] = 1;
    for (int i = 0 ; i < G->getV() ; i++) {
        if (!visited[i])
            dfs(G, i, visited); // , pointer_to_row_ptr, pointer_to_col_idx);
    }
    std::cout << std::endl;
}

void DFS::dfs(Graph *G, int vertex, int *visited) {
    visited[vertex] = 1;
    if (G->doDisplay())
        std::cout << vertex << " ";

    for (int i = G->row_pointer[vertex];
                i < G->row_pointer[vertex + 1]; i++) {
        int neighbor = G->column_index[i];
        if (!visited[neighbor])
            dfs(G, neighbor, visited);
    }
}

void DFS::dfsAPI(Graph *G, int vertex, int *visited, int component_id,
                                                            int* component) {
    visited[vertex] = 1;
    component[vertex] = component_id;

    int row_start = G->row_pointer[vertex];
    int row_end = G->row_pointer[vertex + 1];

    for (int i = row_start; i < row_end ; i++) {
        int neighbor = G->column_index[i];
        if (!visited[neighbor])
            dfsAPI(G, neighbor, visited, component_id, component);
    }

}
}
