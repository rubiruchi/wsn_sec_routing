#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int SharedVariable =0;
void *SimpleThread(void *args)
{
    int num,val,rc;
    int which =(int)args;
    rc = pthread_mutex_lock(&mutex1);
    for(num=0; num<20; num++){
#ifdef PTHREAD_SYNC
        if(random() > RAND_MAX / 2)
            usleep(10);
#endif
        //pthread_mutex_lock(&mutex1);
        val = SharedVariable;
        printf("*** thread %d sees value %d\n", which, val);
        //pthread_mutex_lock(&mutex1);
        SharedVariable = val+1;
        pthread_mutex_unlock(&mutex1);
    }   
    val=SharedVariable;

    printf("Thread %d sees final value %d\n", which, val);
    //pthread_mutex_destroy(&mutex1);
    //pthread_exit((void*) 0);
    //pthread_mutex_unlock(&mutex1);

}

int main (int argc, char *argv[])
{
   if(atoi(argv[1]) > 0){        
   int num_threads = atoi(argv[1]);  
   //pthread_mutex_init(&mutex1, NULL);
   pthread_t threads[num_threads];
   int rc;
   long t;
   rc = pthread_mutex_lock(&mutex1);
   for(t=0; t< num_threads; t++){
      printf("In main: creating thread %ld\n", t);
      rc = pthread_create(&threads[t], NULL, SimpleThread, (void* )t);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }

    //pthread_join(thread1);

   }
    rc= pthread_mutex_unlock(&mutex1);
}
   else{
      printf("ERROR: The parameter should be a valid positive number.");
      exit(-1);
  }

   pthread_mutex_destroy(&mutex1);
   pthread_exit(NULL);
}