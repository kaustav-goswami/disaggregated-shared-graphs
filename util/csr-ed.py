import numpy as np
from scipy.sparse import csr_matrix


def edge_list_to_csr(edge_list, num_nodes):
    # Initialize the CSR components
    data = []
    indices = []
    indptr = [0] * (num_nodes + 1)

    # Count the number of edges for each node to build indptr
    for edge in edge_list:
        indptr[edge[0] + 1] += 1

    # Convert indptr to cumulative sum
    for i in range(1, len(indptr)):
        indptr[i] += indptr[i - 1]

    # Initialize a temporary array to keep track of the current position in indices
    current_pos = indptr[:-1].copy()

    # Fill data and indices arrays
    for edge in edge_list:
        src, dest = edge
        pos = current_pos[src]
        indices.insert(pos, dest)
        data.insert(pos, 1)  # Assuming unweighted graph
        current_pos[src] += 1

    # Convert lists to numpy arrays
    data = np.array(data, dtype=np.int32)
    indices = np.array(indices, dtype=np.int32)
    indptr = np.array(indptr, dtype=np.int32)

    return data, indices, indptr

def edge_list_to_csrx(edge_list, num_nodes):
    # Preallocate arrays for row indices, column indices, and data
    row_indices = np.zeros(len(edge_list), dtype=np.int64)
    col_indices = np.zeros(len(edge_list), dtype=np.int64)
    print(num_nodes, len(row_indices), len(col_indices))
    data = np.ones(len(edge_list), dtype=np.int32)  # Assuming unweighted graph

    for idx, edge in enumerate(edge_list):
        row_indices[idx] = edge[0]
        col_indices[idx] = edge[1]

    # Create CSR matrix
    csr = csr_matrix((data, (row_indices, col_indices)), shape=(num_nodes, num_nodes))

    return csr

# Example usage
f = open("socfb-A-anon.mtx", "r")
o = open("fb.csr", "w")

edge_list = []
num_nodes = 0
count = 0
maxi = 23667396
rows_w_more = 0
progress = 0
total_edges = 0
for lines in f.read().split("\n"):
    if (count == maxi % 2366739):
        print(progress)
        progress = progress + 1
    if len(lines) > 0:
        total_edges += 1
        words = lines.split(" ")
        if len(words) == 2:
            edge_list.append((int(words[0]), int(words[1])))
            if num_nodes < int(words[0]):
                num_nodes = int(words[0])
            if num_nodes < int(words[1]):
                num_nodes = int(words[1])
        else:
            rows_w_more += 1
    count += 1

print("done reading", rows_w_more)

csr = edge_list_to_csrx(edge_list, num_nodes + 1)

print("CSR matrix:")
print(csr)
print("Data:", csr.data)
print("Indices:", csr.indices)
print("Indptr:", csr.indptr)

o.write(str(num_nodes) + "\n")
o.write(str(total_edges) + "\n")

for i in range(len(csr.indptr)):
    o.write(str(csr.indptr[i]) + " ")

o.write("\n")

for i in range(len(csr.indices)):
    o.write(str(csr.indices[i]) + " ")
    
f.close()
o.close()