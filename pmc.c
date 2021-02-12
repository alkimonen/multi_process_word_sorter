#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>

#define MAX_STRING_LENGTH 1024
#define QUEUE_BASE "/message_queue_"

// list node
typedef struct node {
	char *word;
	int size;
    int freq;
    struct node *next;
} node_t;

typedef struct item {
	char word[MAX_STRING_LENGTH];
	int size;
	int freq;
} item_t;

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
int pop( node_t **head, char** str, int *size) {
    int repeat = 0;
    if ( *head != NULL) {
        node_t *current = *head;
        *head = current->next;

        repeat = current->freq;
        *size = current->size;
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

	mqd_t *qs = (mqd_t *) malloc( N * sizeof( mqd_t));
	char **qnames = (char **) malloc( N * sizeof(char *));

	for ( int i = 0; i < N; i++) {
		// generate queue name
		char no[N];
		sprintf( no, "%d", i);
		qnames[i] = (char *) malloc( sizeof(QUEUE_BASE) + 1);
		strcpy( qnames[i], QUEUE_BASE);
		strcat( qnames[i], no);

		// open queue
		qs[i] = mq_open( qnames[i], O_RDWR | O_CREAT, 0666, NULL);
		if ( qs[i] == -1)
			printf( "Message queue creation (%d) failed.\n", i+1);
	}

	// create child processes
	for ( int i = 0; i < N; i++) {
		if ( fork() == 0) {
			node_t *head = NULL;
			char *word = (char *) malloc( MAX_STRING_LENGTH);

			FILE *fp = fopen( argv[i+2], "r");
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
				printf( "Input file \"%s\" has %d words.\n", argv[i+2], count);

				free( word);

				// pop item from list and send it to message queue in ascending order
				item_t *msg = (item_t *) malloc( sizeof( item_t));
				memset( msg, 0, sizeof( item_t));

				int size = 0, repeat = 1;

				while ( repeat > 0) {
					repeat = pop( &head, &word, &size);

					if ( repeat > 0) {
						strcpy( msg->word, word);
						msg->size = size;
						free( word);
					}
					msg->freq = repeat;

					mq_send( qs[i], (char *) msg, sizeof( item_t), 0);
				}
				free( msg);
			}
			else {
				printf( "Unable to open input file \"%s\".\n", argv[i+2]);

				// send an empty node to parent
				item_t *msg = (item_t *) malloc( sizeof( item_t));
				memset( msg, 0, sizeof( item_t));

				msg->freq = 0;
				mq_send( qs[i], (char *) msg, sizeof( item_t), 0);
				free( msg);
			}

			mq_close( qs[i]);
			for ( int i = 0; i < N; i++) {
				if ( qnames[i])
					free( qnames[i]);
			}
			free( qnames);
			free( qs);
			return 0;
		}
	}

	// parent process

	// open output file
	FILE *fp = fopen( argv[N+2], "w");
	if ( fp == NULL) {
		printf( "Unable to open output file.\n");
	}
	else {
		// current words and their properties
		char **words = (char **) malloc( N * sizeof( char *));
		int *reps = (int *) malloc( N * sizeof( int));
		int *qstat = (int *) malloc( N * sizeof( int));
		for ( int i = 0; i < N; i++) {
			qstat[i] = 1;
			reps[i] = 0;
		}

		// messages from queues
		struct mq_attr mq_attr;
		mq_getattr( qs[0], &mq_attr);
		int buflen = mq_attr.mq_msgsize;
		char *bufptr = (char *) malloc( buflen);
		item_t *msg = NULL;

		while ( 1) {
			// get first items from message queues
			for ( int i = 0; i < N; i++) {
				if ( qstat[i] && reps[i] == 0) {
					int s;
					s = mq_receive( qs[i], (char *) bufptr, buflen, NULL);

					if ( s!= -1) {
						msg = (item_t *) bufptr;

						reps[i] = msg->freq;
						if ( reps[i] > 0) {
							int size = msg->size;
							words[i] = (char *) malloc( size+1);
							strcpy( words[i], msg->word);
						}
						else
							qstat[i] = 0;
					}
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

			// if all queues are empty break loop
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

		// exit processes
		for ( int i = 0 ; i <= N ; i++) {
			wait(NULL);
		}
		if ( msg)
			free( msg);

		// close output file
		fclose( fp);
		printf( "Results were written into \"%s\".\n", argv[N+2]);
	}

	// close and unlink message queues and deallocate memory
	for ( int i = 0; i < N; i++) {
		mq_close( qs[i]);
		mq_unlink( qnames[i]);
	}
	for ( int i = 0; i < N; i++) {
		if ( qnames[i])
			free( qnames[i]);
	}
	free( qnames);
	free( qs);

    gettimeofday( &t2, NULL);
    bFTime = (ms(t2) - ms(t1));
    printf( "Completed in %ld μs.\n", bFTime);

	return 0;
}
