//
// Created by bcr33d on 11/28/20.
//å

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define bucket_size 256
#define NumberOfQueue 4
int count=0;
pthread_mutex_t count_lock;
pthread_mutex_t buckets_lock;
struct queue_t* queues;
//struct Bucket *buckets[bucket_size];
//condition variables
int ready =0;
pthread_mutex_t mutex;
pthread_cond_t cond;



void waitForReady(){
    pthread_mutex_lock(&mutex);
    while(!ready){
        pthread_cond_wait(&cond,&mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void markReady(){
    pthread_mutex_lock(&mutex);
    ready=1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}



struct node_t{
    struct node_t* next;
    char* word;
};


struct queue_t{
    struct node_t *head;
    struct node_t *tail;
    pthread_mutex_t  headLock;
    pthread_mutex_t  tailLock;
};



void Queue_Init(struct queue_t* q){
    struct node_t *tmp = malloc(sizeof(struct node_t));
    tmp->next = NULL;
    q->head = q->tail = tmp;
    pthread_mutex_init(&q->headLock,NULL);
    pthread_mutex_init(&q->tailLock,NULL);
}

void Queue_Enqueue(struct queue_t* q, char* word){
    struct node_t *tmp = malloc(sizeof(struct node_t));
    if(tmp == NULL){
        perror("malloc");
        return;
    }
    tmp->word = word;
    tmp->next = NULL;

    pthread_mutex_lock(&q->tailLock);
    q->tail->next = tmp;
    q->tail = tmp;
    pthread_mutex_unlock(&q->tailLock);
}

int Queue_Dequeue(struct queue_t* q, char* word){
    pthread_mutex_lock(&q->headLock);
    struct node_t *tmp = q->head;
    struct node_t *newHead = tmp->next;
    if(newHead == NULL){
        pthread_mutex_unlock(&q->headLock);
        return -1; // queue was empty
    }
    word = newHead->word;
    q->head = newHead;
    pthread_mutex_unlock(&q->headLock);
    free(tmp);
    return 0;
}








struct list_t{
    struct node_t *head;
    pthread_mutex_t head_lock;
};

void List_Init(struct list_t *list){
    list->head = NULL;
}

void List_Insert(struct list_t *list, char* word){
    struct node_t *new = malloc(sizeof(struct node_t));
    if(new == NULL){
        perror("malloc");
        return;
    }
    new->word = word;
    new->next = list->head;
    list->head = new;
}

int List_Lookup(struct list_t *l, char* word){
    int rv = -1;
    struct node_t *cur = l->head;
    while (cur){
        if(strcmp(cur->word,word)==0){
            rv=0;
            break;
        }
        cur = cur->next;
    }
    return rv;
}

struct hash_t{
    struct list_t lists[bucket_size];
};

void Hash_Init(struct hash_t* h){
    for(int i=0; i < bucket_size; i++){
        List_Init(&h->lists[i]);
    }
}

void Hash_Insert(struct hash_t *h, int key, char* word){
    return List_Insert(&h->lists[key],word);
}

int Hash_Lookup(struct hash_t *h, int key, char* word){
    return List_Lookup(&h->lists[key],word);
}



unsigned hashing(char *word){
    unsigned key = 0;
    for(int i=0; i < strlen(word);i++){
        key+=word[i];
    }
    key = key % bucket_size;
    return key;
}

void *consumerThread(void *arg){
    struct hash_t* hashtable = malloc(sizeof(struct hash_t));


    return NULL;
}

void *producerThread(void *arg){
    struct hash_t* table = malloc(sizeof(struct hash_t));
    Hash_Init(table);
    FILE *fh = fopen((char*)arg,"r");
    if(fh == NULL){
        printf("%s: No such file or directory\n",(char*)arg);
        return NULL;
    }
    char* ptr;
    while( fscanf(fh,"%ms",&ptr) != EOF) {
        int twoBits = ptr[0]&3;
        pthread_mutex_lock(&queues[twoBits].headLock);
        Queue_Enqueue(&queues[twoBits],ptr);
        pthread_mutex_unlock(&queues[twoBits].headLock);
    }
    //free(ptr);
    free(ptr);
    fclose(fh);
    return NULL;
}






int main(int argc, char *argv[]) {
    queues = malloc(sizeof(struct queue_t)* NumberOfQueue );
    for(int i=0; i < NumberOfQueue; i++){
        Queue_Init(&queues[i]);
    }
    int threadsNum = argc-1;
    pthread_t threads[threadsNum];
    for(int i=0; i < threadsNum;i++){
        pthread_create(&threads[i],NULL,producerThread,argv[i+1]);
        //pthread_create(&threads[i],NULL,producerThread,argv[i+1]);
        pthread_join(threads[i],NULL);
    }

    for( int i= 0; i < NumberOfQueue; i++){
        struct node_t* cur = queues[i].head;
        while(cur->next != NULL){
            count++;
            cur = cur->next;
        }
    }

    printf("count: %d\n", count);



    //free(buckets);
    printf("%d\n", count);
    return 0;

}

