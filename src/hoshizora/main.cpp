#include <iostream>
#include "hoshizora/core/model/graph.h"
#include "hoshizora/core/util/includes.h"
#include <utility>
#include <hoshizora/core/dispatcher/bulksyncdispatcher.h>
#include <hoshizora/app/pagerank.h>
#include <hoshizora/core/allocator/sharedmemoryallocator.h>

using namespace std;

namespace hoshizora {
    void main() {
        auto len = 4u;
        auto a = heap::array<std::pair<u32, u32>>(len);
        a[0] = std::make_pair(1u, 2u);
        a[1] = std::make_pair(0u, 2u);
        a[2] = std::make_pair(1u, 0u);
        a[3] = std::make_pair(0u, 1u);

        using GType = Graph<u32, skip_t, skip_t, f32, f32>;
        auto x = GType::FromEdgeList(a, len);
        cout << "type: " << sizeof(GType::_ID) << endl;
        cout << "type: " << sizeof(GType::_VProp) << endl;

        BulkSyncDispatcher<PageRankKernel<GType>, SharedMemoryAllocator> dispatcher(x);
        cout << dispatcher.run() << endl;

        for (size_t i = 0; i < 4; ++i) {
            cout << x.vertex_data[i] << endl;
        }

        cout << "-----" << endl;

        for (size_t i = 0; i < 3; ++i) {
            cout << x.vertex_offsets[i] << endl;
        }
    }
}

int main() {
    hoshizora::main();
    return 0;
}