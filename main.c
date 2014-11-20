#include <stdio.h>
#include <string.h>
#include <libmemcached/memcached.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_KEY_SIZE 64
#define key_format "latebench_%d"
#define MEMCACHED_CONF "--SERVER=192.168.0.1"

int MAX_KEY_NUMBER = 1000000;

int MULTIGET_SIZE = 5000;
int MULTIGET_STEP = 100;
int BENCHMARK_ITERATIONS = 100000;

int MAX_PAYLOAD_SIZE = 2048;

int CLIENT_THREADS = 4;

int WARMUP_SERVER = 0;

typedef struct benchmark_args {
    memcached_st* m;
    struct timespec** latencies;
    int num_iters;
    int multiget_size;
} benchmark_args;

int parse_args(int argc, char** argv){
    int i;
    for (i = 0; i < argc; i++){
        if (!strcmp(argv[i], "-maxkeys")){
            MAX_KEY_NUMBER = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "-getsize")){
            MULTIGET_SIZE = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "-stepsize")){
            MULTIGET_STEP = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "-iterations")){
            BENCHMARK_ITERATIONS = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "-threads")){
            CLIENT_THREADS = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "-payload")){
            MAX_PAYLOAD_SIZE = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "-warmup")){
            WARMUP_SERVER = 1;
        }
    }
    return 0;
}

void warmup(memcached_st* m, int numkeys){
   int i;
   char* key = malloc(MAX_KEY_SIZE);
   char* value = malloc(MAX_PAYLOAD_SIZE);
   for (i=0; i < MAX_PAYLOAD_SIZE; i++){
        value[i] = 'a' + (i % 26);
   }
   for (i=0; i < numkeys; i++){
       sprintf(key, key_format, i);
        memcached_return_t rc = memcached_set(
            m,
            key,
            strlen(key),
            value,
            (rand() % MAX_PAYLOAD_SIZE) + 1,
            (time_t)0,
            (uint32_t)0
        );
        if (rc != MEMCACHED_SUCCESS){
            fprintf(stderr, "SET err %d\n", rc);
        }
        if (i % 100 == 0){
            float perc = ((float) i * 100.0) / (float) numkeys;
            fprintf(stderr, "%d/%d (%.2f%%)\r", i, numkeys, perc);
        }
   }
}

void * benchmark(void* thread_args){
    benchmark_args* args = (benchmark_args*) thread_args;
    char** multiget_keys = malloc(sizeof(char*) * args->multiget_size);
    size_t* multiget_keylengths = malloc(sizeof(size_t*) * args->multiget_size);

    int i;
    fprintf(stderr, "Init %d latency values...\n", args->num_iters);
    for (i = 0; i<args->num_iters; i++){
        args->latencies[i] = malloc(sizeof(struct timespec));
    }
    fprintf(stderr, "Init %d keyspaces...\n", args->multiget_size);
    for (i=0; i < args->multiget_size; i++){
        multiget_keys[i] = malloc(sizeof(char) * MAX_KEY_SIZE);
    }

    memcached_return_t rc;
    struct timespec start, end;
    for (i=0; i < args->num_iters; i++){
        uint32_t flags;

        int k;
        for (k=0; k < args->multiget_size; k++){
            sprintf(multiget_keys[k], key_format, (rand() % args->multiget_size));
            multiget_keylengths[k] = strlen(multiget_keys[k]);
        }

        char return_key[MEMCACHED_MAX_KEY];
        size_t return_key_length;
        char *return_value;
        size_t return_value_length;

        clock_gettime(CLOCK_MONOTONIC, &start);
        rc = memcached_mget(
                args->m,
                (const char * const*)multiget_keys,
                multiget_keylengths,
                args->multiget_size
            );
        clock_gettime(CLOCK_MONOTONIC, args->latencies[i]);

        if (args->latencies[i]->tv_nsec < start.tv_nsec){
            args->latencies[i]->tv_nsec = 1000000000 - (start.tv_nsec - args->latencies[i]->tv_nsec);
            args->latencies[i]->tv_sec--;
        } else {
            args->latencies[i]->tv_nsec -= start.tv_nsec;
        }
        args->latencies[i]->tv_sec -= start.tv_sec;
    }
    return NULL;
}

void do_benchmark_to_file(FILE* file, int iterations, int multiget_size){
    benchmark_args** thread_args = malloc(sizeof(benchmark_args*) * CLIENT_THREADS);
    pthread_t thread_ids[CLIENT_THREADS];

    int i, j;
    for (i = 0; i < CLIENT_THREADS; i++){
        thread_args[i] = malloc(sizeof(struct benchmark_args));
        thread_args[i]->m = memcached(MEMCACHED_CONF, strlen(MEMCACHED_CONF));
        thread_args[i]->latencies = malloc(sizeof(struct timespec*) * iterations);
        thread_args[i]->num_iters = iterations;
        thread_args[i]->multiget_size = multiget_size;
        pthread_create(&thread_ids[i], NULL, benchmark, thread_args[i]);
    }

    for (i = 0; i < CLIENT_THREADS; i++){
        pthread_join(thread_ids[i], NULL);
    }

    for (i = 0; i < CLIENT_THREADS; i++){
        for (j=0; j < BENCHMARK_ITERATIONS; j++){
            fprintf(
                file,
                "%d.%09d\n",
                thread_args[i]->latencies[j]->tv_sec,
                thread_args[i]->latencies[j]->tv_nsec
            );
        }
    }
}


int main (int argc, char** argv){
    parse_args(argc, argv);
    srand(time(NULL));
    memcached_st* m = memcached(MEMCACHED_CONF, strlen(MEMCACHED_CONF));
    if (WARMUP_SERVER){
        warmup(m, MAX_KEY_NUMBER);
    }
    int i;
    for (i = MULTIGET_STEP; i < MULTIGET_SIZE; i+= MULTIGET_STEP){
        char* filename = malloc(sizeof(char) * 256);
        sprintf(filename, "%d_iters_%d_keys", BENCHMARK_ITERATIONS, i);
        FILE* output = fopen(filename, "w+");
        do_benchmark_to_file(output, BENCHMARK_ITERATIONS, i);
        fclose(output);
    }
}
