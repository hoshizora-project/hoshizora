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
        const u32 num_iters = argc > 1 ? (u32) std::strtol(argv[1], nullptr, 10) : 1000;
        debug::point("started");
        //auto edge_list = IO::fromFile0("../../data/email-Eu-core.hszr");
        auto edge_list = IO::fromFile0("../../data/web-Google.hszr");
        debug::point("loaded");
        auto graph = _Graph::FromEdgeList(edge_list.data(), edge_list.size());
        debug::point("converted");
        BulkSyncDispatcher<PageRankKernel<_Graph>> dispatcher(graph, num_iters);
        const auto result = dispatcher.run();
        debug::point("done");
        debug::logger->info(result);

        debug::print("started", "loaded");
        debug::print("loaded", "converted");
        debug::print("converted", "done");
        loop::quit();
    }
}

int main(int argc, char *argv[]) {
    hoshizora::main(argc, argv);
    return 0;
}