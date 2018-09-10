#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include "fifo.h"
#include "sem.h"

// Delay opcinal para misturar a execucao das threads
#define DELAY() usleep((rand() % 100) * 100)

#define N 10          // Tamanho da fila (quantidade de mensagens)
#define ITERACOES 50  // Numero total de dados a colocar na fila por produtor

typedef struct {
	semaphore_t sem;
	char s[];
} dados_t;

fifo_t fila;           // Fila de N posicoes
uint32_t produtores;   // Quantidade de produtores
uint32_t consumidores; // Quantidade de consumidores
int * ids;             // IDs das threads

void deposita(dados_t * dados) {
	fifo_push(&fila, dados);
}

dados_t * consome(int id) {
	void * ret;
	fifo_pop(&fila, id, &ret);
	return (dados_t *)ret;
}

void * f_produtor(void * p) {
	int id = *(int *)p; // Thread ID

	for (int i=0; i<ITERACOES; i++) {
		DELAY();
		dados_t * dados = (dados_t *)malloc(sizeof(dados_t) + 100);
		sem_create(&dados->sem, consumidores);
		snprintf(dados->s, 100, "Thread %d: %p", id, dados);
		printf("Produzido (%d): %s\n", id, dados->s);
		deposita(dados);
	}
	return NULL;
}

void * f_consumidor(void * p) {
	int id = *(int *)p; // Thread ID

	for (uint32_t i=0; i<ITERACOES * produtores; i++) {
		DELAY();
		dados_t * dados = consome(id);
		printf("Consumido (%d): %s\n", id, dados->s);
		if (!sem_wait(&dados->sem))
			free(dados);
	}
	return NULL;
}

int main(int argc, char ** argv) {
	srand(time(NULL));

	if (argc != 3) {
		printf("Uso: pcp <produtores> <consumidores>\n");
		return 1;
	}

	produtores   = atoi(argv[1]);
	consumidores = atoi(argv[2]);
	printf("Inicio - %u produtores, %u consumidores\n", produtores, consumidores);

	fifo_init(&fila, N, consumidores);
	
	int qtd_ids = produtores > consumidores ? produtores : consumidores;
	ids = (int *)malloc(qtd_ids * sizeof(int));
	for (int i=0; i<qtd_ids; i++)
		ids[i] = i;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_t * threads = malloc((produtores + consumidores) * sizeof(pthread_t));
	for (int i=0; i<produtores; i++)
		pthread_create(&threads[i], &attr, f_produtor, &ids[i]);

	for (int i=0; i<consumidores; i++)
		pthread_create(&threads[i + produtores], &attr, f_consumidor, &ids[i]);

	for (int i=0; i<produtores + consumidores; i++)
		pthread_join(threads[i], NULL);

	printf("Fim\n");
	fifo_destroy(&fila);
	free(ids);
	free(threads);
	pthread_attr_destroy(&attr);
	pthread_exit(NULL);
	return 0;
}
