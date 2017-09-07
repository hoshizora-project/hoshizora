#ifndef HOSHIZORA_GRAPH_H
#define HOSHIZORA_GRAPH_H

#include <tuple>
#include <algorithm>
#include <vector>
#include <cassert>
#include <thread>
#include <iostream>
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

        ID *tmp_out_offsets;
        ID *tmp_out_indices;
        ID *tmp_in_offsets;
        ID *tmp_in_indices;

        heap::DiscreteArray<ID> out_degrees; // [num_vertices]
        heap::DiscreteArray<ID> out_offsets; // [num_vertices]
        heap::DiscreteArray<ID> out_indices; // [num_edges]
        heap::DiscreteArray<ID *> out_neighbors; // [num_vertices][degrees[i]]
        heap::DiscreteArray<ID> in_degrees;
        heap::DiscreteArray<ID> in_offsets;
        heap::DiscreteArray<ID> in_indices;
        heap::DiscreteArray<ID *> in_neighbors;
        ID *forward_indices; // [num_edges]
        u32 *out_boundaries;
        u32 *in_boundaries;
        VProp *v_props; // [num_vertices]
        EProp *e_props; // [num_edges]
        heap::DiscreteArray<VData> v_data; // [num_vertices]
        heap::DiscreteArray<EData> e_data; // [num_edges]
        heap::DiscreteArray<bool> active_flags; // [num_vertices]

        bool out_degrees_is_initialized = false;
        bool out_offsets_is_initialized = false;
        bool out_indices_is_initialized = false;
        bool out_neighbors_is_initialized = false;
        bool in_degrees_is_initialized = false;
        bool in_offsets_is_initialized = false;
        bool in_indices_is_initialized = false;
        bool in_neighbors_is_initialized = false;
        bool out_boundaries_is_initialized = false;
        bool in_boundaries_is_initialized = false;
        bool forward_indices_is_initialized = false;

        Graph() : out_degrees(heap::DiscreteArray<ID>()),
                  out_neighbors(heap::DiscreteArray<ID *>()),
                  in_degrees(heap::DiscreteArray<ID>()),
                  in_neighbors(heap::DiscreteArray<ID *>()) {}

        u32 num_numa_nodes = 2;

        void set_out_offsets() {
            assert(out_boundaries_is_initialized);

            // FIXME
            if (num_numa_nodes == 1) {
                out_offsets.add(tmp_out_offsets, num_vertices);
            } else {
                /*
                u32 start = 0;

                for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                    const auto numa_id = mock::thread_to_numa(thread_id);
                    if (thread_id == num_threads - 1
                        || mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {

                        // to avoid free (?)
//                        auto offsets = std::vector<ID>(out_boundaries[thread_id + 1] - start);
//                        std::copy(tmp_out_offsets + start,
//                                  tmp_out_offsets + out_boundaries[thread_id + 1],
//                                  std::back_inserter(offsets));
//                        out_offsets.add(offsets);
                        const auto size = out_boundaries[thread_id + 1] - start;
                        auto offsets = heap::array<u32>(size);
                        std::memcpy(offsets, tmp_out_offsets + start, size);
                        out_offsets.add(offsets, size);
                        start = out_boundaries[thread_id + 1];
                    }
                }*/

                parallel::each_numa_node(out_boundaries,
                                         [&](u32 numa_id, u32 lower, u32 upper) mutable {
                                             const auto size = upper - lower;
                                             const auto offsets = heap::array<u32>(size);
                                             std::memcpy(offsets, tmp_out_offsets + lower, size * sizeof(u32));
                                             out_offsets.add(offsets, size);
                                         });

                out_offsets_is_initialized = true;
                free(tmp_out_offsets); // TODO: maybe numa_free
            }
        }

        void set_in_offsets() {
            assert(in_boundaries_is_initialized);

            // FIXME
            if (num_numa_nodes == 1) {
                in_offsets.add(tmp_in_offsets, num_vertices);
            } else {
                /*
                u32 start = 0;

                for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                    const auto numa_id = mock::thread_to_numa(thread_id);
                    if (thread_id == num_threads - 1
                        || mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {

                        // to avoid free (?)
//                        auto offsets = std::vector<ID>(in_boundaries[thread_id + 1] - start);
//                        std::copy(tmp_in_offsets + start,
//                                  tmp_in_offsets + in_boundaries[thread_id + 1],
//                                  std::back_inserter(offsets));
//                        in_offsets.add(offsets);
                        const auto size = in_boundaries[thread_id + 1] - start;
                        auto offsets = heap::array<u32>(size);
                        std::memcpy(offsets, tmp_in_offsets + start, size * sizeof(u32));
                        in_offsets.add(offsets, size);
                        start = in_boundaries[thread_id + 1];
                    }
                }*/

                parallel::each_numa_node(in_boundaries,
                                         [&](u32 numa_id, u32 lower, u32 upper) mutable {
                                             const auto size = upper - lower;
                                             const auto offsets = heap::array<u32>(size);
                                             std::memcpy(offsets, tmp_in_offsets + lower, size * sizeof(u32));
                                             in_offsets.add(offsets, size);
                                         });

                in_offsets_is_initialized = true;
                free(tmp_in_offsets); // TODO: maybe numa_free
            }
        }

        void set_out_indices() {
            assert(out_boundaries_is_initialized);
            assert(out_offsets_is_initialized);

            // FIXME
            if (num_numa_nodes == 1) {
                out_indices.add(tmp_out_indices, num_edges);
            } else {
                /*
                u32 start = 0;

                for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                    const auto numa_id = mock::thread_to_numa(thread_id);
                    if (thread_id == num_threads - 1
                        || mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {

                        // to avoid free (?)
//                        auto indices = std::vector<ID>(
//                                out_offsets(out_boundaries[thread_id + 1], numa_id) - start);
//                        std::copy(tmp_out_indices + start,
//                                  tmp_out_indices + out_offsets(out_boundaries[thread_id + 1], numa_id),
//                                  std::back_inserter(indices));
//                        out_indices.add(indices);

//                        out_indices.add(tmp_out_indices,
//                                        out_offsets(out_boundaries[thread_id + 1], numa_id) - start);


                        const auto size =
                                out_offsets(out_boundaries[thread_id + 1], numa_id) - start;
                        auto indices = heap::array<u32>(size);
                        std::memcpy(indices, tmp_out_indices + start, size);
                        out_indices.add(indices, size);

                        start = out_offsets(out_boundaries[thread_id + 1], numa_id);
                    }
                }*/

                parallel::each_numa_node(out_boundaries,
                                         [&](u32 numa_id, u32 lower, u32 upper) mutable {
                                             const auto start = out_offsets(lower, numa_id);
                                             const auto end = out_offsets(upper, numa_id);
                                             const auto size = end - start;
                                             const auto indices = heap::array<u32>(size);
                                             std::memcpy(indices, tmp_out_indices + start, size * sizeof(u32));
                                             out_indices.add(indices, size);
                                         });

                out_indices_is_initialized = true;
                free(tmp_out_indices); // TODO: maybe numa_free
            }
        }

        void set_in_indices() {
            assert(in_boundaries_is_initialized);
            assert(in_offsets_is_initialized);

            // FIXME
            if (num_numa_nodes == 1) {
                in_indices.add(tmp_in_indices, num_edges);
            } else {
                /*
                u32 start = 0;

                for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                    const auto numa_id = mock::thread_to_numa(thread_id);
                    if (thread_id == num_threads - 1
                        || mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {

                        // to avoid free (?)
//                        auto indices = std::vector<ID>(
//                                in_offsets(in_boundaries[thread_id + 1], numa_id) - start);
//                        std::copy(tmp_in_indices + start,
//                                  tmp_in_indices + in_offsets(in_boundaries[thread_id + 1], numa_id),
//                                  std::back_inserter(indices));
//                        in_indices.add(indices);

//                        in_indices.add(tmp_in_indices,
//                                       in_offsets(in_boundaries[thread_id + 1], numa_id) - start);

                        const auto size =
                                in_offsets(in_boundaries[thread_id + 1], numa_id) - start;
                        auto indices = heap::array<u32>(size);
                        std::memcpy(indices, tmp_in_indices + start, size * sizeof(u32));
                        in_indices.add(indices, size);

                        start = in_offsets(in_boundaries[thread_id + 1], numa_id);
                    }
                }
                 */

                parallel::each_numa_node(in_boundaries,
                                         [&](u32 numa_id, u32 lower, u32 upper) mutable {
                                             const auto start = in_offsets(lower, numa_id);
                                             const auto end = in_offsets(upper - 1, numa_id);
                                             const auto size = end - start;
                                             const auto indices = heap::array<u32>(size);
                                             std::memcpy(indices, tmp_in_indices + start, size * sizeof(u32));
                                             in_indices.add(indices, size);
                                         });

                in_indices_is_initialized = true;
                free(tmp_in_indices); // TODO: maybe numa_free
            }
        }

        void set_out_degrees() {
            // for boundaries
            out_boundaries = heap::array<u32>(num_threads + 1);
            out_boundaries[0] = 0;
            u32 thread_id = 0;
            const u32 chunk_size = num_edges / num_threads;
            u32 boundary = chunk_size;

            u32 offset = 0;
            auto out_degree = new std::vector<ID>(); // TODO: numa-local
            out_degree->reserve(chunk_size);

            for (ID i = 0, end = num_vertices - 1; i < end; ++i) {
                out_degree->emplace_back(tmp_out_offsets[i + 1] - tmp_out_offsets[i]);

                // set boundaries
                if (tmp_out_offsets[i + 1] >= boundary) {
                    // FIXME
                    if (mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {
                        // numa-local discrete array
                        out_degrees.add(std::move(*out_degree));
                        out_degree = new std::vector<ID>();
                        out_degree->reserve(chunk_size);
                    }
                    out_boundaries[++thread_id] = i;

                    boundary = thread_id != num_threads
                               // halfway
                               ? (thread_id + 1) * chunk_size
                               // last (never hit on `if (tmp_out_offsets[i + 1] >= boundary)`)
                               : num_edges + 1;
                }
            }

            // end of boundaries
            out_boundaries[num_threads] = num_vertices;

            // end of degrees
            out_degree->emplace_back(num_edges - tmp_out_offsets[num_vertices - 1]);
            out_degrees.add(std::move(*out_degree));

            // finalize
            out_degrees_is_initialized = true;
            out_boundaries_is_initialized = true;
        }

        void set_in_degrees() {
            // for boundaries
            in_boundaries = heap::array<u32>(num_threads + 1);
            in_boundaries[0] = 0;
            u32 thread_id = 0;
            const u32 chunk_size = num_edges / num_threads;
            u32 boundary = chunk_size;

            u32 offset = 0;
            auto in_degree = new std::vector<ID>(); // TODO: numa-local
            in_degree->reserve(chunk_size);

            for (ID i = 0, end = num_vertices - 1; i < end; ++i) {
                in_degree->emplace_back(tmp_in_offsets[i + 1] - tmp_in_offsets[i]);

                // set boundaries
                if (tmp_in_offsets[i + 1] >= boundary) {
                    if (mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {
                        // numa-local discrete array
                        in_degrees.add(std::move(*in_degree));
                        in_degree = new vector<ID>();
                        in_degree->reserve(chunk_size);
                    }
                    in_boundaries[++thread_id] = i;

                    boundary = thread_id != num_threads
                               // halfway
                               ? (thread_id + 1) * chunk_size
                               // last (never hit on `if (tmp_in_offsets[i + 1] >= boundary)`)
                               : num_edges + 1;
                }
            }

            // end of boundaries
            in_boundaries[num_threads] = num_vertices;

            // end of degrees
            in_degree->emplace_back(num_edges - tmp_in_offsets[num_vertices - 1]);
            in_degrees.add(std::move(*in_degree));

            // finalize
            in_degrees_is_initialized = true;
            in_boundaries_is_initialized = true;
            free(tmp_in_offsets); // TODO: maybe numa_free
        }

        void set_out_neighbor() {
            assert(out_indices_is_initialized);
            assert(out_offsets_is_initialized);

//            auto out_neighbor = std::vector<ID *>();
//            out_neighbor.reserve(out_boundaries[1]); // TODO

            // TODO: multithreading
//            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
//                const auto numa_id = mock::thread_to_numa(thread_id);
//                for (ID i = out_boundaries[thread_id], end = out_boundaries[thread_id + 1]; i < end; ++i) {
//                    out_neighbor.emplace_back(&out_indices(out_offsets(i, numa_id), numa_id));
//                }
//            }

            parallel::each_numa_node(out_boundaries,
                                     [&](u32 numa_id, u32 lower, u32 upper) mutable {
                                         auto out_neighbor = new std::vector<ID *>();
                                         out_neighbor->reserve(upper - lower);
                                         for (ID i = lower; i < upper; ++i) {
                                             out_neighbor->emplace_back(
                                                     &out_indices(out_offsets(i, numa_id), numa_id));
                                         }

                                         auto a1 = (*out_neighbor)[456908];
                                         auto a2 = (*out_neighbor)[456909];
                                         auto a3 = (*out_neighbor)[456910];
                                         auto b1 = out_indices(out_offsets(456908, 0), 0);
                                         auto b2 = out_indices(out_offsets(456909, 0), 0);
                                         auto b3 = out_indices(out_offsets(456910, 0), 0);
                                         out_neighbors.add(std::move(*out_neighbor));
                                     });

            auto x1 = out_neighbors(456908, 0);
            auto x2 = out_neighbors(456909, 0);
//            auto x3 = out_neighbors(456910, 0);
            auto x4 = out_neighbors(456911, 1);
            auto y1 = out_degrees(456908, 0);
            auto y2 = out_degrees(456909, 0);
//            auto y3 = out_degrees(456910, 0);
            auto y4 = out_degrees(456911, 1);
            auto z1 = x1[0];
            auto z2 = x2[0];
//            auto z3 = x3[0];
            auto z4 = x4[0];

            out_neighbors_is_initialized = true;
        }

        void set_in_neighbors() {
            assert(in_offsets_is_initialized);
            assert(in_indices_is_initialized);

            /*
            auto in_neighbor = std::vector<ID *>();
            in_neighbor.reserve(in_boundaries[1]); // TODO

            // TODO: multithreading
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                const auto numa_id = mock::thread_to_numa(thread_id);
                for (ID i = in_boundaries[thread_id]; i < in_boundaries[thread_id + 1]; ++i) {
                    in_neighbor.emplace_back(&in_indices(in_offsets(i, numa_id), numa_id));
                }
            }
             */

            parallel::each_numa_node(in_boundaries,
                                     [&](u32 numa_id, u32 lower, u32 upper) mutable {
                                         auto in_neighbor = new std::vector<ID *>();
                                         in_neighbor->reserve(upper - lower);
                                         for (ID i = lower; i < upper; ++i) {
                                             auto x = in_offsets(i, numa_id);
                                             auto y = in_indices(x, numa_id);
                                             in_neighbor->emplace_back(
                                                     &in_indices(in_offsets(i, numa_id), numa_id));
                                         }
                                         in_neighbors.add(std::move(*in_neighbor));
                                     });

            in_neighbors_is_initialized = true;
        }

        void set_forward_indices() {
            assert(out_offsets_is_initialized);
            assert(out_degrees_is_initialized);
            assert(out_neighbors_is_initialized);
            assert(out_boundaries_is_initialized);

            forward_indices = heap::array<ID>(num_edges); // TODO: should be numa-local

            /*
            auto counts = std::vector<ID>(num_vertices, 0);
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                const auto numa_id = mock::thread_to_numa(thread_id);
                for (ID from = out_boundaries[thread_id]; from < out_boundaries[thread_id + 1]; ++from) {
                    for (ID i = 0, end = out_degrees(from, numa_id); i < end; ++i) {
                        const auto to = out_neighbors(from, numa_id)[i];
                        forward_indices[out_offsets(from, numa_id) + i]
                                = out_offsets(to, numa_id) + counts[to];
                        counts[to]++;
                    }
                }
            }
             */

            auto x1 = out_neighbors(456908, 0);
            auto x2 = out_neighbors(456909, 0);
            auto x3 = out_neighbors(456910, 0);
            auto x4 = out_neighbors(456911, 1);
            auto y1 = out_degrees(456908, 0);
            auto y2 = out_degrees(456909, 0);
            auto y3 = out_degrees(456910, 0);
            auto y4 = out_degrees(456911, 1);
            auto z1 = x1[0];
            auto z2 = x2[0];
            auto z3 = x3[0];
            auto z4 = x4[0];

            auto counts = std::vector<ID>(num_vertices, 0);
            parallel::each_thread(out_boundaries,
                                  [&](u32 numa_id, u32 thread_id, u32 lower, u32 upper) { ;
                                      for (ID src = lower; src < upper; ++src) {
                                          const auto neighbor = out_neighbors(src, numa_id);
                                          for (ID i = 0, end = out_degrees(src, numa_id); i < end; ++i) {
                                              const auto dst = neighbor[i];
                                              forward_indices[out_offsets(src, numa_id) + i] =
                                                      out_offsets(dst, numa_id) + counts[dst];
                                              counts[dst]++;
                                          }
                                      }
                                  });

            forward_indices_is_initialized = true;
        }

        void set_v_data() {
            assert(out_boundaries_is_initialized);
            assert(in_boundaries_is_initialized);

            v_data = heap::DiscreteArray<VData>();
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                const auto numa_id = mock::thread_to_numa(thread_id);
                if (thread_id == 0 || mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {
                    // TODO: consider both out and in boundaries (?)
                    // If readonly, it should be allowed that duplicate vertex data
                    // And should be allocated on each numa node
                    const auto size = out_boundaries[thread_id + 1] - out_boundaries[thread_id];
                    const auto datum = mock::numa_alloc_onnode<VData>(sizeof(VData) * size, numa_id);
                    v_data.add(datum, size);
                }
            }
        }

        void set_e_data() {
            assert(out_boundaries_is_initialized);
            assert(out_offsets_is_initialized);
            assert(in_boundaries_is_initialized);
            assert(in_offsets_is_initialized);

            e_data = heap::DiscreteArray<EData>();
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                const auto numa_id = mock::thread_to_numa(thread_id);
                if (mock::thread_to_numa(thread_id) != mock::thread_to_numa(thread_id + 1)) {
                    // TODO: consider both out and in boundaries (?)
                    // If readonly, it should be allowed that duplicate edge data
                    // And should be allocated on each numa node
                    const auto size = out_offsets(out_boundaries[thread_id + 1], numa_id)
                                      - out_offsets(out_boundaries[thread_id], numa_id);
                    const auto datum = mock::numa_alloc_onnode<EData>(sizeof(EData) * size, numa_id);
                    e_data.add(datum, size);
                }
            }
        }

        static void Next(_Graph &prev, _Graph &curr) {
            // TODO
//            std::swap(&prev.v_data, &curr.v_data);
//            std::swap(&prev.e_data, &curr.e_data);
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
            auto out_indices = heap::array<ID>(num_edges);

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
                out_indices[i] = curr.second;
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
            auto in_indices = heap::array<ID>(num_edges);

            in_offsets[0] = 0;
            prev_src = vec[0].first;
            for (u32 i = 0; i < num_edges; ++i) {
                auto curr = vec[i];
                if (prev_src != curr.first) {
                    // loop for vertices whose degree is 0
                    for (auto j = prev_src + 1; j <= curr.first; ++j) {
                        in_offsets[j] = i;
                    }
                    prev_src = curr.first;
                }
                in_indices[i] = curr.second;
            }
            for (auto i = prev_src + 1; i <= num_vertices; ++i) {
                in_offsets[i] = num_edges;
            }

            auto g = _Graph();
            g.num_vertices = num_vertices;
            g.num_edges = num_edges;
            g.tmp_out_offsets = out_offsets;
            g.tmp_out_indices = out_indices;
            g.tmp_in_offsets = in_offsets;
            g.tmp_in_indices = in_indices;
            g.set_out_degrees();
            g.set_out_offsets();
            g.set_out_indices();
            g.set_out_neighbor();
            g.set_in_degrees();
            g.set_in_offsets();
            g.set_in_indices();
            g.set_in_neighbors();
            g.set_forward_indices();
            g.set_v_data();
            g.set_e_data();

            return g;
        }
    };
}


#endif //HOSHIZORA_GRAPH_H