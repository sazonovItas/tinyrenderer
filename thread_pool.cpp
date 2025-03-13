#include "thread_pool.h"

MT::Task::Task() {
  id = 0;
  status = MT::Task::TaskStatus::awaiting;
  thread_pool = nullptr;
}

void MT::Task::one_thread_pre_method() {
  doWork();
  status = MT::Task::TaskStatus::completed;
}

MT::ThreadPool::ThreadPool(int cntThreads) {
  stopped = false;
  paused = true;
  last_task_id = 0;
  for (int i = 0; i < cntThreads; i++) {
    MT::Thread *th = new MT::Thread;
    th->_thread = std::thread{&ThreadPool::run, this, th};
    th->is_working = false;
    threads.push_back(th);
  }
}

MT::ThreadPool::~ThreadPool() {
  stopped = true;
  tasks_access.notify_all();
  for (auto &thread : threads) {
    thread->_thread.join();
    delete thread;
  }
}

bool MT::ThreadPool::run_allowed() const {
  return (!task_queue.empty() && !paused);
}

void MT::ThreadPool::run(MT::Thread *_thread) {
  while (!stopped) {
    std::unique_lock<std::mutex> lock(task_queue_mutex);

    _thread->is_working = false;
    tasks_access.wait(lock,
                      [this]() -> bool { return run_allowed() || stopped; });
    _thread->is_working = true;

    if (run_allowed()) {
      auto elem = std::move(task_queue.front());
      task_queue.pop();
      lock.unlock();

      elem->one_thread_pre_method();

      completed_task_count++;
    }

    wait_access.notify_all();
  }
}

void MT::ThreadPool::start() {
  if (paused) {
    paused = false;
    tasks_access.notify_all();
  }
}

void MT::ThreadPool::pause() { paused = true; }

void MT::ThreadPool::stop() { paused = true; }

void MT::ThreadPool::wait() {
  std::lock_guard<std::mutex> lock_wait(wait_mutex);

  start();

  std::unique_lock<std::mutex> lock(task_queue_mutex);
  wait_access.wait(lock, [this]() -> bool { return is_completed(); });

  pause();
}

bool MT::ThreadPool::is_completed() const {
  return completed_task_count == last_task_id;
}

bool MT::ThreadPool::is_standby() const {
  if (!paused)
    return false;

  for (const auto &thread : threads)
    if (thread->is_working)
      return false;

  return true;
}
