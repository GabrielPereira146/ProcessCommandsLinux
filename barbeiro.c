#include <pthread.h>
#include <semaphore.h>
#include <stdio.h> 
#include <stdlib.h>
#include<unistd.h>

struct job {
	/* Campo de ligação para a fila encadeada. */
	struct job* next;
	int id_Client;
};

struct barbeiros_parms
{
  int id;
  
  
};

int maxClients;
int clientsWaiting = 0;
int timeJob;

extern void do_work();
/* Uma fila encadeada de trabalhos pendentes. */
struct job* client_queue;

extern void process_job (struct job*);

/* Uma mutex protegendo a fila de tarefas. */
pthread_mutex_t client_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Um semáforo contando o número de tarefas na fila. */
sem_t client_queue_count;

/* Realizar a iniciação da fila de tarefas uma única vez. */
void initialize_client_queue ()
{
	/* A fila está inicialmente vazia. */
	client_queue = NULL;
	/* Inicializar o semáforo que conta as tarefas na fila. Seu
	   valor inicial deve ser zero. */
	sem_init (&client_queue_count, 0, 0);
}


int thread_flag;

pthread_cond_t thread_flag_cv;
pthread_mutex_t thread_flag_mutex;

void initialize_flag ()
{
	/* Inicializar a mutex e a variável de condição. */
	pthread_mutex_init (&thread_flag_mutex, NULL);
	pthread_cond_init (&thread_flag_cv, NULL);
	/* Inicializar o valor da flag. */
	thread_flag = 0;
}

/* Chamar do_work repetidamente até que a flag da thread seja
   ativada; bloqueia se a flag estiver desativada. */
void* thread_function (void* thread_arg)
{
	//fprintf(stderr,"AAAA");
	struct barbeiros_parms* b = (struct barbeiros_parms*) thread_arg;

	/* Loop infinito. */
	while (1) {
		/* Trava a mutex antes de acessar o valor da flag. */
		pthread_mutex_lock (&thread_flag_mutex);
		printf("Barbeiro %d dormindo\n", b->id);
		while (!thread_flag)
			/* A flag está desligada. Espere por um sinal na variável de
			   condição, indicando que o valor da flag mudou. Quando o
			   sinal chega e a thread desbloqueia, faça o loop e cheque a

			   flag novamente. */
			pthread_cond_wait (&thread_flag_cv, &thread_flag_mutex);
		/* Quando chegamos aqui, sabemos que a flag deve estar ativada.
		   Destrave a mutex. */
		pthread_mutex_unlock (&thread_flag_mutex);
		printf("Barbeiro %d acordou\n",b->id);
		/* Faça algum trabalho. */
	        do_work(b->id);
	}
	return NULL;
}
/* Ajuste o valor da flag para FLAG_VALUE. */
void set_thread_flag (int flag_value)
{
	/* Travar a mutex antes de acessar o valor da flag. */
	pthread_mutex_lock (&thread_flag_mutex);
	/* Ajustar o valor da flag, e então sinalizar para o caso da função de
	   thread
	   estar bloqueada, esperando pela ativação da flag. Porém,
	   a função de thread não poderá realmente checar o valor da flag até que
	   a mutex seja desbloqueada. */
	thread_flag = flag_value;
	pthread_cond_signal (&thread_flag_cv);
	/* Destravar a mutex. */
	pthread_mutex_unlock (&thread_flag_mutex);
}

/* Adiconar uma nova tarefa na frente da fila de tarefas. */
void enqueue_client (int id)
{
	printf("Cliente %d chegou\n", id);
	if(clientsWaiting>=maxClients){
		printf("Cliente %d foi embora sem cortar o cabelo. Sala de espera cheia\n", id);
	}else{

		struct job* new_client;
		/* Alocar o novo objeto de tarefa. */
		new_client = (struct job*) malloc (sizeof (struct job));
		new_client->next = NULL;
		/* Ajustar outros campos para a estrutura da tarefa aqui... */
		new_client->id_Client = id;
		/* Travar a mutex na fila de tarefas antes de acessá-la. */
		pthread_mutex_lock (&client_queue_mutex);
		/* Colocar a nova tarefa no fim da fila. */
		if(client_queue == NULL){
			client_queue = new_client;
		}else{
			struct job* temp = (struct job*) malloc (sizeof (struct job));
			temp = client_queue;
			while(temp->next != NULL)
				temp= temp->next;
			temp->next = new_client;
			free(temp);
		}
		clientsWaiting++;
	//	set_thread_flag(1);
		/* Postar para o semáforo para indicar que outra tarefa está disponível. Se threads estão
		   bloqueadas, esperando o semáforo, uma será desbloqueada e processará a tarefa. */
		sem_post (&client_queue_count);
		/* Destravar a mutex da fila de tarefas. */
		pthread_mutex_unlock (&client_queue_mutex);
	}
}

void do_work(int idBarbeiro){
	printf("Barbeiro %d cortando o cabelo do cliente\n",idBarbeiro);
	sleep(timeJob);
	clientsWaiting--;
	set_thread_flag(0);
}

int main (int argc, char* argv[])
{
 
	int num_barbeiros = atoi(argv[1]);
	maxClients = atoi(argv[2]);
	timeJob = atoi(argv[3]);
	int i;
	int timeComing = atoi(argv[4]);
	struct barbeiros_parms barbeiros_args[num_barbeiros];
	pthread_t barbeiros[num_barbeiros];
	/* Create a new thread. */
	for(i = 0; i< num_barbeiros; i++){
		barbeiros_args[i].id = i+1;
		pthread_create (&(barbeiros[i]), NULL, &thread_function , &barbeiros_args[i]);
	}

	i = 1;
	while(1){
		
		enqueue_client(i);
		i++;
		sleep(timeComing);
	}

	return 0;
}

