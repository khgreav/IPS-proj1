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

//using namespace std;

std::vector<std::mutex *> zamky; /* pole zamku promenne velikosti */
std::mutex *entered_chain_mutex;

bool line_ready = false, line_processed = false, terminate = false;
int in_chain = 0;
char *line;

char *to_cstr(std::string a) {
	// prevede retezec v c++ do retezce v "c" (char *)
	char *str;
	str = (char *)malloc(sizeof(char) * (a.length() + 1));
	strcpy(str, a.c_str());
	return str;
}

char *read_line(int *res) {
	std::string line;
	char *str;
	if (std::getline(std::cin, line)) {
		str = to_cstr(line);
		*res = 1;
		return str;
	} else {
		*res = 0;
		return NULL;
	}

}

void worker_thread(int ID, char *RE, char *REPL, int thread_count) {
	char *buf;
	while (1) {
		/* lock mutex for next thread in chain, no one is waiting for last thread */
		if (ID != (thread_count - 1)) {
			zamky[ID]->lock();
		}

		/* thread ID is now part of the chain, increment shared counter of threads in chain */
		entered_chain_mutex->lock();
		in_chain++;
		entered_chain_mutex->unlock();

		while (!line_ready) {
			if (terminate) {
				exit(EXIT_SUCCESS);
			}
		}

		//std::regex_replace(line,RE,REPL)
		//malloc(sizeof(*buf) * strlen(line) + 1);
		//strcpy(buf, line);
		//std::regex re1(RE);
		//std::string res = std::regex_replace((std::vector<char *>)*buf, re1, REPL);

		/* make sure all threads are in the chain before entering the stage of unchaining */
		while(in_chain != thread_count);

		/* unchain */
		/* Thread with id 0 doesn't have to wait for anything */
		if (!ID == 0) {
			zamky[ID - 1]->lock();
		}

		/* print the output and unlock next thread in chain */
		printf("%d - %s\n", ID, line);

		if (ID != (thread_count - 1)) {
			zamky[ID]->unlock();
		}
		/* unlock mutex of predeceasing thread to be ready for next line */
		if (ID != 0) {
			zamky[ID - 1]->unlock();
		}

		if (ID != (thread_count - 1)) {
			zamky[ID]->unlock();
		} else {
		/* last thread must set information about completing the line */
			line_ready = false;
			line_processed = true;
			in_chain = 0;
		}

		/* threads wait until the line is processed */
		while(!line_processed);
	}
}

int main(int argc, char **argv) {

	if (argc < 3 || !((argc -1 ) % 2 == 0)) {
		printf("Invalid arguments\n");
		return EXIT_FAILURE;
	}

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

	int thread_count = (argc - 1) / 2;
	int lock_count = thread_count - 1;
	std::vector <std::thread *> threads; /* pole threadu promenne velikosti */

	/* vytvorime zamky */
	zamky.resize(lock_count); /* nastavime si velikost pole zamky */
	entered_chain_mutex = new std::mutex();
	for (int i = 0; i < lock_count; i++) {
		std::mutex *new_zamek = new std::mutex();
		zamky[i] = new_zamek;
	}

	/* vytvorime thready */
	threads.resize(thread_count); /* nastavime si velikost pole threads */
	for(int i = 0; i < thread_count; i++){
		std::thread *new_thread = new std::thread(worker_thread, i, argv[2 * i + 1], argv[2 * i + 2], thread_count);
		threads[i] = new_thread;
	}

	int res;
	line = read_line(&res);
	line_ready = true;
	while (res) {
		while(!line_processed);
		free(line); /* uvolnim pamet */
	 	line = read_line(&res);
		line_processed = false;
		line_ready = true;
	}
	terminate = true;

	/* provedeme join a uvolnime pamet threads */
	for(int i = 0; i < thread_count; i++){
		(*(threads[i])).join();
		delete threads[i];
	}
	/* uvolnime pamet zamku */
	for(int i = 0;i < lock_count; i++){
		delete zamky[i];
	}

	return EXIT_SUCCESS;
}
