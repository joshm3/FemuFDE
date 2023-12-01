/*
 * memalloc.c for cs 5204 lab 2 part 2- Joshua Martin
 * 
 * gcc memalloc.c pagemap.c -o memalloc 

 * gcc memalloc.c -o memalloc
*/

/*DISABLE VIRTUAL ADDRESS SPACE RANDOMIZATION
sudo setarch --verbose --addr-no-randomize ./memalloc

To run 1000 times to create data for CDF

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

static int continueThread = 1;

void* fastWrite(void* p){
    while(continueThread){
        for (int i = 0; i < 256*1024*1024; i+= 4096){
            void * temp = p+i;
            ((int*)temp)[0] = 9;
        }
    }
}

void* delayWrite(void* p){
    //each loop takes at least 90ms cant do every ms?
    //not WORKING!!!!
    for (int i = 256*1024*1024; i < 512*1024*1024; i+= 4096){
        void * temp = p+i;
        ((int*)temp)[0] = 9;
    }
}

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
    // char pid[6];
    // sprintf(pid, "%d", pidNum);
    // fprintf(stderr, "%s\n", pid);
    // char *pagemap = "/pagemap";
    // strcat(pmstr, proc);
    // strcat(pmstr, pid);
    // strcat(pmstr, pagemap);
    // FILE *pagemapp = fopen(pmstr, "r");
    // int pagemapfd = fileno(pagemapp);

    //allocate in heap and make sure it has physical memory
    void* p = malloc(1024*1024*1024);
    ((int*)p)[0] = 68;
    int check = ((int*)p)[0];
    
    pthread_t kernelThread;
    pthread_create(&kernelThread, NULL, kernelCall, p);
    // logToFile(test, pagemapfd, logfilep);
    
    //In your memalloc.c, keep writing to all the pages in [0, 256MB] sequentially as fast
    //  as you can and writing to all the page [256MB, 512MB] every 1ms.
    
    pthread_t fast;
    pthread_create(&fast, NULL, fastWrite, p);
    //should be 60*1000
    int counter = 0;

    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    double last = start.tv_sec+1e-9*start.tv_nsec;
    while( (now.tv_sec+1e-9*now.tv_nsec) < (start.tv_sec+1e-9*start.tv_nsec+60.001) ){
        //fprintf(stderr, "%f\n", newTime);
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec+1e-9*now.tv_nsec > last + 0.001){
            last = now.tv_sec+1e-9*now.tv_nsec;
            //counter++;
            pthread_t delay;
            pthread_create(&delay, NULL, delayWrite, p);  
        }   
    }
    //fprintf(stderr, "counter %d\n", counter);

    continueThread = 0;

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















