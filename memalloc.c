
//#define improvement
#define basic

/*
 * memalloc.c for cs 5204 lab 2 part 2- Joshua Martin
 * 
make
sudo insmod ko5204.ko
gcc memalloc.c -o memalloc
sudo setarch --verbose --addr-no-randomize ./memalloc
sudo rmmod ko5204.ko
sudo tail -n 650 /var/log/kern.log > heatLog.csv

clean up heatLog.csv manually - pretty easy

python3 heatmap.py
*/

/*For latency distribution
for x in {1..1000}; do sudo setarch --verbose --addr-no-randomize ./memalloc; done

while also running sudo dmesg --follow > dmesgLog
*/
 
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<pthread.h>

//part 2 only uses /proc/5204(k)
// #include "pagemap.h"

#ifdef improvement
static int continueThread = 1;

void* fastWrite(void* p){
    while(continueThread){
        for (int i = 0; i < 256*1024*1024; i+= 4096){
            void * temp = p+i;
            ((int*)temp)[0] = 1;
        }
    }
}

void* delayWrite(void* p){
    // for (int j = 0; j < 1000; j++){
        for (int i = 256*1024*1024; i < 512*1024*1024; i+= 4096){
            void * temp = p+i;
            ((int*)temp)[0] = 1;
        }
    // }
}
#endif

void sendVa(void* va, FILE *procFile){
    fprintf(stderr, "sending 0x%lx\n", (unsigned long)va);
    fprintf(procFile, "%p", va);
}


void* kernelCall(void* p){
    //each loop takes at least 90ms cant to every ms?
    FILE *procFile = fopen("/proc/5204k", "w");
    sendVa(p, procFile);
    fclose(procFile);
}

int main(int argc, char* argv[]){
    //open up logfile and pagemap - only used for checking kernel module
    // FILE *logfilep = fopen("log", "w");
    // fprintf(logfilep, "va,pa,latency\n");
    // char *pmstr = calloc(20, sizeof(char));
    // char *proc = "/proc/";
    // int pidNum = getpid();
    // char pidn[6];
    // sprintf(pidn, "%d", pidNum);
    // fprintf(stderr, "%s\n", pidn);
    // char *pagemap = "/pagemap";
    // strcat(pmstr, proc);
    // strcat(pmstr, pid);
    // strcat(pmstr, pagemap);
    // FILE *pagemapp = fopen(pmstr, "r");
    // int pagemapfd = fileno(pagemapp);

    //allocate in heap and make sure it has physical memory
    void* p = malloc(1024*1024*1024);
    for (int i = 0; i < 1024*1024*1024; i+= 4096){
        void * temp = p+i;
        ((int*)temp)[0] = 1;
    }

#ifdef basic
    FILE *procFile = fopen("/proc/5204k", "w");
    sendVa(p, procFile);
    fclose(procFile);
    // logToFile(test, pagemapfd, logfilep);
#endif

#ifdef improvement
    pthread_t kernelThread;
    pthread_create(&kernelThread, NULL, kernelCall, p);

    //In your memalloc.c, keep writing to all the pages in [0, 256MB] sequentially as fast
    //  as you can and writing to all the page [256MB, 512MB] every 1ms.
    pthread_t fast;
    pthread_create(&fast, NULL, fastWrite, p);

    int counter = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double start = ts.tv_sec+1e-9*ts.tv_nsec;
    double end = start+60.001;
    double now = start;
    pthread_t pid[60000]; //should start 60,000 threads

    while(now < end){ 
        clock_gettime(CLOCK_MONOTONIC, &ts); 
        now = ts.tv_sec+1e-9*ts.tv_nsec;

        //can only do increments of .025 reliably
        if (now > (start+counter*.025) ){
            pthread_create(&pid[counter], NULL, delayWrite, p);  
            counter++;
        }   
    }
    fprintf(stderr, "loops of 25ms in 60s: %d\n", counter);

    sleep(0.01); //wait for delayWrite thread
    continueThread = 0;
    sleep(3); //wait for kernel thread
#endif
    // fclose(pagemapp);
    // fclose(logfilep);
    free(p);
}



















/*
 * memalloc.c for cs 5204 lab 2 - Joshua Martin
 * 
 * Is there any need to keep the pointers around longer? 
 * Probably at least wait to free until the last one is done
 *
 * gcc pagemap.c pagemap.h memalloc.c -o memalloc
*/

/*DISABLE VIRTUAL ADDRESS SPACE RANDOMIZATION
sudo setarch --verbose --addr-no-randomize ./memalloc 1 log1.csv
*/

// int main(int argc, char* argv[]){


//     void *p;
//     pointerArray = calloc(16*1024*256, sizeof(void *));

//     //open up logfile and pagemap
//     FILE *logfilep = fopen(argv[2], "w");
//     fprintf(logfilep, "va,pa,latency\n");

//     char *pmstr = calloc(20, sizeof(char));
//     char *proc = "/proc/";
//     int pidNum = getpid();
//     char pid[6];
//     sprintf(pid, "%d", pidNum);
//     fprintf(stderr, "%s\n", pid);
//     char *pagemap = "/pagemap";
//     strcat(pmstr, proc);
//     strcat(pmstr, pid);
//     strcat(pmstr, pagemap);
//     FILE *pagemapp = fopen(pmstr, "r");
//     int pagemapfd = fileno(pagemapp);
    

//     if(strcmp(argv[1],"1")==0){
//         //first allocate 16GB via 4KB allocations
//         for (int a = 0; a < 16*1024*256; a++){
//             pointerArray[a] = calloc(1, 4*1024);
//             *(int*)pointerArray[a] = 68;
//             int check = *(int*)pointerArray[a];

            
//             //logToFile(pointerArray[a], pagemapfd, logfilep);
//         }     
//     }
//     else if(strcmp(argv[1],"2")==0){
//         //second allocate 16GB via 1GB allocations
//         void **pointerArray = calloc(16, sizeof(void *));
//         for (int i = 0; i < 16; i++){
//             pointerArray[i] = calloc(1, 1024*1024*1024);
//             *(int*)pointerArray[i] = 68;
//             int check = *(int*)pointerArray[i];
//             //logToFile(pointerArray[i], pagemapfd, logfilep); 
//         }
//     }
//     else if(strcmp(argv[1],"3")==0){
//         //third allocate 16GB in one allocation
//         void* p = calloc(16, 1024*1024*1024);
//         //logToFile(p, pagemapfd, logfilep);
//     }

//     fclose(pagemapp);
//     fclose(logfilep);

//     //just something to keep the process going
//     int var;
//     scanf("%d", &var);
// }















