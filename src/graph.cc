#include "graph.hh"


namespace simple {
// This version is strictly for space-control. It is also depended on the ext
// middleware layer.

/*
Graph::Graph(std::string path, int subgraph_id, int host_id,
        bool test = false,
        bool randomize = false,
        bool display = true,
        bool verbose = false) : _test(test), 
                                _randomize(randomize),
                                _display(display),
                                _verbose(verbose) {
    // this version needs secure_alloc() to function correctly. However, this
    // needs to sit behind the middleware as for the user, this it doesn't
    // matter where his graph is located at.
    
    // graph info must be private. Let's ask for both read and write
    // permissions. Why?
    bool permissions = 0b1;
    this->_graph_info = graph_middleware(path.c_str(),
                                        subgraph_id,
                                        host_id,
                                        permissions,
                                        test,
                                        verbose);
    // The pointer to the permission table is never given to the user-level
    // process. Instead, the user-level process only sees the
    // data_start_pointer as the starting address, if it is given permission to
    // access the entire memory region.
    //
    // In this code, both the pointers are behind the middleware. The
    // user-level process, that is, this* only gets access to the number of
    // nodes, edges, (invalid weights), array to row_ptr, array to col_ptr,
    // invalid weights. Since this is a worker node, it does need another
    // pointer to the synch variable.

    if (host_id == 0) {
        // I am the allocator
    }
    else {
        // I am a worker node.
        this->_metadata = &graph_info->data_ptr[0];
        // Only the sync variable is accessible by this node. flexible
        // permissions.

        // variable to true and let the workers start working.
        do {
            // This is a memory pooling statement, which needs the value to be
            // refreshed! We can implement this as a mwait instead of a pool
            _local_sync_copy = this->_metadata[SYNC];

            // The worker will wait.
            if (_local_sync_copy == READY)
                break;
        } while (_local_sync_copy != 1);

        // The start of the graph is stored in *_graph; Set the rest of the
        // metadata variables. The master sets these variables up and sets its
        // own private variables. But the workers needs to wait until master
        // sets these up. The setter methods right now sets both these
        // values up, so this part has to be done manually;
        _V = this->_graph_info->num_nodes;
        _E = this->_graph_info->num_edges;

        _size_row_pointer = this->_metadata[ROWP_SIZE];
        _size_col_idx = this->_metadata[COLI_SIZE];
        _size_weights = this->_metadata[WEIGHT_SIZE];

        if (_size_weights == 0)
            _has_weight = false;
        else {
            _has_weight = true;
            // This version does not support weighted graphs!
            std::cout << "fatal! This version foes not support weights!" <<
                std::endl;
            exit(-1);
        }

        // finally allocate the pointer arrays
        row_pointer = &_graph[METADATA];
        column_index = &_graph[METADATA + getRowPointerSize()];
        if (_has_weight == true)
            weights = &_graph[METADATA + getRowPointerSize() +
                                                            getColIndexSize()];
        
        // The worker is ready to work.
    }
    // Be very careful when to use the synchronization variable. It is very
    // expensive! The allocation is complete and the workers are ready to
    // start working on the graph!
    if (_verbose)
        printGraph();
        this->_metadata[SYNC] = &graph_info->sync_variable;
        // loop until the data is ready!
        while (this->_metadata != READY) {}

        this->_metadata[NODE]
}
        */


#ifdef USE_MIDDLEWARE
// for secure memory, we need a secureDefineMemory. The use only knows their
// host id and process id.
void Graph::secureDefineMemory(int host_id) {
    // to prevent the user requesting arbitary size, size is simply ignored.
    // the PID is dynamically calculated to make sure that the process cannot
    // fake it.
    // TODO: It is the job of the allocator program to define the groups
    assert(false && "NotImplementedError\n");
    
    // getpid.

    // If size is less than 1 GiB, then just allocate the whole 1 GiB block.
    size_t real_size = 0x200000000;
    
    // Allocate the graph using dalloc. Ignore the class method. Make sure that
    // the user is running it in gem5. If not, the test mode must be true.
    if (_test) {
        // if verbose is enabled, then print the message notifying the user.
        // huge page is currently disabled. we'll add that feature back with
        // madvise so that the user doesn't need root anymore.
        if (_verbose)
            std::cout << "info: shmem will be used!" << std::endl;
        this->_graph = shmalloc(real_size, host_id);
    }
    else {
        // if verbose is enabled, then print the message notifying the user
        if (_verbose)
            std::cout << "info: /dev/dax0.0 will be used!" << std::endl;
        this->_graph = dmalloc(real_size, host_id);
    }
    // The pointer to the graph is set!
}
#else
// There needs to be a difference between initialization and allocation and
// definition. We need a pointer and need to define the variables. Allocation
// only needs to happen once.
void Graph::defineMemory(size_t size, int host_id) {
    // :size: size of the graph in bytes. In most cases, this is the size of
    //      file storing the graph.
    // :host_id: self explanatory!
    // returns a pointer to the mmapped region
    //
    // If size is less than 1 GiB, then just allocate the whole 1 GiB block.
    size_t real_size = size;
    if (size < 0x40000000) {
        std::cout << "warn: small graph! allocating 1 GiB of shared memory"
                                                                << std::endl;
        real_size = 0x40000000;
    }
    
    // Allocate the graph using dalloc. Ignore the class method. Make sure that
    // the user is running it in gem5. If not, the test mode must be true.
    if (_test) {
        // if verbose is enabled, then print the message notifying the user.
        // huge page is currently disabled. we'll add that feature back with
        // madvise so that the user doesn't need root anymore.
        if (_verbose)
            std::cout << "info: shmem will be used!" << std::endl;
        this->_graph = shmalloc(real_size, host_id);
    }
    else {
        // if verbose is enabled, then print the message notifying the user
        if (_verbose)
            std::cout << "info: /dev/dax0.0 will be used!" << std::endl;
        this->_graph = dmalloc(real_size, host_id);
    }
    // The pointer to the graph is set!
}
#endif

void Graph::initializeMemory() {
    // Now that the pointer to the memory is DEFINED, either allocate or
    // initialize memory depending on the host id and if loading is allowed
    // agamin.

    // The start of the graph is stored in *_graph; Set the rest of the
    // metadata variables. The master sets these variables up and sets its
    // own private variables. But the workers needs to wait until master
    // sets these up. The setter methods right now sets both these
    // values up, so this part has to be done manually;
    //
    // wait if the metadata is not ready
    while (this->_metadata[SYNC] != READY) {}

    _V = this->_metadata[VERTEX];
    _E = this->_metadata[EDGE];

    _size_row_pointer = this->_metadata[ROWP_SIZE];
    _size_col_idx = this->_metadata[COLI_SIZE];
    _size_weights = this->_metadata[WEIGHT_SIZE];

    if (_size_weights == 0)
        _has_weight = false;
    else
        _has_weight = true;

    // finally allocate the pointer arrays
    row_pointer = &_graph[METADATA];
    column_index = &_graph[METADATA + getRowPointerSize()];
    if (_has_weight == true)
        weights = &_graph[METADATA + getRowPointerSize() +
                                                        getColIndexSize()];
    
    // The worker is ready to work.

}

#ifdef USE_MIDDLEWARE
void Graph::initializeSecureMemory() {
    // Now that the pointer to the memory is DEFINED, either allocate or
    // initialize memory depending on the host id and if loading is allowed
    // agamin.

    // The start of the graph is stored in *_graph; Set the rest of the
    // metadata variables. The master sets these variables up and sets its
    // own private variables. But the workers needs to wait until master
    // sets these up. The setter methods right now sets both these
    // values up, so this part has to be done manually;
    //
    // wait if the metadata is not ready
    while (this->_metadata[SYNC] != READY) {}

    _V = this->_metadata[VERTEX];
    _E = this->_metadata[EDGE];

    _size_row_pointer = this->_metadata[ROWP_SIZE];
    _size_col_idx = this->_metadata[COLI_SIZE];
    _size_weights = this->_metadata[WEIGHT_SIZE];

    if (_size_weights == 0)
        _has_weight = false;
    else
        _has_weight = true;

    // Make sure that access groups are defined before the actual data
    access_group_pointer = &_graph[METADATA];

    // finally allocate the pointer arrays
    row_pointer = &_graph[METADATA + getPermissionPointerSize()];
    column_index = &_graph[METADATA
                        + getPermissionPointerSize()
                        + getRowPointerSize()];

    if (_has_weight == true)
        weights = &_graph[METADATA
                        + getPermissionPointerSize() 
                        + getRowPointerSize() 
                        + getColIndexSize()];
    
    // The worker is ready to work.

}
#endif

Graph::Graph(std::string path, size_t size, int host_id, bool test = false,
            bool randomize = false, bool display = true, bool verbose = false,
            bool load = false) : _test(test),
                                _randomize(randomize),
                                _display(display),
                                _verbose(verbose),
                                _load(load) {
    // This is the new version with subgraphs.
    // TODO: Add an assert to make sure that the graph is twitter graph.
    // Allocate the pointer to the shm so that the converted graph can be
    // allocated!
    defineMemory(size, host_id);
    this->_metadata = &_graph[0];
    if (_load) {
        const auto start{std::chrono::steady_clock::now()};
        convertELtoCSR(path, host_id);
        const auto finish{std::chrono::steady_clock::now()};
        const std::chrono::duration<double> elapsed_seconds{finish - start};
        
        std::cout << "Loading " << path << " took ";
        std::cout << elapsed_seconds.count();
        std::cout <<
                " seconds" << std::endl;
    }
    else
        // the user just wants to the data to be pointed correctly
        initializeMemory();
    if (_verbose)
        printGraph();

    // Resume loading the variables.
}

Graph::Graph(std::string path, size_t size, int host_id, bool test = false,
            bool randomize = false, bool display = true, bool verbose = false)
            : _test(test),
              _randomize(randomize),
              _display(display),
              _verbose(verbose) {

    // Make sure that the graph pointers are defined
    defineMemory(size, host_id);

    // Assign the metadata array so that we dont have to deal with offsets in
    // this implementation. Doesn't matter if I am the master or a worker.
    this->_metadata = &_graph[0];

    // Allocation complete! If I am the master, set the synchronization
    if (host_id == 0) {
        // Wait, first write the graph!
        this->_metadata[SYNC] = ALLOCATING;
        graphWriter(path);
        this->_metadata[SYNC] = READY;
        // Honestly, the master can now end the process!
    }
    else {
        // variable to true and let the workers start working.
        do {
            // This is a memory pooling statement, which needs the value to be
            // refreshed! We can implement this as a mwait instead of a pool
            _local_sync_copy = this->_metadata[SYNC];

            // The worker will wait.
            if (_local_sync_copy == READY)
                break;
        } while (_local_sync_copy != 1);

        initializeMemory();
    }
    // Be very careful when to use the synchronization variable. It is very
    // expensive! The allocation is complete and the workers are ready to
    // start working on the graph!
    if (_verbose)
        printGraph();
}

#ifdef USE_MIDDLEWARE
// The final constructor to assign group ids to participant hosts
Graph::Graph(std::string path,
            size_t size,
            int host_id, 
            int group_id,
            bool test = false,
            bool randomize = false, 
            bool display = true, 
            bool verbose = false,
            bool load = false) : _my_access_group(group_id),
                                _test(test),
                                _randomize(randomize),
                                _display(display),
                                _verbose(verbose),
                                _load(load) {
    // Only implemented for lookups!
    assert(false && "NotImplementedError\n");
    // define some graph partiion number. For the fb graph, keep it as 10.
    int group_elements;
    if (path == "twitter.scsr")
        group_elements = 10000;
    else
        group_elements = 10;

    // first we need to request permission to access the header to the middle-
    // ware
    this->_secure_graph = graph_middleware(host_id, group_id, group_elements);

    // if I am the allocator then I don't care.
    if (host_id == 0) {

        this->_graph = this->_secure_graph->_data_ptr;
        // Assign the metadata array so that we dont have to deal with offsets in
        // this implementation. Doesn't matter if I am the master or a worker.
        this->_metadata = &_graph[0];

        // Wait, first write the graph!
        this->_metadata[SYNC] = ALLOCATING;
        secureGraphWriter(path);
        this->_metadata[SYNC] = READY;
        // Be very careful when to use the synchronization variable. It is very
        // expensive! The allocation is complete and the workers are ready to
        // start working on the graph!
        if (_verbose)
            printGraph();
    }
    else {
        // Next, depending upon our group id, we need the middle_graph_t to
        // initialize the row_ptr and the col_idx.
        this->_graph = this->_secure_graph->_data_ptr;

        // The metadata is already allocated and read by the middleware
        // variable to true and let the workers start working.
        do {
            // This is a memory pooling statement, which needs the value to be
            // refreshed! We can implement this as a mwait instead of a pool
            _local_sync_copy = this->_metadata[SYNC];

            // The worker will wait.
            if (_local_sync_copy == READY)
                break;
        } while (_local_sync_copy != 1);
        this->_metadata = &_graph[0];

        _V = this->_metadata[VERTEX];
        _E = this->_metadata[EDGE];

        _size_row_pointer = this->_metadata[ROWP_SIZE];
        _size_col_idx = this->_metadata[COLI_SIZE];
        _size_weights = this->_metadata[WEIGHT_SIZE];

        _total_access_groups = this->_metadata[GROUPS];

        if (_size_weights == 0)
            _has_weight = false;
        else
            _has_weight = true;

        // finally allocate the pointer arrays
        access_group_pointer = &_graph[METADATA];
        row_pointer = &_graph[METADATA + getPermissionPointerSize()];
        column_index = &_graph[METADATA + getPermissionPointerSize() +
                                                        getRowPointerSize()];
        if (_has_weight == true)
            weights = &_graph[METADATA + getPermissionPointerSize() +
                                    getRowPointerSize() + getColIndexSize()];
        
        // The worker is ready to work.
        
        // only a part of the actual row_ptr will be allocated. Depending upon the
        // row pointers, the correct col idx will be assigned.

        // this is completly automated. Yes, this will create 2x the number of
        // entries, but the other option is to allow the entire col idx to be read
        // by everyone else. poeple will not know from where to where an edge is
        // present because they DO NOT KNOW the row_ptr completely.

        // We also need to implement an interrupt in the software to make sure that
        // information is not leaked.

        // The hardware will enforce the permission. Ideally, from the software
        // side, I should not be concerned if I set the permissions incorrectly.
    }
}
#endif

// A utility method to convert an EL file to CSR (twitter)
void Graph::convertELtoCSR(std::string path, int host_id) {
    // TODO: Marked for deletion
    // @params
    // :path: path to the *twitter.el graph

    // If I call load, then I must be host 0.
    assert(_load && host_id == 0);

    // start reading the el and writing into the assigned pointers?

    // Wait, first write the graph!
    this->_metadata[SYNC] = ALLOCATING;
    std::ifstream infile(path);            
    // local memory!
    std::vector<std::pair<int, int>> edges;
    int max_node = 0;                                 
                                                      
    // Read edge list                                 
    std::string line;                                 
    while (std::getline(infile, line)) {              
        std::istringstream iss(line);                 
        int u, v;                                     
        if (!(iss >> u >> v)) continue;               
        edges.emplace_back(u, v);                     
        max_node = std::max({max_node, u, v});        
    }                                 
    
    if (_verbose)
        std::cout << "info: file reading done!" << std::endl;

    int num_vertices = max_node + 1;
    setV(num_vertices);
    int num_edges = edges.size();
    setE(num_edges);
    this->row_pointer = &this->_graph[METADATA];
    setRowPointerSize(num_vertices + 1);
    this->column_index = &this->_graph[METADATA + getRowPointerSize()];
    setColIndexSize(num_edges);
                                                      
    // Count outgoing edges per vertex (parallel)     
    #pragma omp parallel                              
    {                                                 
        std::vector<int> local_count(num_vertices, 0);
                                                      
        #pragma omp for nowait                        
        for (int i = 0; i < num_edges; ++i) {         
            int u = edges[i].first;                   
            local_count[u]++;                         
        }                                             
                                                      
        #pragma omp critical                          
        {                                             
            for (int i = 0; i < num_vertices; ++i) {  
                row_pointer[i + 1] += local_count[i];     
            }                                         
        }                                             
    }                                                 
                                                      
    // Prefix sum to get row_ptr                      
    for (int i = 0; i < num_vertices; ++i) {
        this->row_pointer[i + 1] += this->row_pointer[i];
    }

    // populate a local copy of the row ptr
    if (_verbose)
        std::cout << "info: copying row ptr" << std::endl;
    int *temp_row_ptr = (int *) malloc (getRowPointerSize() * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < getRowPointerSize() ; i++)
        temp_row_ptr[i] = this->row_pointer[i]; 
                                                      
    // Fill col_idx (parallel)                        
    #pragma omp parallel for                          
    for (int i = 0; i < num_edges; ++i) {             
        int u = edges[i].first;                       
        int v = edges[i].second;                      
                                                      
        int pos;                                      
        #pragma omp atomic capture                    
        pos = temp_row_ptr[u]++;                      
                                                      
        column_index[pos] = v;
    }                                                 
    free(temp_row_ptr);
    this->_has_weight = false;
    setWeightsSize(0);
    this->_metadata[SYNC] = READY;

    // Honestly, the master can now end the process!
}

int Graph::getOffset(int index) {
    // In the stable version with integers, we deprecate this function!
    assert(false && "deprecated: using offsets for the mmap is removed!");
    // Returns the offset which when added to the start address gives the value
    // stored at that offset.
    return index * sizeof(uint64_t);
}


#ifdef USE_MIDDLEWARE
void Graph::secureGraphWriter(std::string path) {
    // Uses fstream from C++. This is the specilized version where each vertex
    // has a pointer stored in the shared memory and permissions can be then
    // assigned using any logic.
    std::fstream file;
    char *filename = new char[path.length()];
    std::strcpy(filename, path.c_str());
    file.open(filename, std::ios::in);
    if (!file) {
        printf("The specified graph file %s does not exist!\n", filename);
        exit(EXIT_FAILURE);
    }
    // The start of the mmap is stored in _graph.
    // We assume a simple storage for the graph i.e. also stored in CSR format.
    if (file.is_open()) {
        std::string lines;
        int line_count = 0;
        while (getline(file, lines)) {
            // printf("line : *%s*\n", lines.c_str());
            // The format is very specific.
            // Line 1: V
            // Line 2: E
            // Line 3: Compressed vertex pointer array size
            // Line 4: Compressed vertex pointer array
            // Line 5: row_pointer_array
            // Line 6: column_index_array
            // Line 7: weights (optional)

            char *cstr = new char[lines.length()];
            std::strcpy(cstr, lines.c_str());

            // For each line, do something!
            if (line_count == 0) {
                setV(std::stoi(cstr));
            }
            else if (line_count == 1) {
                setE(std::stoi(cstr));
            }
            else if (line_count == 2) {
                // here is the size of the vertex pointer
                setPermissionPointerSize(std::stoi(cstr));
            }
            else if (line_count == 3) {
                this->access_group_pointer = &_graph[METADATA];
                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                int size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    this->access_group_pointer[size++] = std::stoi(words);
                    words = strtok(nullptr, " ");
                }
            }
            else if (line_count == 4) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata.
                this->row_pointer = &_graph[METADATA 
                                        + getPermissionPointerSize()];

                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                int size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    row_pointer[size++] = std::stoi(words);
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                // The next array is ready to be initialized.
                setRowPointerSize(size);
            }
            else if (line_count == 5) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata + end of the row pointer array.
                column_index = &_graph[METADATA 
                                + getPermissionPointerSize()
                                + getRowPointerSize()];

                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                int size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    column_index[size++] = std::stoi(words);
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                // The next array is ready to be initialized.
                setColIndexSize(size);
            }
            // If the graph has weights, then there will be another line with
            // valid weights.
            else if (line_count == 6) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata + end of the row pointer array.
                weights = &_graph[METADATA
                                + getPermissionPointerSize()
                                + getRowPointerSize()
                                + getColIndexSize()];

                // See if this is an extra line first.
                if (lines.length() == 0 || lines.length() == 1) {
                    _has_weight = false;
                    setWeightsSize(0);
                }
                else {
                    _has_weight = true;
                    // First break up all the numbers
                    char *words = strtok(cstr, " ");
                    uint64_t size = 0;
                    
                    while (words != nullptr) {
                        // The setter methods should easily do the trick!
                        weights[size++] = std::stoi(words);
                        words = strtok(nullptr, " ");
                    }
                    // Finally set the offset value correctly.
                    setWeightsSize(size);
                }
            }
            else {
                printf("Invalid csr format! c = %d\n", line_count);
                exit(EXIT_FAILURE);
            }
            line_count++;
        }
        if (line_count == 4) {
            // There are only  four lines in the file. therefore, set the
            // weight count to 0
            _has_weight = false; 
            setWeightsSize(0);
        }
    }
    file.close();
}
#else
void Graph::graphWriter(std::string path) {
    // Uses fstream from C++
    std::fstream file;
    char *filename = new char[path.length()];
    std::strcpy(filename, path.c_str());
    file.open(filename, std::ios::in);
    if (!file) {
        printf("The specified graph file %s does not exist!\n", filename);
        exit(EXIT_FAILURE);
    }
    // The start of the mmap is stored in _graph.
    // We assume a simple storage for the graph i.e. also stored in CSR format.
    if (file.is_open()) {
        std::string lines;
        int line_count = 0;
        while (getline(file, lines)) {
            // printf("line : *%s*\n", lines.c_str());
            // The format is very specific.
            // Line 1: V
            // Line 2: E
            // Line 3: row_pointer_array
            // Line 4: column_index_array
            // Line 5: weights (optional)

            char *cstr = new char[lines.length()];
            std::strcpy(cstr, lines.c_str());

            // For each line, do something!
            if (line_count == 0) {
                setV(std::stoi(cstr));
            }
            else if (line_count == 1) {
                setE(std::stoi(cstr));
            }
            else if (line_count == 2) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata.
                this->row_pointer = &_graph[METADATA];

                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                int size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    row_pointer[size++] = std::stoi(words);
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                // The next array is ready to be initialized.
                setRowPointerSize(size);
            }
            else if (line_count == 3) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata + end of the row pointer array.
                column_index = &_graph[METADATA + getRowPointerSize()];

                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                int size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    column_index[size++] = std::stoi(words);
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                // The next array is ready to be initialized.
                setColIndexSize(size);
            }
            // If the graph has weights, then there will be another line with
            // valid weights.
            else if (line_count == 4) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata + end of the row pointer array.
                weights = &_graph[METADATA + getRowPointerSize() +
                                                            getColIndexSize()];

                // See if this is an extra line first.
                if (lines.length() == 0 || lines.length() == 1) {
                    _has_weight = false;
                    setWeightsSize(0);
                }
                else {
                    _has_weight = true;
                    // First break up all the numbers
                    char *words = strtok(cstr, " ");
                    uint64_t size = 0;
                    
                    while (words != nullptr) {
                        // The setter methods should easily do the trick!
                        weights[size++] = std::stoi(words);
                        words = strtok(nullptr, " ");
                    }
                    // Finally set the offset value correctly.
                    setWeightsSize(size);
                }
            }
            else {
                printf("Invalid csr format! c = %d\n", line_count);
                exit(EXIT_FAILURE);
            }
            line_count++;
        }
        if (line_count == 4) {
            // There are only  four lines in the file. therefore, set the
            // weight count to 0
            _has_weight = false; 
            setWeightsSize(0);
        }
    }
    file.close();
}
#endif

void Graph::printGraph() {
    // This method prints the graph in CSR format to verify whether the
    // allocation is correct. The user has to enable verbose.

    std::cout << "== Printing the graph ==\n" << std::endl;
    std::cout << "V = " << getV() << std::endl;
    std::cout << "E = " << getE() << std::endl;
    std::cout << "Row Ptr Array Size = " << getRowPointerSize() << std::endl;
    std::cout << "Column Idx Array Size = " << getColIndexSize() << std::endl;
    std::cout << "Weights Size = " << getWeightsSize() << std::endl;

    if (_load) {
    std::cout << "N = [ ";
    for (int i = 0 ; i < getRowPointerSize() ; i++)
        std::cout << this->row_pointer[i] << " ";

    std::cout << "]\nF = [ " ;
    for (int i = 0 ; i < getColIndexSize() ; i++)
        std::cout << column_index[i] << " ";
        
    std::cout << "]\nW = [ ";
    
    if (_has_weight) {
        for (int i = 0 ; i < getWeightsSize() ; i++)
        std::cout << weights[i] << " ";
    }
    std::cout << "]" << std::endl;
    }
    std::cout << "== End of the graph ==\n" << std::endl;
}

// Defining all the getter methods here. For most of the code, use the getter
// methods instead of using the private variable directly.
int Graph::getV() {
    return _V;
}

int Graph::getE() {
    return _E;
}
int Graph::getRowPointerSize() {
    return _size_row_pointer;
}
int Graph::getColIndexSize() {
    return _size_col_idx;
}
int Graph::getWeightsSize() {
    return _size_weights;
}

// need a method to get a random node that exits in the graph!
int Graph::getStartingNode() {
    int return_value;
    if (this->_randomize) {
        srand(time(NULL));
        int r = rand() % getRowPointerSize();
        return_value = row_pointer[r];
    }
    else {
        return_value = row_pointer[0];
    }
    return return_value;
}

// #ifdef USE_MIDDLEWARE
// // methods to the group id
// int Graph::getMyGroupId() {
//     return this->_my_access_group;
// }
// #endif


// Writing protected setter methods. Only this and its children should be able
// to set the private variables. The setter methods SETS BOTH THE MMAP and THE
// PRIVATE variables!
void Graph::setV(int value) {
    this->_metadata[VERTEX] = value;
    _V = value;
}
void Graph::setE(int value) {
    this->_metadata[EDGE] = value;
    _E = value;
}
void Graph::setRowPointerSize(int size) {
    this->_metadata[ROWP_SIZE] = size;
    _size_row_pointer = size;
}
void Graph::setColIndexSize(int size) {
    this->_metadata[COLI_SIZE] = size;
    _size_col_idx = size;
}
void Graph::setWeightsSize(int size) {
    this->_metadata[WEIGHT_SIZE] = size;
    _size_weights = size;
}
}
