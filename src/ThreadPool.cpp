#include "ThreadPool.h"

void RBSem::wait()
{
	std::unique_lock<std::mutex> lock(mx);
	if (--count < 0)
	{
		cv.wait(lock, [&]()->bool {return count >= 0; });
	}
}

void RBSem::signal()
{
	std::lock_guard<std::mutex> lock(mx);
	++count;
	cv.notify_one();

}

#if 0

void RBTaskExecutor::get_thread_ids(std::vector<std::thread::id>& v)
{
	for (int i = 0; i < pool.size(); i++)
	{
		v.push_back(pool[i].get_id());
	}
}

RBTaskExecutor::RBTaskExecutor(size_t size /*= 4*/) : stop(false), runned(0)
{
	size = size < 1 ? 1 : size;
	for (size_t i = 0; i < size; ++i)
	{
		pool.emplace_back(&RBTaskExecutor::schedual, this);
	}
}

RBTaskExecutor::~RBTaskExecutor()
{
	for (std::thread& thread : pool)
	{
		thread.detach();
	}
}

void RBTaskExecutor::shutdown()
{
	this->stop.store(true);
}

void RBTaskExecutor::restart()
{
	this->stop.store(false);
}


#endif





void* ThreadPool::threadFunc(void * arg, int id)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	while (pool->isRunning_)
	{
		Task* task = pool->take();
		if (!task)
		{
			printf("thread %lu will exit\n", std::this_thread::get_id());
			break;
		}

		assert(task);
		task->run(id);
	}
	printf("thread %lu exited\n", std::this_thread::get_id());
	return 0;
}

ThreadPool::ThreadPool(int threadNum /*= 4*/)
{
	threadNum = threadNum < 1 ? 1 : threadNum;
	isRunning_ = true;
	threadsNum_ = threadNum;
	for (size_t i = 0; i < threadNum; ++i)
	{
		pool.emplace_back(&ThreadPool::threadFunc, this, i);
	}
}

ThreadPool::~ThreadPool()
{
	stop();

	for (std::deque<Task*>::iterator it = taskQueue_.begin(); it != taskQueue_.end(); ++it)
	{
		delete *it;
	}

	taskQueue_.clear();
}

size_t ThreadPool::addTask(Task *task)
{
	mutex_.lock();
	taskQueue_.push_back(task);
	int size = taskQueue_.size();
	mutex_.unlock();

	condition_.notify_all();
	return size;
}

void ThreadPool::stop()
{
	if (!isRunning_)
		return;
	isRunning_ = false;
	condition_.notify_all();


	for (int i = 0; i < threadsNum_; ++i)
	{
		//if (pool[i].joinable())
		pool[i].join();
		// printf("joining...\n");
	}



}

int ThreadPool::size()
{
	mutex_.lock();
	int size = taskQueue_.size();
	mutex_.unlock();
	return size;
}

Task* ThreadPool::take()
{

	Task* task = NULL;
	std::unique_lock<std::mutex> lock(mutex_);

	condition_.wait(lock, [this]()
	{
		return (!taskQueue_.empty() || !isRunning_);
	}
	);
	if (!isRunning_)
	{
		return task;
	}
	assert(!taskQueue_.empty());
	task = taskQueue_.front();
	taskQueue_.pop_front();
	return task;

}

void ThreadPool::cancel()
{
  mutex_.lock();
  taskQueue_.clear();
  mutex_.unlock();
  isRunning_ = false;
  condition_.notify_all();


  for (int i = 0; i < threadsNum_; ++i)
  {
    //if (pool[i].joinable())
    pool[i].join();
    // printf("joining...\n");
  }
}



