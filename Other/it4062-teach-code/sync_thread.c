#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include 	<pthread.h>

#define	MAXNITEMS 		1000000
#define	MAXNTHREADS		100

		/* globals shared by threads */
int		nitems;				/* read-only by producer and consumer */
int		buff[MAXNITEMS];
struct {
  pthread_mutex_t	mutex;
  int				nput;	/* next index to store */
  int				nval;	/* next value to store */
} put = { PTHREAD_MUTEX_INITIALIZER };

struct {
  pthread_mutex_t	mutex;
  pthread_cond_t	cond;
  int				nready;	/* number ready for consumer */
} nready = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER };
/* end globals */

void	*produce(void *), *consume(void *);

/* include main */
int main(int argc, char **argv)
{
	int		i, nthreads, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3){
		printf("usage: sync <#items> <#threads>");
		return 0;
	}
	nitems = atoi(argv[1]);
	if (nitems > MAXNITEMS)
		nitems = MAXNITEMS;
	printf("Number of item: %d\n", nitems);
	nthreads = atoi(argv[2]);
	if (nthreads > MAXNTHREADS)
		nthreads = MAXNTHREADS;
	printf("Number of thread: %d\n", nthreads);
	
	
	/* create all producers and one consumer */
	for (i = 0; i < nthreads; i++) {
		count[i] = 0;
		pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}
	pthread_create(&tid_consume, NULL, consume, NULL);

		/* wait for all producers and the consumer */
	for (i = 0; i < nthreads; i++) {
		pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);	
	}
	pthread_join(tid_consume, NULL);

	return 0;
}
/* end main */

/* include prodcons */
void * produce(void *arg)
{
	for ( ; ; ) {
		pthread_mutex_lock(&put.mutex);
		if (put.nput >= nitems) {
			pthread_mutex_unlock(&put.mutex);
			return(NULL);		/* array is full, we're done */
		}
		buff[put.nput] = put.nval;
		put.nput++;
		put.nval++;
		pthread_mutex_unlock(&put.mutex);

		pthread_mutex_lock(&nready.mutex);
		if (nready.nready == 0)
			pthread_cond_signal(&nready.cond);
			
		nready.nready++;
		pthread_mutex_unlock(&nready.mutex);

		*((int *) arg) += 1;
	}
}

void * consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) {
		pthread_mutex_lock(&nready.mutex);
		while (nready.nready == 0)
			pthread_cond_wait(&nready.cond, &nready.mutex);
		nready.nready--;
		pthread_mutex_unlock(&nready.mutex);

		if (buff[i] != i)
			printf("buff[%d] = %d\n", i, buff[i]);
	}
	return(NULL);
}
/* end prodcons */