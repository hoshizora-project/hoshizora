#ifndef HOSHIZORA_BULKSYNC_THREAD_POOL_H
#define HOSHIZORA_BULKSYNC_THREAD_POOL_H

#include <atomic>
#include <cassert>
#include <functional>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <vector>
#ifdef __linux__
#include <sched.h>
#elif __APPLE__
#include <mach/thread_act.h>
#endif
#include "hoshizora/core/condition_variable_barrier.h"
#include "hoshizora/core/includes.h"
#include "hoshizora/core/spin_barrier.h"

namespace hoshizora {
struct BulkSyncThreadPool {
  std::vector<std::thread> pool;
  std::vector<std::queue<std::function<void()>> *> task_queues;
  bool quit_flag = false;
  bool force_quit_flag = false;
  u32 num_threads;
  std::mutex mtx; // TODO
  spin_barrier barrier;
  // condition_variable_barrier barrier;

  explicit BulkSyncThreadPool(u32 num_threads)
      : num_threads(num_threads), barrier(num_threads) {
    for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
      auto queue = new std::queue<std::function<void()>>();
      queue->push([&, thread_id]() {
      // set own thread affinity
#ifdef __linux__
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(thread_id, &cpuset); // FIXME
        sched_setaffinity(syscall(SYS_gettid), sizeof(cpu_set_t), &cpuset);
#elif __APPLE__
      // FIXME: not work properly
      // const auto policy = thread_affinity_policy_data_t{
      //        static_cast<i32>(thread_id)
      //}; // FIXME
      // thread_policy_set(pthread_mach_thread_np(pool[thread_id].native_handle()),
      //                  THREAD_AFFINITY_POLICY,
      //                  (thread_policy_t) &policy,
      //                  THREAD_AFFINITY_POLICY_COUNT);
#else
        debug::logger->info("No thread affinity")
#endif
      });
      task_queues.emplace_back(queue);
    }

    for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
      pool.emplace_back(std::thread([&, thread_id]() {
        {
          std::stringstream system_thread_id;
          system_thread_id << std::this_thread::get_id();
          SPDLOG_DEBUG(debug::logger, "created[{}, {}]", thread_id,
                       system_thread_id.str());
        }

        while (!force_quit_flag &&
               !(quit_flag && task_queues[thread_id]->empty())) {
          if (task_queues[thread_id]->empty())
            continue;

          const auto &task = task_queues[thread_id]->front();
          task();

          mtx.lock();
          SPDLOG_DEBUG(debug::logger, "done[{}] on CPU{}", thread_id,
                       sched::get_cpu_id());
          // TODO: delete task
          task_queues[thread_id]->pop(); // TODO: concurrent_queue
          mtx.unlock();
          barrier.wait(thread_id);
        }
      }));
    }
  }

  void push_task(std::function<void()> task) {
    mtx.lock();
    task_queues[0]->push(std::move(task));
    mtx.unlock();

    for (u32 n = 1; n < num_threads; ++n) {
      mtx.lock();
      task_queues[n]->push([]() {});
      mtx.unlock();
    }
  }

  void push_tasks(std::vector<std::function<void()>> *tasks) {
    assert(num_threads == tasks->size());

    for (u32 n = 0; n < num_threads; ++n) {
      mtx.lock();
      task_queues[n]->push(std::move((*tasks)[n]));
      mtx.unlock();
    }
  }

  void quit() {
    quit_flag = true;
    for (auto &thread : pool) {
      thread.join();
    }
  }

  void force_quit() {
    force_quit_flag = true;
    for (auto &thread : pool) {
      thread.join();
    }
  }
};
} // namespace hoshizora

#endif // HOSHIZORA_BULKSYNC_THREAD_POOL_H
