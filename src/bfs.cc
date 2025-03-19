#include "bfs.hh"

namespace simple {
BFS::BFS(Graph *G, int starting_node) {

    // XXX: can overflow, maybe warn the user?
    int *visited = (int *) calloc (G->getV(), sizeof(int));
    int *queue = (int *) malloc (G->getV() * sizeof(int));

    int front = 0, rear = 0;
    visited[starting_node] = 1;
    queue[rear++] = starting_node;

    while (front < rear) {
        int current_vertex = queue[front++];

        if (G->doDisplay())
            std::cout << current_vertex << " ";

        int row_start = G->row_pointer[current_vertex];
        int row_end = G->row_pointer[current_vertex + 1];

        for (int i = row_start; i < row_end ; i++) {
            int neighbor = G->column_index[i];
            if (!visited[neighbor]) {
                visited[neighbor] = 1;
                queue[rear++] = neighbor;
            }
        }
    }

    free(visited);
    free(queue);

    std::cout << std::endl;
}
}
