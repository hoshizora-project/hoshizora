#include <utility>
#include <iostream>
#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/model/graph.h"
#include "hoshizora/core/dispatcher/bulksyncdispatcher.h"
#include "hoshizora/app/pagerank.h"
#include "hoshizora/core/io/io.h"

using namespace std;

namespace hoshizora {
    void main(int argc, char *argv[]) {
        using _Graph = Graph<u32, empty_t, empty_t, f32, f32>;
        init();
        const u32 num_iters = argc > 1 ? (u32) std::atoi(argv[1]) : 1000;
        debug::logger->info("started");
        //auto edge_list = IO::fromFile0("../../data/email-Eu-core.hszr");
        auto edge_list = IO::fromFile0("../../data/web-Google.hszr");
        debug::logger->info("loaded");
        auto graph = _Graph::FromEdgeList(edge_list.data(), edge_list.size());
        debug::logger->info("converted");
        BulkSyncDispatcher<PageRankKernel<_Graph>> dispatcher(graph, num_iters);
        debug::logger->info(dispatcher.run());
        loop::quit();
        debug::logger->info("done");
    }
}

int main(int argc, char *argv[]) {
    hoshizora::main(argc, argv);
    return 0;
}