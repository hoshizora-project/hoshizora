#include <utility>
#include <iostream>
#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/model/graph.h"
#include "hoshizora/core/dispatcher/bulksyncdispatcher.h"
#include "hoshizora/app/pagerank.h"
#include "hoshizora/core/io/io.h"
#include "hoshizora/core/compress/multiple_encode.h"
#include "hoshizora/core/compress/multiple_decode.h"

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

        a32_vector<u32> ints;
        a32_vector<u32> offsets = {0,100,200};
        for(u32 j=0;j<3;++j){for(u32 i=0;i<100;++i){ints.emplace_back(i);}}
        a32_vector<u8> compressed(600);
        debug::logger->info("enc: {}", ints.size());
        encode(ints.data(),offsets.data(),3,compressed.data());
        a32_vector<u32> decompressed(300);
        a32_vector<u32> decompressed_offsets(3);
        debug::logger->info("dec");
        decode(compressed.data(),3,decompressed.data(),decompressed_offsets.data());
        for(const auto &i: decompressed)debug::logger->info(i);
    }
}

int main(int argc, char *argv[]) {
    hoshizora::main(argc, argv);
    return 0;
}
