#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <string.h>
#include <regex>

std::vector<std::mutex *> zamky; /* pole zamku promenne velikosti */

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


void worker_thread(int ID, char *RE1, char *RE2) {
	/* funkce implementujici thread */
	printf("===thread %d===\nREG1 %s\nREG2 %s\n\n", ID, RE1, RE2);
	sleep(2);
}

int main(int argc, char **argv) {

	if (argc < 3 || !((argc -1 ) % 2 == 0)) {
		printf("Invalid arguments\n");
		return EXIT_FAILURE;
	}

	int thread_count = (argc - 1) / 2;
	int lock_count = thread_count - 1 ;
	std::vector <std::thread *> threads; /* pole threadu promenne velikosti */

	/* vytvorime zamky */
	zamky.resize(lock_count); /* nastavime si velikost pole zamky */
	for (int i = 0; i < lock_count; i++) {
		std::mutex *new_lock = new std::mutex();
		zamky[i] = new_lock;
	}

	/* vytvorime thready */
	threads.resize(thread_count); /* nastavime si velikost pole threads */
	for(int i = 0; i < thread_count; i++){
		std::thread *new_thread = new std::thread(worker_thread, i, argv[2 * i + 1], argv[2 * i + 2]);
		threads[i] = new_thread;
	}
	/**********************************
	 * Vlastni vypocet psed
	 * ********************************/
	int res;
	line = read_line(&res);
	while (res) {
		printf("%s\n",line);
		free(line); /* uvolnim pamet */
		line = read_line(&res);
	}

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
