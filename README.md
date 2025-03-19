# A Simple Graph Processing Software for Shared Disaggregated Memory

A single threaded graph processing framework where a graph is allocated on a
shared memory and workers (either processes or hosts) can work on the same
graph together.

Use-case: Shared disaggregated memories.

This is a simple framework for graph processing.
I have implemented this to quickly test multihost graph processing.

Depends on:
* cpp-arg-parse (`github.com/kaustav-goswami/cpp-arg-parse`)
* cmake

```sh
make -j4
# The master node is called via
./simple_graph \
    --algorithm allocator \
    --graph tests/graph.csr \
    --total-hosts 3 \
    --host-id 0
# The worker nodes are called via
./simple_graph --algorithm bfs \
    --graph tests/graph.csr \
    --total-hosts 3 \
    --host-id 1
./simple_graph --algorithm dfs \
    --graph tests/graph.csr \
    --total-hosts 3 \
    --host-id 2
```

The graph is read from a file in csr format and is stored in class Graph as a
CSR graph in memory.

`class Graph` is a host-side object, that has a pointer to the start of the
remote memory.
It assumes that the remote memory is a memory-mapped range on the
disaggregated/remote device.
Only the allocator initializes the graph, which also needs to be the master
node.
Once the setup is complete, a synchronization variable is set in the shared
memory range (needs to be uncacheable; must be hardware managed).
The workers then begin working on the algorithm specified.

Current implementation is single-threaded.

## Graph Format

The input graphs can be **directed** and **weighted**.
The program expects that the graph will be in csr format with:
```
<Vertex Count>
<Edge Count>
<Values of the Row Pointer Array separated by " ">
<Values of the Column Index Array separated by " ">
<(Optional) Weights Array separated by " ">
```

Sample graphs are stored in `tests/` directory.

The graph is stored pretty much the same way in the memory with the addition
of some more metadata.
All entries are assumed to be `int`.
Following the a flat representation of the mmaped region.
```
_____________________________________________________________________ .. ______
| V | E | size | size | size | sync | row ptr | col idx | weights         | | |
|___|___|______|______|______|______|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|__ .. _|_|_|
<---------- METADATA (6) ---------->|         |         |
                                    | int *column_index |
                                    |                   |
                            int* row_pointer        int *weights
```

The start of the graph is stored as _graph in class Graph.
Each entry is separated by sizeof(int).

## Building

```sh
git clone git@github.com:kaustav-goswami/simple-graph.git
cd simple-graph
git submodule init
git submodule update
mkdir build
cd build
cmake ..
make -j4
```

## Testing

### Command-line arguments

In this version, we take in the following arguments:
```txt
This program understands the following arguments:
    -a --algorithm  algorithm to run bc, bfs, cc, dfs, pr, sssp, tc, allocator
    -g --graph      path to a graph 
    -t --total-hosts        total number of hosts 
    -i --host-id    ID of the current host 
    -h --help       display this message 
    -v --verbose    enable verbose 
    -z --test-mode  enable testing mode (uses shmem interface) [false], true
    -r --randomize  randomize the starting node [false], true
    -d --display    display output in stdout false, [true]

```

### Quick Testing

To test on a single system, you need to enable huge pages.
This can be done by:

See this: https://github.com/comsec-group/blacksmith/issues/2#issuecomment-971810211

This would require the user to be root.
The host now becomes the master, allocates a graph and performs some graph
processing algorithm.

```sh
# make sure to enable testing mode and enable verbose
./simple_graph \
        --graph ../tests/graph4.csr \
        --host-id 0 \
        --total-hosts 3 \
        --test-mode true \
        --algorithm allocator \         # can also be allocator and worker
        -v
```
A different process can be started with a different algorithm working on the
same graph.
```sh
# make sure to enable testing mode and enable verbose
./simple_graph \
        --graph ../tests/graph4.csr \
        --host-id 1 \                   # worker node 1
        --total-hosts 3 \
        --test-mode true \
        --algorithm sssp \
        -v  &
        ```sh
# make sure to enable testing mode and enable verbose
./simple_graph \
        --graph ../tests/graph4.csr \
        --host-id 2 \                   # worker node 2
        --total-hosts 3 \
        --test-mode true \
        --algorithm dfs \
        -v
```

Note: Use my machine if you have root access. Huge pages are already allocated.

### Gem5 Testing

TODO.

## Roadmap

- [*] Test with large(r) graphs
- [ ] Update the algorithms to be HPC-compatible.
- [ ] Add `gem5` annotations to enable gem5 testing faster.
- [ ] Make `MAP_SHARED` work on a single host.
- [ ] `allocator` and `host_id` arguments are conflicting.

### Low Priority

- [ ] Figure out ways to parallelize the algorithm implementation.
- [ ] More graph algorithms

## Supported Graph Algorithms

* BFS
* DFS
* BC
* SSSP
* TC
* CC
* PR

Cite: The graph algorithm implementation code is mostly AI generated.
