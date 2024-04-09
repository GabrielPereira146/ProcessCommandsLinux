#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define VECTOR_SIZE 100000000

struct thread_parms
{
  int start;
  int end;
  double *x;
};

double calculate_f(int x) {
    double test;	
    test = pow(2, pow((-2) * ((x - 0.1)/0.9),2))* pow(sin(5*M_PI*x),6);
    //fprintf(stderr,"%.6f",test);
    return test;
}

void* thread_exec (void *parameters)
{
  /* Cast do ponteiro para o tipo correto. */
  struct thread_parms* p = (struct thread_parms*) parameters;
  intptr_t sum = 0;  
  for(int i = p->start; i< p->end;i++){
    sum += calculate_f(p->x[i]);	
  }
  return (void*) sum;
}

/* The main program.  */

int main (int argc, char* argv[])
{
   int num_threads = atoi(argv[1]);
   int i;
   if (num_threads <= 0) {
       printf("Invalid number of threads\n");
       return 1;
   }
   // vector x of ramdon elements
   double *x = (double *)malloc(VECTOR_SIZE * sizeof(double));

   for (i = 0; i < VECTOR_SIZE; i++) {
       x[i] = ((double)rand()/RAND_MAX ); // Values 0 and  1
       //fprintf(stderr,"%d ",x[i]);				// 
   }

  //Count time of execution
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);	
  
  struct thread_parms thread_args[num_threads];
  pthread_t threads[num_threads];
  int num_elements = VECTOR_SIZE/num_threads;
  /* Create a new thread. */
  for(i = 0; i< num_threads; i++){
    thread_args[i].start = i*num_elements;
    thread_args[i].end = (i+1)*num_elements;
    thread_args[i].x = x;    
    pthread_create (&(threads[i]), NULL, thread_exec, &thread_args[i]);
  }
   // Esperar todas as threads terminarem.
  double tot_sum = 0;
  for(i = 0; i< num_threads; i++){
    intptr_t thread_result;	  
    pthread_join (threads[i],(void *)&thread_result);
   // fprintf(stderr,"%f ",thread_result);
    tot_sum += thread_result;
  }
  // Parar contagem de tempo e calcular duração
  gettimeofday(&end_time, NULL);
  double execution_time = (double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_usec - start_time.tv_usec) / 1000000;
 
 double avarage = tot_sum/VECTOR_SIZE; 
  // Exibir resultados
    printf("Average value of f(xi): %.6f\n", avarage);
    printf("Execution time: %.6f seconds\n", execution_time);
   free(x);
    
  return 0;
}

