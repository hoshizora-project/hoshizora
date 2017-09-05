#ifndef HOSHIZORA_GRAPH_H
#define HOSHIZORA_GRAPH_H

#include <tuple>
#include <algorithm>
#include <vector>
#include <cassert>
#include <thread>
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

        const u32 num_threads = thread::hardware_concurrency(); // TODO

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
        u32 *out_boundaries;
        u32 *in_boundaries;
        VProp *v_props; // [num_vertices]
        EProp *e_props; // [num_edges]
        VData *v_data; // [num_vertices]
        EData *e_data; // [num_edges]
        bool *active_flags; // [num_vertices]

        Graph() = default;

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

            // for boundaries
            out_boundaries = heap::array<u32>(num_threads + 1);
            out_boundaries[0] = 0;
            u32 j = 1;
            const u32 chunk_size = num_edges / num_threads;
            u32 boundary = chunk_size;

            for (ID i = 0, end = num_vertices - 1; i < end; ++i) {
                out_degrees[i] = out_offsets[i + 1] - out_offsets[i];

                // set boundaries
                if (out_offsets[i + 1] >= boundary) {
                    out_boundaries[j++] = i;
                    boundary += chunk_size;
                }
            }

            // end of boundaries
            out_boundaries[num_threads] = num_vertices;

            // end of degrees
            out_degrees[num_vertices - 1] = num_edges - out_offsets[num_vertices - 1];
        }

        inline void set_in_degrees() {
            in_degrees = heap::array<ID>(num_vertices);

            // for boundaries
            in_boundaries = heap::array<u32>(num_threads + 1);
            in_boundaries[0] = 0;
            u32 j = 1;
            const u32 chunk_size = num_edges / num_threads;
            u32 boundary = chunk_size;

            for (ID i = 0, end = num_vertices - 1; i < end; ++i) {
                in_degrees[i] = in_offsets[i + 1] - in_offsets[i];

                // set boundaries
                if (in_offsets[i + 1] >= boundary) {
                    in_boundaries[j++] = i;
                    boundary += chunk_size;
                }
            }

            // end of boundaries
            in_boundaries[num_threads] = num_vertices;

            // end of degrees
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

        static void Next(_Graph &prev, _Graph &curr) {
            swap(prev.v_data, curr.v_data);
            swap(prev.e_data, curr.e_data);
        }

//        template<class Executor>
//        static _Graph FromEdgeListWithExecutor() {
//            auto hoge = Executor::alloc();
//            return Empty(); //
//        }

        // TODO: poor performance
        // *require packed index* (process in preprocessing)
        static _Graph FromEdgeList(const std::pair<ID, ID> *edge_list,
                                   size_t len) {
            assert(len > 0 && edge_list != nullptr);

            using VVType = std::pair<ID, ID>;
            auto vec = std::vector<VVType>(edge_list, edge_list + len); // [(from, to)]

            // outbound
            std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.second < r.second;
            });
            auto tmp_max = vec.back().second;
            std::stable_sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.first < r.first;
            });

            auto num_vertices = std::max(tmp_max, vec.back().first) + 1; // 0-based
            auto num_edges = len;

            auto out_offsets = heap::array<ID>(num_vertices);
            auto out_data = heap::array<ID>(num_edges);

            out_offsets[0] = 0u;
            auto prev_src = vec[0].first;
            for (auto i = 0u; i < num_edges; ++i) {
                auto curr = vec[i];
                if (prev_src != curr.first) {
                    // loop for vertices whose degree is 0
                    for (auto j = prev_src + 1; j <= curr.first; ++j) {
                        out_offsets[j] = i;
                    }
                    prev_src = curr.first;
                }
                out_data[i] = curr.second;
            }
            for (auto i = prev_src + 1; i <= num_vertices; ++i) {
                out_offsets[i] = num_edges;
            }

            // and vice versa (inbound)
            for (auto &p: vec) {
                std::swap(p.first, p.second);
            }
            std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.second < r.second;
            });
            std::stable_sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
                return l.first < r.first;
            });

            auto in_offsets = heap::array<ID>(num_vertices);
            auto in_data = heap::array<ID>(num_edges);

            in_offsets[0] = 0u;
            prev_src = vec[0].first;
            for (auto i = 0u; i < num_edges; ++i) {
                auto curr = vec[i];
                if (prev_src != curr.first) {
                    // loop for vertices whose degree is 0
                    for (auto j = prev_src + 1; j <= curr.first; ++j) {
                        in_offsets[j] = i;
                    }
                    prev_src = curr.first;
                }
                in_data[i] = curr.second;
            }
            for (auto i = prev_src + 1; i <= num_vertices; ++i) {
                in_offsets[i] = num_edges;
            }

            auto g = _Graph();
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