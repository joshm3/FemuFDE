/*
 * memalloc.c for cs 5204 lab 2 part 2- Joshua Martin
 * 
 * gcc memalloc.c -o memalloc
*/

/*DISABLE VIRTUAL ADDRESS SPACE RANDOMIZATION
sudo setarch --verbose --addr-no-randomize ./memalloc
*/

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>

//part 2 only uses /proc/5204(k)
//#include "pagemap.h"


void **pointerArray;

void sendVa(void* va, FILE* procFile){
    fprintf(stderr, "%lx\n", (unsigned long)va);
    fprintf(procFile, "%p", va);
}

int main(int argc, char* argv[]){
    void* p = calloc(1, 1024*1024*1024);
    FILE *procFile = fopen("/proc/5204k", "w");
    
    sendVa(p, procFile);


    free(p);
    fclose(procFile);
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















