#pragma once

#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
#include <future>
#include <atomic>
#include <vector>
#include <queue>

class RBSem
{
public:
	RBSem(int init_c = 0) :count(init_c) {}
	//wait
	void wait();

	//signal
	void signal();


  inline void reset()
  {
    count = 0;
  }
private:
	int count;
	std::condition_variable cv;
	std::mutex mx;

};

#if 0 
class RBTaskExecutor
{
private:

  typedef std::function<void() > Task;

	std::vector<std::thread> pool;
	std::queue<Task> tasks;
	std::mutex m_task;
	std::condition_variable cv_task;
	std::atomic<bool> stop;
	std::atomic<int> runned;

public:
	void get_thread_ids(std::vector<std::thread::id>& v);


  RBTaskExecutor(size_t size = 4); 

  ~RBTaskExecutor();


  void shutdown();


  void restart();


	template<class F,class... Args>
	auto commit(F&& f,Args&&... args)->std::future<decltype(f(args...))>
	{
		typedef decltype(f(args...)) ResT;
		auto task = std::make_shared<std::packaged_task<ResT()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);
		{
			std::lock_guard<std::mutex> lock(m_task);
			tasks.emplace([task](){(*task)(); });
		}

		cv_task.notify_all();

		std::future<ResT> fut = task->get_future();
		return fut;

	}

	void join()
	{
		for (std::thread& thread : pool)
		{
			thread.join();
		}
	}

private:
	Task get_one_task()
	{
		std::unique_lock<std::mutex> lock(m_task);
		cv_task.wait(lock,[this]()
		{
			return !tasks.empty();
		}
		);
		Task task(std::move(tasks.front()));
		tasks.pop();
		return task;
	}

	void schedual()
	{
		while (true)
		{
			if(Task task = get_one_task())
			{
				
				task();
				runned++;
			}
			else
				return;
		}
	}
};
#endif

class Task
{
public:
  Task(void* arg = NULL, const std::string taskName = "")
    : arg_(arg)
    , taskName_(taskName)
  {
  }
  virtual ~Task()
  {
  }
  void setArg(void* arg)
  {
    arg_ = arg;
  }

  virtual int run(int id) = 0;

protected:
  void*       arg_;
  std::string taskName_;

};


class ThreadPool
{
public:
	ThreadPool(int threadNum = 4);
  ~ThreadPool();
	size_t addTask(Task *task);
	void   stop();
  int    size();
  Task*  take();
  void cancel();

private:
  int createThreads();
  static void* threadFunc(void * arg, int id);


private:
  ThreadPool& operator=(const ThreadPool&);
  ThreadPool(const ThreadPool&);

private:
  volatile  bool              isRunning_;
  int                         threadsNum_;
  std::vector<std::thread> pool;

  std::deque<Task*>           taskQueue_;
  std::mutex mutex_;
  std::condition_variable condition_;
};