#include <iostream>
#include <string>
#include <cassert>

#include "graph.hh"
#include "dfs.hh"
#include "dfs_hpc.hh"
#include "sssp.hh"
#include "bfs.hh"
#include "bc.hh"
#include "tc.hh"
#include "pr.hh"
#include "cc.hh"
#include "lookup.hh"

#include "cc_hpc.hh"

#include "../ext/cpp-arg-parse/src/argparse.hh"

// This is tuned version to work with the SNAP7/Twitter graph. Additional
// features include loading the graph and resuming the previously loaded graph!

using namespace simple;
int main(int argc, char *argv[]) {
    // Parse the input arguments
    std::string info = "This program disaggregated processes graphs!";

    // Write the expected number of arguments. Keeping it simple!
    int expected_count = 10;

    // Argparse is ready to be initialized.
    Argparse args(argc, expected_count, info);

    // Allocate memory for the parser and initialize the arguments.
    args.allocArgs(expected_count);
    args.initArgs("-a", "--algorithm", "algorithm to run",
                                "bc, bfs, cc, dfs, pr, sssp, tc, allocator");
    args.initArgs("-g", "--graph", "path to a graph. expects the path to"
            " twitter/SNAP7 graph in el format", "");
    // Twitter is a huge graph, there needs to be a feature to load the graph
    // once in the memory and future runs will simply read the graph directly.
    args.initArgs("-l", "--load", "in case the graph needs to be loaded into"
            " the memory again.", "[false], true");
    args.initArgs("-t", "--total-hosts", "total number of hosts", "");
    args.initArgs("-i", "--host-id", "ID of the current host", "");
    // special arguments
    args.initArgs("-h", "--help", "display this message", "");
    args.initArgs("-v", "--verbose", "enable verbose", "");
    args.initArgs("-z", "--test-mode",
                "enable testing mode (uses shmem interface)", "[false], true");
    args.initArgs("-r", "--randomize", "randomize the starting node",
                                                            "[false], true");
    args.initArgs("-d", "--display", "display output in stdout",
                                                            "false, [true]");    

    args.setArgs(argv);

    // All arguments parsed! Now create the initial errors!
    // check if there is --help or --verbose present in the initial string.
    if(args.getArgs("-h") == "1") {
        // help requested.
        args.printHelpArgs();
        return 0;
    }
    // See if the user wants test and verbose enabled.
    bool verbose = false,
         test_mode = false,
         randomize = false,
         display = true,
         load = false;

    if(args.getArgs("-v") == "1")
        verbose = true;

    if (args.getArgs("-z") == "true")
        test_mode = true;
    else {
        // if verbose is true, then inform the user that there will be no
        // testing
        if (verbose)
            std::cout << "info: testing mode is disabled. " <<
                        "Expecting /dev/dax0.0 is valid." << std::endl; 
    }

    if (args.getArgs("-r") == "true")
        randomize = true;
    if (args.getArgs("-d") == "false")
        display = false;

    if (args.getArgs("-l") == "true")
        load = true;

    // convert host id
    int host_id = -1;
    try {
        host_id = std::stoi(args.getArgs("-i"));
    }
    catch (std::exception &err) {
        std::cout << "Illegal host!" << std::endl;
        // tell the user how to use the program
        args.printHelpArgs();
        // exit the program graccefully
        return -1;
    }

    // ---------------------- end of preprocessing ------------------------- //

    // Create a Graph object for the given graph. This is when the entire graph
    // is loaded. We need to implement a middleware, where we specify a graph,
    // and, a user_group which then determines what part of the graph am i
    // supposed to access.

    // graph_t *graph = new graph_t;

    // 20 GiB. The twitter graph needs < 6 GiB to be loaded in the memory. use
    // the util program to load the graph in the shared memory as the allocator 
    size_t size = 0x500000000;
    Graph *G = new Graph(args.getArgs("-g"),
                                            size,
                                            host_id,
                                            test_mode,
                                            randomize,
                                            display,
                                            verbose,
                                            load);

    // The graph is allocated or is being allocated. The workers now can go
    // ahead and start the work specified. Notify the user that work is
    // starting soon.
    std::cout << "======= Graph Processing Started for ";

    // Determine the algorithm to run. Ideally we need a Worker class to
    // just maintain one single worker object.
    // TODO: We need visitor classes to simplify the structure of the workers.
    // If there is a parent class, then it'll be easy for use to cast it!
    if (args.getArgs("-a") == "bfs") {
        // BFS code
        std::cout << "BFS =======" << std::endl; 
        BFS *worker = new BFS(G, G->getStartingNode());
        delete worker;
    }
    else if (args.getArgs("-a") == "dfs") {
        // DFS code
        std::cout << "DFS =======" << std::endl;
        DFS_HPC *worker = new DFS_HPC(G);
        // DFS *worker = new DFS(G, G->getStartingNode());
        delete worker;
    }
    else if (args.getArgs("-a") == "sssp") {
        // SSSP case
        std::cout << "SSSP =======" << std::endl; 
        SSSP *worker = new SSSP(G, G->getStartingNode());
        delete worker;
    }
    else if (args.getArgs("-a") == "bc") {
        // BC case
        std::cout << "BC =======" << std::endl; 
        BC *worker = new BC(G);
        delete worker;
    }
    else if (args.getArgs("-a") == "tc") {
        // TC case
        std::cout << "TC =======" << std::endl; 
        TC *worker = new TC(G);
        delete worker;
    }
    else if (args.getArgs("-a") == "pr") {
        std::cout << "PR =======" << std::endl;
        PR *worker = new PR(G);
        delete worker;
    }
    else if (args.getArgs("-a") == "cc") {
        std::cout << "CC =======" << std::endl;
        CC_HPC *worker = new CC_HPC(G);
        worker->printCC(G);
        delete worker;
    }
    else if (args.getArgs("-a") == "allocator") {
        assert(host_id == 0);
        // The host should keep the program alive until all workers are done.
        int dummy;
        std::cout << " $ prompt $ waiting for the master to end!" << std::endl;
        std::cin >> dummy;
    }
    else if (args.getArgs("-a") == "lookup") {
        std::cout << "Lookup =======" << std::endl;
        const auto start{std::chrono::steady_clock::now()};
        Lookup *worker = new Lookup(G, "uniform");
        const auto finish{std::chrono::steady_clock::now()};
        // print stats?
        const std::chrono::duration<double> elapsed_seconds{finish - start};
        std::cout << "Algorithm " << args.getArgs("-a") << " took " <<
                        elapsed_seconds.count() << " secods" << std::endl;
        worker->displayRandomResults();
        delete worker;
    }
    else {
        std::cout << "Unkonwn algo: " << args.getArgs("-a") << std::endl;
        return -1;
    }


    // Clear the memory? This will delete the local copy of the object. But
    // make sure that the master does not clear the memory.
    if (host_id != 0)
        delete G;
    else {
        // can the master end without munmapping the mmap??
        // TODO: Remove this when finalizing the code for sharing
        delete G;
    }
    return 0;
}
