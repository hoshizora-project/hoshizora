#ifndef HOSHIZORA_GRAPH_H
#define HOSHIZORA_GRAPH_H

#include <tuple>
#include <algorithm>
#include <vector>
#include <assert.h>
#include "hoshizora/core/util/includes.h"

using namespace std;

namespace hoshizora {
    template<class ID,
            class VProp, class EProp,
            class VData, class EData,
            bool IsDirected = true>
    struct Graph {
        using _ID = ID;
        using _VProp = VProp;
        using _EProp = EProp;
        using _VData = VData;
        using _EData = EData;
        using _Graph = Graph<ID, VProp, EProp, VData, EData, IsDirected>;

        ID num_vertices;
        ID num_edges;

        ID *out_degrees; // [num_vertices]
        ID *out_offsets; // [num_vertices]
        ID *out_data; // [num_edges]
        ID *in_degrees;
        ID *in_offsets;
        ID *in_data;
        ID **in_neighbors;
        ID **out_neighbors; // [num_vertices][degrees[i]]
        ID *forward_indices; // [num_edges]
        VProp *v_props; // [num_vertices]
        EProp *e_props; // [num_edges]
        VData *v_data; // [num_vertices]
        EData *e_data; // [num_edges]
        bool *active_flags; // [num_vertices]

        Graph() {}

        Graph(ID num_vertices, ID num_edges,
              ID *out_offsets, ID *out_data,
              ID *forward_indices,
              VProp *v_props, EProp *e_props,
              VData *v_data, EData *e_data
        ) : num_vertices(num_vertices), num_edges(num_edges),
            out_offsets(out_offsets), out_data(out_data),
            forward_indices(forward_indices),
            v_props(v_props), e_props(e_props),
            v_data(v_data), e_data(e_data),
            active_flags(heap::array0<bool>(num_vertices)) {
//            set_degrees();
//            set_in_degrees();
//            set_neighbors();
        }

        Graph(ID num_vertices, ID num_edges,
              ID *out_offsets, ID *out_data,
              ID *forward_indices,
              VProp *v_props, EProp *e_props,
              VData *v_data, EData *e_data,
              bool *active_flags
        ) : num_vertices(num_vertices), num_edges(num_edges),
            out_offsets(out_offsets), out_data(out_data),
            forward_indices(forward_indices),
            v_props(v_props), e_props(e_props),
            v_data(v_data), e_data(e_data),
            active_flags(active_flags) {
            set_degrees();
            set_in_degrees();
            set_neighbors();
            set_forward_indices();
            set_v_data();
            set_e_data();
        }

        inline void set_degrees() {
            out_degrees = heap::array<ID>(num_vertices);

            for (ID i = 0, end = num_vertices - 1; i < end; ++i) {
                out_degrees[i] = out_offsets[i + 1] - out_offsets[i];
            }
            out_degrees[num_vertices - 1] = num_edges - out_offsets[num_vertices - 1];
        }

        inline void set_in_degrees() {
            in_degrees = heap::array<ID>(num_vertices);

            for (ID i = 0, end = num_vertices - 1; i < end; ++i) {
                in_degrees[i] = in_offsets[i + 1] - in_offsets[i];
            }
            in_degrees[num_vertices - 1] = num_edges - in_offsets[num_vertices - 1];
        }

        inline void set_in_neighbors() {
            in_neighbors = heap::array<ID *>(num_vertices);
            ID offset = 0; // <= num_edges
            for (ID i = 0; i < num_vertices; ++i) {
                in_neighbors[i] = &in_data[in_offsets[i]];
            }
        }

        inline void set_neighbors() {
            out_neighbors = heap::array<ID *>(num_vertices);
            ID offset = 0; // <= num_edges
            for (ID i = 0; i < num_vertices; ++i) {
                out_neighbors[i] = &out_data[out_offsets[i]];
            }
        }

        inline void set_forward_indices() {
            forward_indices = heap::array<ID>(num_edges);

            ID counts[num_vertices];
            for (ID i = 0; i < num_vertices; ++i) {
                counts[i] = 0;
            }

            for (ID from = 0; from < num_vertices; ++from) {
                for (ID j = 0, end = out_degrees[from]; j < end; ++j) {
                    auto to = out_neighbors[from][j];
                    forward_indices[out_offsets[from] + j] = in_offsets[to] + counts[to];
                    counts[to]++;
                }
            }
        }

        inline void set_v_data() {
            v_data = heap::array0<VData>(num_vertices);
        }

        inline void set_e_data() {
            e_data = heap::array0<EData>(num_edges);
        }

        static _Graph Empty() {
            return _Graph();
        }

        static _Graph Next(_Graph &prev) {
            /*
            auto gg = _Graph(
                    prev.num_vertices, prev.num_edges,
                    prev.out_offsets, prev.out_data,
                    prev.forward_indices,
                    prev.v_props, prev.e_props,
//                    heap::array0<VProp>(prev.num_vertices),
//                    heap::array0<EProp>(prev.num_edges),

                    nullptr, nullptr,
//                    std::is_same<VProp, skip_t>::value
//                    ? nullptr
//                    : heap::array0<VProp>(prev.num_vertices),
//                    std::is_same<EProp, skip_t>::value
//                    ? nullptr
//                    : heap::array0<EProp>(prev.num_edges),
                    prev.active_flags);
                    */
            auto g = _Graph::Empty();
            g.num_vertices = prev.num_vertices;
            g.num_edges = prev.num_edges;
            g.out_degrees = prev.out_degrees;
            g.out_offsets = prev.out_offsets;
            g.out_neighbors = prev.out_neighbors;
            g.out_data = prev.out_data;
            g.in_degrees = prev.in_degrees;
            g.in_data = prev.in_data;
            g.in_offsets = prev.in_offsets;
            g.in_neighbors = prev.in_neighbors;
            g.active_flags = prev.active_flags;
            g.set_v_data();
            g.set_e_data();
            return g;
        }

        template<class Executor>
        static _Graph FromEdgeListWithExecutor() {
            auto hoge = Executor::alloc();
            return Empty(); //
        }

        // TODO
        static _Graph FromEdgeList(const std::pair<ID, ID> *edge_list,
                                   size_t len) {
            assert(len > 0 && edge_list != nullptr);

            using VVType = std::pair<ID, ID>;
            auto vec = std::vector<VVType>(edge_list, edge_list + len); // [(from, to)]
            std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.second < r.second;
            });
            auto tmp_max = vec.back().second;
            std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.first < r.first;
            });
            auto num_vertices = std::max(tmp_max, vec.back().first) + 1; // 0-based
            auto num_edges = len;

            auto out_offsets = heap::array<ID>(num_vertices);
            auto out_data = heap::array<ID>(num_edges);

            auto src = 0u;
            out_offsets[src++] = 0u;
            auto prev_src = vec[0].first;
            for (auto i = 0u; i < num_edges; ++i) {
                auto curr = vec[i];
                if (prev_src != curr.first) {
                    prev_src = curr.first;
                    out_offsets[src++] = i;
                }
                out_data[i] = curr.second;
            }
            for (auto i = src; i <= num_vertices; ++i) {
                out_offsets[i] = num_edges;
            }

            // set inverse
            for (auto &p:vec) {
                std::swap(p.first, p.second);
            }
            std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.second < r.second;
            });
            std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.first < r.first;
            });
            auto in_offsets = heap::array<ID>(num_vertices);
            auto in_data = heap::array<ID>(num_edges);
            src = 0u;
            in_offsets[src++] = 0u;
            prev_src = vec[0].first;
            for (auto i = 0u; i < num_edges; ++i) {
                auto curr = vec[i];
                if (prev_src != curr.first) {
                    prev_src = curr.first;
                    in_offsets[src++] = i;
                }
                in_data[i] = curr.second;
            }
            for (auto i = src; i <= num_vertices; ++i) {
                in_offsets[i] = num_edges;
            }

            auto g = _Graph::Empty();
            g.num_vertices = num_vertices;
            g.num_edges = num_edges;
            g.out_offsets = out_offsets;
            g.out_data = out_data;
            g.in_offsets = in_offsets;
            g.in_data = in_data;
            g.set_degrees();
            g.set_neighbors();
            g.set_in_degrees();
            g.set_in_neighbors();
            g.set_forward_indices();
            g.set_v_data();
            g.set_e_data();
            return g;
        }
    };
}


#endif //HOSHIZORA_GRAPH_H