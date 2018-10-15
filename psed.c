#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <string.h>

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
		str=to_cstr(line);
		*res=1;
		return str;
	} else {
		*res=0;
		return NULL;
	}

}


void worker_thread(int ID) {
	/* funkce implementujici thread */
	printf("Thread %i started\n",ID);
}

int main() {
	/*******************************
	 * Inicializace threadu a zamku
	 * *****************************/
	int num = 10;
	int num_zamky = 15;
	std::vector <std::thread *> threads; /* pole threadu promenne velikosti */

	/* vytvorime zamky */
	zamky.resize(num_zamky); /* nastavime si velikost pole zamky */
	for(int i = 0; i < num_zamky; i++){
		std::mutex *new_zamek = new std::mutex();
		zamky[i] = new_zamek;
	}

	/* vytvorime thready */
	threads.resize(num); /* nastavime si velikost pole threads */
	for(int i = 0; i < num; i++){
		std::thread *new_thread = new std::thread(worker_thread, i);
		threads[i]=new_thread;
	}
	/**********************************
	 * Vlastni vypocet psed
	 * ********************************/
	int res;
	line=read_line(&res);
	while (res) {
		printf("%s\n",line);
		free(line); /* uvolnim pamet */
		line = read_line(&res);
	}


	/**********************************
	 * Uvolneni pameti
	 * ********************************/

	/* provedeme join a uvolnime pamet threads */
	for(int i=0; i<num; i++){
		(*(threads[i])).join();
		delete threads[i];
	}
	/* uvolnime pamet zamku */
	for(int i=0;i<num_zamky;i++){
		delete zamky[i];
	}

}
