#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_STRING_LENGTH 30
#define ASCII_SIZE	256
int stat[MAX_STRING_LENGTH];	// distribution of input string length
int stat2[ASCII_SIZE];			// frequency of input ascii char values

typedef struct sharedobject {
	FILE *rfile;
	int linenum;
	char *line[10000];			// 버퍼의 갯수를 늘리기 위함
	pthread_mutex_t lock;		
	pthread_mutex_t lock2;		// count++를 보호하기 위한 다른 lock변수 선언
	pthread_cond_t cv;
	int full[10000];			
	int c_num;					// 선언된 Consumer의 갯수
} so_t;	

int c_index;					
int res;						// buffer의 상태를 제어하기 위함
int sum = 0;

void *producer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int i = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int target;        

	while (1) {
		read = getdelim(&line, &len, '\n', rfile);
		pthread_mutex_lock(&so->lock);
		
		while (so->full[target] == 1) {		
			pthread_cond_wait(&so->cv, &so->lock);
		}

		if (read == -1) {                       
			so->full[target] = 1;		
			so->line[target] = NULL;		

			pthread_cond_broadcast(&so->cv);
			pthread_mutex_unlock(&so->lock);
			break;
		}
		so->linenum = i;
		so->line[target] = strdup(line);
		i++;

		so->full[target] = 1;
		target++;
		if(target == so->c_num){		// target변수 동기화
			target = 0;
			pthread_cond_broadcast(&so->cv);
		}

		pthread_cond_broadcast(&so->cv);
		pthread_mutex_unlock(&so->lock);
	}
	free(line);
	line = NULL;		
	printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
	*ret = i;
	pthread_exit(ret);

}

void *consumer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int i = 0;
	int len;
	char *line;
	int target=0;

	size_t length = 0;
	char *cptr = NULL;
	char *substr = NULL;
	char *brka = NULL;
	char *sep = "{}()[],;\" \n\t^";
	
	pthread_mutex_lock(&so->lock2);
	target = c_index++;
	pthread_mutex_unlock(&so->lock2);

	while (1) {
		cptr = NULL;
		substr = NULL;
		brka = NULL;

		pthread_mutex_lock(&so->lock);
		while (so->full[target] == 0) {
			if (res == 1)
				break;
			pthread_cond_wait(&so->cv, &so->lock);	
		}

		line = so->line[target];
		if (line == NULL) {		
			pthread_cond_broadcast(&so->cv);
			pthread_mutex_unlock(&so->lock);
			break;
		}

		len = strlen(so->line[target]);
		printf("Cons_%x: [%02d:%02d] %s",
			(unsigned int)pthread_self(), i, so->linenum, line);

		cptr = line;

		for (substr = strtok_r(cptr, sep, &brka); substr; substr = strtok_r(NULL, sep, &brka))
		{
			length = strlen(substr);
			cptr = cptr + length + 1;
			if (length >= 30) length = 30;
			stat[length - 1]++;
			if (*cptr == '\0') break;

			cptr = substr;
			for (int i = 0; i < length; i++)
			{
				if (*cptr < 256 && *cptr > 1)
				{
					stat2[*cptr]++;
				}
				cptr++;
			}
		}

		sum = 0;
		for (int i = 0; i < 30; i++) {
			sum += stat[i];
		}
		
		free(so->line[target]);
		so->line[target] = NULL;		
		i++;

		so->full[target] = 0;
	
		
		pthread_cond_broadcast(&so->cv);
		pthread_mutex_unlock(&so->lock);
	}
	res = 1;
	printf("Cons: %d lines\n", i);
	*ret = i;
	pthread_exit(ret);

}

int main(int argc, char *argv[])
{
	pthread_t prod[100];
	pthread_t cons[100];
	int Nprod, Ncons;
	int rc;   long t;
	int *ret;
	int i;
	FILE *rfile;
	if (argc == 1) {
		printf("usage: ./prod_cons <readfile> #Producer #Consumer\n");
		exit(0);
	}
	so_t *share = malloc(sizeof(so_t));
	memset(share, 0, sizeof(so_t));
	rfile = fopen((char *)argv[1], "r");
	if (rfile == NULL) {
		perror("rfile");
		exit(0);
	}
	if (argv[2] != NULL) {
		Nprod = atoi(argv[2]);
		if (Nprod > 100) Nprod = 100;
		if (Nprod == 0) Nprod = 1;
	}
	else Nprod = 1;
	if (argv[3] != NULL) {
		Ncons = atoi(argv[3]);
		if (Ncons > 100) Ncons = 100;
		if (Ncons == 0) Ncons = 1;
	}
	else Ncons = 1;

	share->c_num = Ncons;		
	share->rfile = rfile;

	pthread_mutex_init(&share->lock, NULL);
	pthread_mutex_init(&share->lock2, NULL);
	pthread_cond_init(&share->cv, NULL);
	
	for (i = 0; i < Nprod; i++)
		pthread_create(&prod[i], NULL, producer, share);
	for (i = 0; i < Ncons; i++)
		pthread_create(&cons[i], NULL, consumer, share);

	printf("main continuing\n");

	for (i = 0; i < Ncons; i++) {
		rc = pthread_join(cons[i], (void **)&ret);
		printf("main: consumer_%d joined with %d\n", i, *ret);
	}
	for (i = 0; i < Nprod; i++) {
		rc = pthread_join(prod[i], (void **)&ret);
		printf("main: producer_%d joined with %d\n", i, *ret);
	}

	printf("*** print out distributions *** \n");
	printf("  #ch  freq \n");
	for (i = 0; i < 30; i++) {
		int j = 0;
		int num_star = stat[i] * 80 / sum;
		printf("[%3d]: %4d \t", i + 1, stat[i]);
		for (j = 0; j < num_star; j++)
			printf("*");
		printf("\n");
	}
	printf("       A        B        C        D        E        F        G        H        I        J        K        L        M        N        O        P        Q        R        S        T        U        V        W        X        Y        Z\n");
	printf("%8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
		stat2['A'] + stat2['a'], stat2['B'] + stat2['b'], stat2['C'] + stat2['c'], stat2['D'] + stat2['d'], stat2['E'] + stat2['e'],
		stat2['F'] + stat2['f'], stat2['G'] + stat2['g'], stat2['H'] + stat2['h'], stat2['I'] + stat2['i'], stat2['J'] + stat2['j'],
		stat2['K'] + stat2['k'], stat2['L'] + stat2['l'], stat2['M'] + stat2['m'], stat2['N'] + stat2['n'], stat2['O'] + stat2['o'],
		stat2['P'] + stat2['p'], stat2['Q'] + stat2['q'], stat2['R'] + stat2['r'], stat2['S'] + stat2['s'], stat2['T'] + stat2['t'],
		stat2['U'] + stat2['u'], stat2['V'] + stat2['v'], stat2['W'] + stat2['w'], stat2['X'] + stat2['x'], stat2['Y'] + stat2['y'],
		stat2['Z'] + stat2['z']);

	pthread_exit(NULL);
	exit(0);
}

