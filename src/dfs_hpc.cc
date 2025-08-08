#include "dfs_hpc.hh"

namespace simple {
DFS_HPC::DFS_HPC() {
    // someone wantes to use DFS as an algorithm.
}

DFS_HPC::DFS_HPC(Graph *G) {
    std::vector<bool> visited(G->getV(), false);

    #pragma omp parallel for schedule(dynamic)
    for (int start = 0; start < G->getV(); ++start) {
        if (!visited[start]) {
            std::stack<int> stack;
            stack.push(start);

            while (!stack.empty()) {
                int node = stack.top();
                stack.pop();

                bool already_visited;
                #pragma omp critical
                {
                    already_visited = visited[node];
                    if (!already_visited) visited[node] = true;
                }

                if (already_visited) continue;
  
                // std::cout << "Visited node: " << node << std::endl;

                for (int i = G->row_pointer[node];
                                            i < G->row_pointer[node + 1]; ++i) {
                    int neighbor = G->column_index[i];
                    #pragma omp critical
                    {
                        if (!visited[neighbor]) stack.push(neighbor);
                    }
                }
            }
        }
    }
}
}
