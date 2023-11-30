/* 
 * File pagemap.c
 * User-space page table walker
 *
 * Written by Huaicheng Li <huaicheng@cs.uchicago.edu>
 *
 * Modified by Joshua Martin joshm3
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>

// #define DEBUG_PAGEMAP

#define FILE_NAME_SZ_MAX    128

/* pagemap entry size in bytes */
#define PAGEMAP_ENTRY_SZ    8

#define PAGE_SHIFT          12
#define PAGE_SIZE           (1 << PAGE_SHIFT)
#define PFN_PRESENT         (1ull << 63)
#define PFN_PFN             ((1ull << 55) - 1)

uint32_t page_offset(uint32_t addr)
{
    return addr & ((1 << PAGE_SHIFT) - 1);
}

/* virtual addr to frame number, i.e. GVA -> GFN, or HVA -> HFN */
uint64_t pagemap_va2pfn(int pagemapfd, void *addr)
{
    uint64_t pme, pfn;
    size_t offset;
    int nbytes;
    offset = (uintptr_t)addr / PAGE_SIZE * PAGEMAP_ENTRY_SZ;
    //offset = 0x2ae54ce610;
    //fprintf(stderr, "offset:%lx\n", offset);
    nbytes = pread(pagemapfd, &pme, 8, offset);
    if (nbytes != 8) {
        printf("Coperd,%s,failed to read 8B for addr translation\n", __func__);
        return -1;
    }

    if (!(pme & PFN_PRESENT))
        return -1;

    pfn = pme & PFN_PFN;

    // fprintf(stderr, "pme: %lx\n", pme);
    // fprintf(stderr, "pme: %llx\n", PFN_PFN);
    // fprintf(stderr, "pme: %lx\n", pfn);

#ifdef DEBUG_PAGEMAP
    printf("Coperd,pfn:%lx\n", pfn);
#endif
    return pfn;
}

/* virtual addr to physical addr, i.e., GVA -> GPA, or HVA -> HPA */
uint64_t pagemap_va2pa(int pagemapfd, void *addr)
{
    //get physical frame number
    uint64_t pfn = pagemap_va2pfn(pagemapfd, addr);
    if (pfn == (uint64_t)(-1)) {
        printf("Coperd,ERROR,no physical addr mapped!\n");
        exit(1);
    }

#ifdef DEBUG_PAGEMAP
    printf("Coperd,%s,pfn:0x%lx\n", __func__, pfn);
#endif
    return (pfn << PAGE_SHIFT) | page_offset((uint64_t)addr);
}


//Added functionality to take va, convert to pa, time the latency, and log it all
int logToFile(void* va, int pagemapfd, FILE* logfilep){
//*********NOT SURE IF THIS IS WHERE THE LATENCY SHOULD BE MEASURED???
    //start time
    clock_t t; 
    t = clock(); 

    //get pa from va
    uint64_t pa = pagemap_va2pa(pagemapfd, va);

    //end time
    t = clock() - t; 
    double time = ((double)t)/CLOCKS_PER_SEC;
    
    //log it all to logfilefd
    fprintf(logfilep, "%p,%"PRIx64",%lf\n", va, pa, time);

    return 1;
}

// int main(int argc, char** argv){
//     FILE *logfilep = fopen(argv[2], "w");
//     fprintf(logfilep, "virtual address (hex), physical address (hex), latency (seconds)\n");

//     FILE *pagemapp = fopen(argv[1], "r");
//     int pagemapfd = fileno(pagemapp);

//     FILE *templog = fopen("log", "r");
//     char line[100]; 
 
//     while (fgets(line, 100, templog) != NULL) { 
//         strtok(line, "\n");
//         void **p = calloc(1, sizeof(void*));
//         sscanf(line, "%p", p);
//         void *va = *p;
        
//         logToFile(va, pagemapfd, logfilep);
//     } 
 
//     fclose(templog);

//     fclose(pagemapp);
//     fclose(logfilep);
// }
