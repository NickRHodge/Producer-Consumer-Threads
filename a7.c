/*
	Name: Nick Hodge(Hodge1nr)		Class: CPS470
	Section: 22377311			Assignment: 07
	Due: 4/22/20				Points: 10

	Description: Uses threads and semaphores to implement a bounded buffer program.
		Multiple producer and consumer threads will be created and synchronized using
		binary and counting semaphores (4 in total). The producer thread deposits a random 
		character of the alphabet (A-Z) into the buffer. The consumer thread will remove an
		item from the buffer. Both threads will print the results of their deposit or removal. 

	Constraints: A producer cannot deposit items into a full buffer, and the consumer cannot remove
		items from an empty buffer. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define  MXTHRDS 380
#define BFSIZE 301
typedef struct queue {
	char data[BFSIZE];
	int front, rear;
	sem_t emptyslots, fullslots, placeitem, removeitem;
} BUFFER;
BUFFER buffer;

pthread_t *p, *c;

int chkargs(int argc, char *argv[], int *prods, int *proditers, int *consmrs, int *criters);
void *consumer(void *arg);
void destroy();
int deq(char *item);
void die(char *why);
int enq(char item);
int init();
void *producer(void *arg);
char randchar();
void usage(char *progname);


/*	Main function
*/
int main(int argc, char *argv[]) {

	int nrprods;
	int proditers;
	int nrcons;
	int consiters;
	int check;
	int initcheck;
	int i;

	if (argc != 5) {
		usage(argv[0]);
	}

	srand(time(NULL));
	
	nrprods = atoi(argv[1]);
	proditers = atoi(argv[2]);
	nrcons = atoi(argv[3]);
	consiters = atoi(argv[4]);
	
	check = chkargs(argc, argv, &nrprods, &proditers, &nrcons, &consiters);

	if (check == 1) {
		initcheck = init();
		if (initcheck == 0) {
			die("Buffer/Semaphore initialization failed");
		}
		else {
			p = malloc(nrprods * sizeof(pthread_t));
			c = malloc(nrcons * sizeof(pthread_t));

			for (i = 0; i < nrprods; i++) {
				pthread_create(&p[i], NULL, producer, &proditers);
			}
		
			for (i = 0; i < nrcons; i++) {
				pthread_create(&c[i], NULL, consumer, &consiters);
			}

			for (i = 0; i < nrprods; i++) {
				pthread_join(p[i], NULL);
			}

			for(i = 0; i < nrcons; i++) {
				pthread_join(c[i], NULL);
			}

			destroy();
			free(p);
			free(c);
		}
	}
	if (check == 0) {
		die("Check input");
	}
	exit(0);
}


/*	Prints usage to stderr and terminates execution of program 
	nrprods - number of producers
	nrcons - number of consumers
	proditers - number of iterations done by a producer
	consiters - number of iterations done by a consumer 
*/
void usage(char *progname) {

	fprintf(stderr, "usage: %s <nrprods> <proditers> <nrcons> <consiters>\n", progname);
	exit(1);

}


/*	Checks for proper number, nature and constraints in command line arguments
	Must be true:
		argc = 5
		prods * proditers = consmrs * criters
		prods + consmrs <= 380
		prods and consmrs > 0
*/
int chkargs(int argc, char *argv[], int *prods, int *proditers, int *consmrs, int *criters) {

	if (argc != 5) {
		usage(argv[0]);
	}
	else {
		if ((*proditers * *prods) != (*criters * *consmrs)) {
			return 0;
		}
		if (!((*prods + *consmrs) <= 380)) {
			return 0;
		}
		if (*prods > 0 && *consmrs > 0) {
			return 1;
		}
		else {
			return 0;
		}
	}
	return 1;
}


/*	Executed by producer thread, inserts a random char (created by randchar()) into the buffer
*/ 
void *producer(void *arg) {

	int i;
	int index;
	int iter = *(int*)arg;

	for (i = 0; i < iter; i++) {
		char rand = randchar();

		sem_wait(&buffer.emptyslots);
		sem_wait(&buffer.removeitem);

		index = enq(rand);

		printf("%lu: produced: %c, placed in buffer: %d\n", pthread_self(), rand, index);

		sem_post(&buffer.removeitem);
		sem_post(&buffer.fullslots); 
	}

	pthread_exit(NULL);
}


/*	Executed by consumer thread, removes a char from the buffer
*/
void *consumer(void *arg) {

	int i;
	int index;
	int iter = *(int*)arg;

	for (i = 0; i < iter; i++) {
	
		sem_wait(&buffer.fullslots);
		sem_wait(&buffer.placeitem);

		char ret;
		index = deq(&ret);
		
		printf("%lu: consumed: %c, removed from buffer: %d\n", pthread_self(), ret, index);

		sem_post(&buffer.placeitem);
		sem_post(&buffer.emptyslots);
	}

	pthread_exit(NULL);
}


/*	Generates a random character between 'A' and 'Z'.
	Uses rand() to generate value from 0 to 25, which is used to pick random character.
*/
char randchar() {

	char randchar;

	randchar = (rand() % 26) + 'A';	

	return randchar;
}


/*	Prints the reaon for dying and die
*/
void die(char *why) {

	fprintf(stderr, "Error: %s\n", why);
	exit(1);
}


/*	Places given item at rear in buffer, returns the index the item was inserted at 
*/
int enq(char item) {

	int i;

	i = buffer.front;
	buffer.data[buffer.front] = item;
	buffer.front = (buffer.front + 1) % BFSIZE;
	
	return i;
}


/*	Removes a char from front in buffer, returns the index the item was removed from
*/
int deq(char *item) {

	int i;

	i = buffer.rear;
	*item = buffer.data[buffer.rear];
	buffer.data[buffer.rear] = '\0';
	buffer.rear = (buffer.rear + 1) % BFSIZE;

	return i;
}


/*	Initializes the semaphores, buffer, and rear/front indexes
	Returns 1 on success, 0 otherwise
*/
int init() {

	int i;
	int ret;

	for (i = 0; i < BFSIZE; i++) {
		buffer.data[i] = '\0';
	}

	buffer.rear = 0;
	buffer.front = 0;

	ret = sem_init(&buffer.emptyslots, 0, BFSIZE);
	ret = sem_init(&buffer.fullslots, 0, 0);
	ret = sem_init(&buffer.placeitem, 0, 1);
	ret = sem_init(&buffer.removeitem, 0, 1);

	if (ret == 0) {
		return 1;
	}
	else {
		return 0;
	}
}


/*	Releases resources associates with semaphores
*/
void destroy() {

	sem_destroy(&buffer.fullslots);
	sem_destroy(&buffer.emptyslots);
	sem_destroy(&buffer.placeitem);
	sem_destroy(&buffer.removeitem);

}


