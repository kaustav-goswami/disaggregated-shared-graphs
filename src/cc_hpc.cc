#include "cc_hpc.hh"

namespace simple {

CC_HPC::CC_HPC() {
}
CC_HPC::CC_HPC(Graph *G) {
    // use the private variable to call component
    //connectedComponents_CSR(int num_nodes, const std::vector<int>& row_ptr, const std::vector<int>& col_ind, std::vector<int>& component) {
    this->component.resize(G->getV());
    std::vector<int> new_component(G->getV());
    bool changed;

    // Initialize each node to its own component
    #pragma omp parallel for
    for (int i = 0; i < G->getV(); ++i) {
        this->component[i] = i;
    }

    do {
        changed = false;

        #pragma omp parallel for
        for (int u = 0; u < G->getV(); ++u) {
            int min_comp = this->component[u];
            for (int i = G->row_pointer[u]; i < G->row_pointer[u + 1]; ++i) {
                int v = G->column_index[i];
                if (this->component[v] < min_comp) {
                    min_comp = this->component[v];
                }
            }
            new_component[u] = min_comp;
        }

        #pragma omp parallel for reduction(|:changed)
        for (int i = 0; i < G->getV(); ++i) {
            if (this->component[i] != new_component[i]) {
                changed = true;
                this->component[i] = new_component[i];
            }
        }

    } while (changed);
}

void CC_HPC::printCC(Graph *G) {
    for (int i = 0 ; i < G->getV() ; i++) {
        std::cout << "Node " << i << " is in component " << this->component[i] << std::endl;
    }
}
}
