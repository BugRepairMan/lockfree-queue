#ifndef QUEUE_H
#define QUEUE_H

/*
 * This queue is implemented with mutex_lock and condional variables.
 */

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue
{
public:
	Queue(unsigned int c):capacity_(c) {}
	Queue(const Queue&) = delete;	//disable copying
	Queue& operator=(const Queue&) = delete; // disable assignment

	T pop()
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		while(queue_.empty()) {
			cond_empty.wait(mlock);
		}

		auto item = queue_.front();
		queue_.pop();
		mlock.unlock();
		cond_full.notify_all();
		return item;
	}

	void push(const T& item)
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		while(queue_.size() == capacity_) {
			cond_full.wait(mlock);
		}

		queue_.push(item);
		mlock.unlock();
		cond_empty.notify_all();
	}

	size_t size()
	{
		return queue_.size();
	}
private:
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_empty;
	std::condition_variable cond_full;
	unsigned int capacity_;
};

#endif
