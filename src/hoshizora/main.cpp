#include <utility>
#include <iostream>
#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/model/graph.h"
#include "hoshizora/core/dispatcher/bulksyncdispatcher.h"
#include "hoshizora/app/pagerank.h"
#include "hoshizora/core/io/io.h"

using namespace std;

namespace hoshizora {
    void main() {
        using _Graph = Graph<u32, empty_t, empty_t, f32, f32>;
        debug::init_logger();
#ifdef SUPPORT_NUMA
        mem::init_allocators();
#endif
        debug::logger->info("started");
        auto edge_list = IO::fromFile0("../../data/email-Eu-core.hszr");
        //auto edge_list = IO::fromFile0("../../data/web-Google.hszr");
        debug::logger->info("loaded");
        auto graph = _Graph::FromEdgeList(edge_list.data(), edge_list.size());
        debug::logger->info("converted");
        BulkSyncDispatcher<PageRankKernel<_Graph>> dispatcher(graph);
        debug::logger->info(dispatcher.run());
        loop::quit();
        debug::logger->info("done");
    }
}

int main() {
    hoshizora::main();
    return 0;
}