#ifndef MAIN_H
#define MAIN_H
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#define RESP_SIZE 256
#define AUTH_TRUE 0
#define AUTH_FALSE 1
#define AUTH_SPAM 2
#define INITIAL_PORT 9050

static const char* alpha_charset = "abcdefghilmnopqrsrtuvzABCDEFGHILMNOPQRSTUVZ";
static const char* number_charset = "123456789";

typedef struct th{
    pid_t tor;
    size_t port;
}input_t;

typedef struct memory{
   char response[RESP_SIZE];
   size_t size;
 }callback_t;
 
typedef struct list{
    char* word;
    struct list* next;
}list_t;

typedef struct random{
    char number[11];
    char token[33];
}random_t;

void insert(list_t** head, char* w);
char* pop(list_t** head);
void visit(list_t* head);
void help(char* elf_name);
void spawn_proxy(pid_t* tor, size_t ses);
void rand_gen(random_t* rands, unsigned int* seed_r);
short ast(char* line);

#define SYSNM1(name, sys)		    \
  if (sys == -1) {					\
      perror(#name);						\
      exit(EXIT_FAILURE);					\
    }

#define SYSM1(name, ret, sys)		    \
  if ((ret = sys) == -1) {					\
      perror(#name);						\
      exit(EXIT_FAILURE);					\
    }

#define CHECKEQUAL(name, check, val) \
  if ((check)!=val) {					 \
    perror(#name);						 \
    exit(EXIT_FAILURE);					 \
  }


void insert(list_t** head, char* w){
    list_t* node = (list_t*)malloc(sizeof(list_t));
    assert(node);
    (*node).word = (char*)malloc(sizeof(char)*strlen(w));
    assert((*node).word);
    memcpy((*node).word,w,sizeof(char)*strlen(w));
    (*node).word[strlen(w)-1] = '\0'; 
    (*node).next = *head;
    *head = node;
}

char* pop(list_t** head){
    if (*head == NULL){
        return NULL;
    }
    char* myw = (*head)->word;
    list_t* save = *head;
    *head = (*head)->next;
    free(save);
    return myw;
}

void visit(list_t* head){
    while(head != NULL){
        printf("word -> %s\n",(*head).word);
        head = (*head).next;
    }
}

void help(char* elf_name){
    printf("Usage %s\n  %s <igname> <wlist> <thread_number>\n",elf_name+2,elf_name);
}

void spawn_proxy(pid_t* tor, size_t num){
    char portC[5];
    char dir[14];
    size_t port = INITIAL_PORT;
    for (size_t i = 0; i < num; i++){
        snprintf(portC,5,"%lu",port);
        snprintf(dir,14,"/tmp/tor_%lu",i);
        printf("p = %s, dir = %s\n",portC,dir);
        tor[i] = fork();
        if (tor[i] == 0){
          close(0);
          execlp("tor","tor","--SocksPort",portC,"--DataDirectory",dir,(char*)NULL);
          break;
        }
        port++;
    }
}

void rand_gen(random_t* rands, unsigned int* seed_r){
    *seed_r ^= time(NULL);
    for (size_t i = 0; i < 32; i++){
        (*rands).token[i] = alpha_charset[rand_r(seed_r)%43];
        if (i < 10){
            (*rands).number[i] = number_charset[rand_r(seed_r)%9];
        }
        *seed_r|=i;
    }
}

short ast(char* line){
    char* subinit = NULL;
    subinit = strstr(line,"authenticated\":true");
    if (subinit != NULL){
        return AUTH_TRUE;
    }
    subinit = strstr(line,"checkpoint_required");
    if (subinit != NULL){
        return AUTH_TRUE;
    }
    subinit = strstr(line,"authenticated\":false");
    if (subinit != NULL){
        return AUTH_FALSE;
    }
    return AUTH_SPAM;
}

#endif
