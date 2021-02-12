#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX_STRING_LENGTH 1024

// list node
typedef struct node {
	char *word;
	int size;
    int freq;
    struct node *next;
} node_t;

// prints the list is ascending order
void printList( node_t **head) {
	if ( *head == NULL)
		printf( "List is empty.\n");
	else
		printf( "List:\n");
	
	node_t *curr = *head;
	while ( curr != NULL) {
		printf( "%s(%d) %d\n", curr->word, curr->size ,curr->freq);
		curr = curr->next;
	}
}

// inserts the word in an ascending order
void insert( node_t **head, char* str, int size) {
	if ( *head == NULL || strcmp( str, (*head)->word) < 0) {
    	node_t *newNode = (node_t *) malloc( sizeof(node_t));
    	newNode->next = *head;
    	newNode->freq = 1;
    	newNode->size = size;
    	newNode->word = (char *) malloc( size + 1);
    	strcpy( newNode->word, str);
    	*head = newNode;
	}
    else {
    	node_t *prev = *head, *curr = prev;

    	while ( curr != NULL) {
	    	if ( strcmp( str, curr->word) == 0) {
		    	curr->freq = (curr->freq) + 1;
		    	break;
	    	}
	    	else if ( strcmp( str, curr->word) < 0) {
    			node_t *newNode = (node_t *) malloc( sizeof(node_t));
    			newNode->next = curr;
    			newNode->freq = 1;
    			newNode->size = size;
    			newNode->word = (char *) malloc( size + 1);
    			strcpy( newNode->word, str);
    			prev->next = newNode;
    			break;
	    	}
	    	else if ( curr->next == NULL) {
	    		node_t *newNode = (node_t *) malloc( sizeof(node_t));
	    		newNode->next = NULL;
	    		newNode->freq = 1;
    			newNode->size = size;
    			newNode->word = (char *) malloc( size + 1);
	    		strcpy( newNode->word, str);
	    		curr->next = newNode;
	    		break;
	    	}
	    	prev = curr;
	    	curr = curr->next;
    	}
    }
}

// returns the first element and removes it from the list, returns 0 if list is empty
int pop( node_t **head, char** str) {
    int repeat = 0;
    if ( *head != NULL) {
        node_t *current = *head;
        *head = current->next;

        repeat = current->freq;
        *str = (char *) malloc( current->size + 1);
        strcpy( *str, current->word);

        current->next = NULL;
        if ( current->word)
        	free( current->word);
        free(current);
    }
    return repeat;
}

void truncateList( node_t **head) {
	node_t * curr = *head;
	while ( (*head) != NULL) {
		curr = *head;
		*head = curr->next;
		free( curr->word);
		free( curr);
	}
}

long ms( struct timeval t)
{
    return t.tv_sec * 1000000 + t.tv_usec;
}

void *workerThread(void *ars)
{
	char *fname = (char *) ars;
	node_t *head = NULL;

	// open input file
	FILE *fp = fopen( fname, "r");
	char *word = (char *) malloc( MAX_STRING_LENGTH);
	if ( fp != NULL) {
		int count = 0;
		// read file word by word and insert them to list
		while ( fscanf( fp, "%s", word) != EOF) {
			int len = 0;
			for ( int j = 0; j < MAX_STRING_LENGTH; j++) {
				if ( word[j] == NULL)
					break;
				len++;
			}
			insert( &head, word, len);
			count++;
		}
		fclose( fp);
		printf( "Input file \"%s\" has %d words.\n", fname, count);
	}
	else {
		printf( "Unable to open input file \"%s\".\n", fname);
		exit(1);
	}
	free( word);

	// return pointer to the head of list
	return (void *) head;
}

int main( int argc, char* argv[])
{
    struct timeval t1, t2;
    long bFTime;
    gettimeofday( &t1, NULL);

	int N = atoi(argv[1]);
	if ( N == 0) {
		printf( "There is nothing to work with.\n");
		return 1;
	}
	else if ( argc == N+2) {
		printf( "Missing output file.\n");
		return 1;
	}
	else if ( argc > N+3) {
		printf( "Extra arguments are given.\n");
		return 1;
	}
	else if ( argc < N+2) {
		printf( "Missing input file/s.\n");
		return 1;
	}

	// create linked lists
	node_t **lists = (node_t **) malloc( N * sizeof( node_t *));

	// create threads
	pthread_t *tids = (pthread_t *) malloc( N * sizeof(pthread_t));

	for ( int i = 0; i < N; i++) {
		pthread_create( &tids[i], NULL, workerThread, (void *) argv[i+2]);
	}


	// terminate threads
	for ( int i = 0; i < N; i++) {
		void *bufptr;
		pthread_join( tids[i], &bufptr);
		lists[i] = (node_t *) bufptr;
	}
	free( tids);

	// current words and their properties
	char **words = (char **) malloc( N * sizeof( char *));
	int *reps = (int *) malloc( N * sizeof( int));
	int *qstat = (int *) malloc( N * sizeof( int));
	for ( int i = 0; i < N; i++) {
		qstat[i] = 1;
		reps[i] = 0;
	}

	// open output file
	FILE *fp = fopen( argv[N+2], "w");
	if ( fp == NULL) {
		printf( "Unable to open output file.\n");
		for ( int i = 0; i < N; i++) {
			truncateList( &lists[i]);
		}
		free( lists);
		free( words);
		free( reps);
		free( qstat);
		return 1;
	}

	while ( 1) {
		// get first items from lists
		for ( int i = 0; i < N; i++) {
			if ( qstat[i] && reps[i] == 0) {
				reps[i] = pop( &lists[i], &words[i]);

				if ( reps[i] == 0)
					qstat[i] = 0;
			}
		}

		// compare items
		int first = 0;
		for ( int i = 0; i < N; i++) {
			if ( qstat[i]) {
				first = i;
				break;
			}
		}
		for ( int i = first+1; i < N; i++) {
			if ( qstat[i]) {

				if ( strcmp( words[first], words[i]) > 0) {
					first = i;
				}
				else if ( strcmp( words[first], words[i]) == 0) {
					reps[first] = reps[first] + reps[i];
					reps[i] = 0;
					free( words[i]);
				}
			}
		}

		// write first item in ascending order into file
		if ( qstat[first]) {
			fprintf( fp, "%s %d\n", words[first], reps[first]);
			reps[first] = 0;
			free(words[first]);
		}

		// if all lists are empty break loop
		int br = 1;
		for ( int i = 0; i < N; i++) {
			if ( qstat[i])
				br = 0;
		}
		if ( br) {
			break;
		}
	}
	free( words);
	free( reps);
	free( qstat);
	free( lists);

	fclose( fp);
	printf( "Results were written into \"%s\".\n", argv[N+2]);

	gettimeofday( &t2, NULL);
    bFTime = (ms(t2) - ms(t1));
    printf( "Completed in %ld μs.\n", bFTime);

	return 0;
}
