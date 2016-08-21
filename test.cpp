#include <cassert>
#include <iostream>
#include <thread>
#include "queue.hpp"
#include <chrono>
#include "cas_queue.h"

#define N_ELEM 1000000 // total mount of item for all producers

std::vector<int> dequeued(N_ELEM);
int n_consume = 0;

void oneProducerMultiConcumers(int n) {
	std::cout << "One P multi C\n";

	Queue<int> q(100);
	n_consume = n;
	int step = N_ELEM/n_consume;
	
	auto producer = std::thread([&]() {
			for(int i = 0; i < N_ELEM; ++i) {
				q.push(i);
			}
	});

	std::vector<std::thread> consumers(n_consume);
	for(int i = 0; i < n_consume; ++i) {
		consumers[i] = std::thread([&]() {
				for(int j = 0; j < step; ++j) {
					auto item = q.pop();
					dequeued[item] += 1;
					}
				});
	}
	
	producer.join();
	for(int i = 0; i < n_consume; ++i) {
		consumers[i].join();
	}

	std::cout << "Leftovers: " << q.size() << "\n";

	// Make sure everything went in and came back out!
	for(int i = 0; i < N_ELEM; ++i) {
		if(dequeued[i] != 1) {
			std::cout << "Fail: index=" << i <<" value: " << dequeued[i] << "\n";
			//return;
		}
	}
	
	std::cout << "Success!\n";
}

void multiPmultiC(int n_threads) {
	std::cout << n_threads << " Producer " << n_threads << " Consumer\n";

	Queue<int> q(100);
	//int dequeued[N_ELEM] = {0};
	std::vector<std::thread> threads(n_threads * 2);
	int step = N_ELEM/n_threads;

	clock_t startTime = clock();
	auto start = std::chrono::high_resolution_clock::now();

	// Producers
	for (int i = 0; i < n_threads; ++i) {
		threads[i] = std::thread([&](int i) {
				for (int j = 0; j < step; ++j) {
				q.push( i*step+j ); // range: 0--110
			}
		},i);
	}
	
	// Consumers
	for (int i = n_threads; i < n_threads*2; ++i) {
		threads[i] = std::thread([&]() {
			//int item;
			for (int j = 0; j < step; ++j) {
				auto item = q.pop();
				dequeued[item] += 1;
			}
		});
	}

	// Wait for all threads
	for (int i = 0; i < n_threads*2; ++i) {
		threads[i].join();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end -start;
	std::cout << "Chrono time: " << diff.count() << "s\n";

	clock_t stopTime = clock();
	float elapsed = (float)(stopTime - startTime)/CLOCKS_PER_SEC;
	std::cout << "Time: " << elapsed << "\n";

	std::cout << "Leftovers: " << q.size() << "\n";

	// Make sure everything went in and came back out!
	for(int i = 0; i < N_ELEM; ++i) {
		if(dequeued[i] != 1) {
			std::cout << "Fail: index=" << i <<" value: " << dequeued[i] << "\n";
			return;
		}
	}
	
	std::cout << "Success!\n";
}

void cas_multiPmultiC(int n_threads) {
	std::cout << n_threads << " Producer " << n_threads << " Consumer\n";
	CAS_Queue<int> q;
	std::vector<std::thread> threads(n_threads * 2);
	int step = N_ELEM/n_threads;

	auto start = std::chrono::high_resolution_clock::now();

	// Producers
	for(int i = 0; i < n_threads; ++i) {
		threads[i] = std::thread([&](int i) {
				for (int j = 0; j < step; ++j) {
					q.enqueue( i*step+j );
				}
			}, i);
	}

	// Consumers
	for (int i = n_threads; i < n_threads*2; ++i) {
		threads[i] = std::thread([&]() {
			int item;
			for(int j = 0; j < step; ++j) {
				while(!q.dequeue(&item))	;
				dequeued[item] += 1;
			}
		});
	}

	for (int i = 0; i < n_threads*2; ++i) {
		threads[i].join();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end -start;
	std::cout << "Chrono time: " << diff.count() << "s\n";

	// Make sure everything went in and came back out!
	for(int i = 0; i < N_ELEM; ++i) {
		if(dequeued[i] != 1) {
			std::cout << "Fail: index=" << i <<" value: " << dequeued[i] << "\n";
			return;
		}
	}
	
	std::cout << "Success!\n";
}

void cas_onePmultiC(int n_threads) {
	std::cout << "1 Producer " << n_threads << " Consumer\n";
	CAS_Queue<int> q;
	std::vector<std::thread> consumers(n_threads);
	int step = N_ELEM/n_threads;

	auto start = std::chrono::high_resolution_clock::now();

	// Producers
	auto producer = std::thread([&]() {
				for (int i = 0; i < N_ELEM; ++i) {
					q.enqueue(i);
				}
			});

	// Consumers
	for (int i = 0; i < n_threads; ++i) {
		consumers[i] = std::thread([&]() {
			int item;
			for(int j = 0; j < step; ++j) {	// Each consumer takes step items from the queue.
				while(!q.dequeue(&item))	;
				dequeued[item] += 1;	// As each item exists only once, so dequeued is not needed to guarded.
			}
		} );
	}

	producer.join();
	for (int i = 0; i < n_threads; ++i) {
		consumers[i].join();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end -start;
	std::cout << "Chrono time: " << diff.count() << "s\n";

	// Make sure everything went in and came back out!
	for(int i = 0; i < N_ELEM; ++i) {
		if(dequeued[i] != 1) {
			std::cout << "Fail: index=" << i <<" value: " << dequeued[i] << "\n";
			return;
		}
	}
	
	std::cout << "Success!\n";
}

int main(int argc, char** argv)
{
	int n_threads = 0;
	if(argc == 2) {
		n_threads = atoi(argv[1]);
		std::cout << "Threads number:" << n_threads << "\n";
	} else {
		n_threads = 2;
		std::cout << "Threads number: 2\n";
	}
	
	//multiPmultiC(n_threads);
	
	cas_multiPmultiC(n_threads);
	//cas_onePmultiC(n_threads);

	//oneProducerMultiConcumers(n_threads);

	//onePmultiC(n_threads);
		
	return 0;
}
