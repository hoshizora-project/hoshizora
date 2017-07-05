#include <utility>
#include <iostream>
#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/model/graph.h"
#include "hoshizora/core/dispatcher/bulksyncdispatcher.h"
#include "hoshizora/app/pagerank.h"
#include "hoshizora/core/allocator/sharedmemoryallocator.h"
#include "hoshizora/core/io/io.h"

using namespace std;

namespace hoshizora {
    void main() {
        using _Graph = Graph<u32, skip_t, skip_t, f32, f32>;
        auto edge_list = IO::fromFile("edge_list");
        auto graph = _Graph::FromEdgeList(edge_list.data(), edge_list.size());
        BulkSyncDispatcher<PageRankKernel<_Graph>, SharedMemoryAllocator> dispatcher(graph);
        cout << dispatcher.run() << endl;
    }
}

int main() {
    hoshizora::main();
    return 0;
}