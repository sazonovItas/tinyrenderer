#pragma once

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <queue>
#include <vector>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace MT {
typedef unsigned long long int task_id;

class ThreadPool;

class Task {
public:
  enum class TaskStatus { awaiting, completed };

  Task();

  virtual void doWork() = 0;

protected:
  MT::Task::TaskStatus status;
  MT::task_id id;

  friend class ThreadPool;

  ThreadPool *thread_pool;

  void one_thread_pre_method();
};

struct Thread {
  std::thread _thread;
  std::atomic<bool> is_working;
};

class ThreadPool {
public:
  ThreadPool(int cntThreads);

  template <typename TaskChild> MT::task_id addTask(const TaskChild &task) {
    std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push(std::make_shared<TaskChild>(task));
    task_queue.back()->id = ++last_task_id;
    task_queue.back()->thread_pool = this;
    tasks_access.notify_one();
    return last_task_id;
  }

  void wait();

  void stop();

  void pause();

  void start();

  ~ThreadPool();

private:
  std::mutex task_queue_mutex;
  std::mutex wait_mutex;

  std::condition_variable tasks_access;
  std::condition_variable wait_access;

  std::vector<MT::Thread *> threads;
  std::queue<std::shared_ptr<Task>> task_queue;
  MT::task_id last_task_id;

  std::atomic<bool> stopped;
  std::atomic<bool> paused;
  std::atomic<unsigned long long> completed_task_count;

  void run(MT::Thread *thread);

  bool run_allowed() const;

  bool is_completed() const;

  bool is_standby() const;
};

} // namespace MT

#endif
