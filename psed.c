/* David Sedlák <xsedla1d>, Karel Hanák <xhanak34> */
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <string.h>
#include <regex>

using namespace std;

vector <mutex *> locks;
mutex *entered_chain_mutex, *finished_mutex;

bool line_ready = false, line_processed = false, thread_terminate = false;
int in_chain = 0, finished = 0;
char *line;

/* converts c++ string to null terminated array of chars (char *)*/
char *to_cstr(string a) {
	char *str;
	str = (char *)malloc(sizeof(char) * (a.length() + 1));
	strcpy(str, a.c_str());
	return str;
}

/* reads one line from input */
char *read_line(int *res) {
	string line;
	char *str;
	if (getline(cin, line)) {
		str = to_cstr(line);
		*res = 1;
		return str;
	} else {
		*res = 0;
		return NULL;
	}
}

/* implementation of worker thread */
void worker_thread(int ID, char *RE, char *REPL, int thread_count) {
	char *buf;
	int iteration = 0;
	while (1) {
		/* lock mutex for next thread in chain, no one is waiting for last thread */
		if (ID != (thread_count - 1)) {
			locks[ID]->lock();
		}

		/* thread ID is now part of the chain, increment shared counter of threads in chain */
		entered_chain_mutex->lock();
		in_chain++;
		entered_chain_mutex->unlock();

		while (!line_ready) {
			if (thread_terminate) {
				return;
			}
		}

		/* make sure all threads are in the chain before entering the stage of unchaining */
		while(in_chain != thread_count);

		/**
		 * Wait for output of predecesing thread
		 * Thread with id 0 doesn't have to wait for anything
		 */
		if (!ID == 0) {
			locks[ID - 1]->lock();
		}

		/* apply regex */
		regex re1(RE);
		string res = regex_replace(line, re1, REPL);
		char *str = to_cstr(res);
		/* print the output */
		printf("%s\n", str);
		free(str);

		if (ID != (thread_count - 1)) {
			locks[ID]->unlock();
		} else {
		/**
		 * last thread must set information about completing the line
		 * and set default value of shared variables
		 */
			in_chain = 0;
			line_ready = false;
			line_processed = true;
		}

		/* unlock mutex of predeceasing thread to be ready for next line */
		if (ID != 0) {
			locks[ID - 1]->unlock();
		}

		/* threads wait until the line is processed */
		while(!line_processed);

		finished_mutex->lock();
		finished++;
		finished_mutex->unlock();
	}
}

int main(int argc, char **argv) {

	if (argc < 3 || !((argc -1 ) % 2 == 0)) {
		printf("Invalid arguments\n");
		return EXIT_FAILURE;
	}

	/* turn of buffering */
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

	int thread_count = (argc - 1) / 2;
	int lock_count = thread_count - 1;
	/* array of threads */
	vector <thread *> threads;

	/* try to create locks and threads */
	try {
		entered_chain_mutex = new mutex();
		finished_mutex = new mutex();

		locks.resize(lock_count);

		for (int i = 0; i < lock_count; i++) {
			mutex *new_zamek = new mutex();
			locks[i] = new_zamek;
		}

		threads.resize(thread_count);
		for(int i = 0; i < thread_count; i++){
			thread *new_thread = new thread(worker_thread, i, argv[2 * i + 1], argv[2 * i + 2], thread_count);
			threads[i] = new_thread;
		}

	} catch (bad_alloc&) {
		fprintf(stderr, "Unable to allocate resources\n");
		return(EXIT_FAILURE);
	}

	int res;
	line = read_line(&res);
	if (res) {
		line_ready = true;
	}
	int processed = 0;
	while (res) {
		while(!line_processed);
		free(line);
	 	line = read_line(&res);

		while (finished != thread_count);
		finished = 0;
		line_processed = false;

		if (res) {
			line_ready = true;
		}
	}
	thread_terminate = true;

	/* join and free threads */
	for(int i = 0; i < thread_count; i++){
		(*(threads[i])).join();
		delete threads[i];
	}

	/* free locks */
	delete entered_chain_mutex;
	delete finished_mutex;
	for(int i = 0;i < lock_count; i++){
		delete locks[i];
	}

	return EXIT_SUCCESS;
}
