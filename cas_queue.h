#ifndef CAS_QUEUE_H
#define CAS_QUEUE_H

#include <atomic>
#include <stdint.h>

/*
 * This queue is implemented with c++11 double-word-compare-and-swap(DWCAS) API.
 *
 * Node: for clang and gcc, '-mcx16' should be included in order to generate the x86-64 DWCAS instruction.
 */

//TODO: use union to enable access each element of Pointer more efficiently.
template <typename T>
struct Node;  			// declare at the beginning.

template <typename T>
struct Pointer {
	Node<T> *ptr;
	uintptr_t count; // use pointer-sized intergers to avoid padding.
};

template <typename T>
struct Node{
	T value;
	std::atomic<Pointer<T>> next;
};

template<typename T>
static bool operator==(const Pointer<T>& p1, const Pointer<T>& p2) {
	return (p1.ptr == p2.ptr) && (p1.count == p2.count);
}

template <typename T>
class CAS_Queue
{
private:
	std::atomic<Pointer<T>> _q_head;
	std::atomic<Pointer<T>> _q_tail;
public:
	
	CAS_Queue() {
		Node<T> *node = new Node<T> ();
		//node->next.load().ptr = nullptr; 	// not assignable, as load() return a rvalue which only can be read.
		//node->next.load().count = 0;
		Pointer<T> p;
		p.ptr = nullptr;
		p.count = 0;
		node->next.store(p);

		_q_head = _q_tail = {node, 0};

		assert(std::atomic<Pointer<T>>{}.is_lock_free() && "atomic<Pointer<T> is not lock free!\n");
	}

	void enqueue(T value)
	{
		Pointer<T> local_tail;	// owned by only one thread.
		Pointer<T> local_next;	// owned by only one thread.
		Pointer<T> desired; 	// for the desired in CAS.

		Node<T> *node = new Node<T> ();
		node->value = value;
		Pointer<T> p;
		p.ptr = nullptr;
		p.count = 0;
		node->next.store(p);

		while(1) 
		{
			local_tail = _q_tail.load();
			//local_next = _q_tail.load().ptr->next.load();
			local_next = local_tail.ptr->next;
			if (local_tail == _q_tail.load()) 
			{
				if (local_next.ptr == NULL)
				{
					desired = {node, local_next.count+1};
					if (std::atomic_compare_exchange_weak( &(local_tail.ptr->next), &local_next, desired))
					{
						break;
					}
				} 
				else 
				{
					desired = {local_next.ptr, local_tail.count+1};
					std::atomic_compare_exchange_weak(&_q_tail, &local_tail, desired);
				}
			}
		}
		desired = {node, local_tail.count+1};
		std::atomic_compare_exchange_weak(&_q_tail, &local_tail, desired); 
	};

	bool dequeue(T *pvalue)
	{
		Pointer<T> local_head;
	 	Pointer<T> local_tail;   
	 	Pointer<T> next;
		Pointer<T> desired;

		while(1) 
	 	{
			local_head = _q_head.load();
			local_tail = _q_tail.load();
			next = local_head.ptr->next.load();

			if (local_head == _q_head.load()) 
			{
				if (local_head.ptr == local_tail.ptr) 
				{
					if (next.ptr == NULL) 
					{
						return false;
					}
					desired = {next.ptr, local_tail.count+1};
					std::atomic_compare_exchange_weak(&_q_tail, &local_tail, desired);
				} 
				else 
				{
					*pvalue = next.ptr->value;
					desired = {next.ptr, local_head.count+1};
					if( std::atomic_compare_exchange_weak(&_q_head, &local_head, desired) )
					{
						break;
					}
				}
			}
		}
		delete local_head.ptr;
		return true;
	};
};

#endif
