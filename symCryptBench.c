/*
symCryptBench.c is the primary file for measuring all encryption from user space
This is all done in the same file rather than just using each tools respective 
benchmark to maintain consistency and provide transparency.

Author: Joshua Martin

current support for Crypto API (kernel) , openssl, tomcrypt

Just doing XTS-AES for now with key size 256*2=512 (luks default)

sudo apt install libtomcrypt-dev cryptsetup-bin libssl-dev

gcc symCryptBench.c -o scb -lcrypto -ltomcrypt
*/

#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<time.h>

#include<tomcrypt.h>
#include<sys/random.h>
#include<openssl/evp.h>

//test variables
#define ENC_PER_TEST 100000 //recommended encryptions: XXXX
#define ENCRYPTION_SIZE 4096 //(in bytes) recommended value is PAGE_SIZE

//global variables for use across tests (/8 is just bits to bytes)
static uint8_t key512[512/8]; //this is just because xts requires 2 keys
static uint8_t tweak[128/8];
static uint8_t input[ENCRYPTION_SIZE];
static uint8_t output[ENCRYPTION_SIZE];

//helper function to print time
void printTime(struct timespec* start, struct timespec* end){
    //overall speed
    //double time = (end->tv_sec + end->tv_nsec*0.000000001) - (start->tv_sec + start->tv_nsec*0.000000001); //Seconds
    // double size = ENCRYPTION_SIZE*ENC_PER_TEST/1024/1024; //MB
    // printf("%lf (MB/S)\n", size/time);

    //time for one page encryption
    double time = (end->tv_sec*1000000000 + end->tv_nsec) - (start->tv_sec*1000000000 + start->tv_nsec); //nanoseconds
    printf("%lf (microseconds)\n", time/1000/ENC_PER_TEST);
}

void kernelCryptoApi(){
    FILE *fp;
    char line[1024];

    /* Open the command for reading. */
    fp = popen("cryptsetup benchmark", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    int lineCount = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (lineCount == 18){ //line of aes-xts with key size 512
            double encrypt, decrypt;
            if (sscanf(line, "%s %s %lf %s %lf %s\n", line, line, &encrypt, line, &decrypt, line) > 0){
                // convert MiB/s to 4k/s, then s/4k, then microseconds per kilobyte
                encrypt = (1/((1024/4)*encrypt)) * 1000000;
                decrypt = (1/((1024/4)*decrypt)) * 1000000;
                printf("Encryption Kernel CryptoAPI: %lf (microseconds)\n", encrypt);
                printf("Decryption Kernel CryptoAPI: %lf (microseconds)\n", decrypt);
            }
            break;
        }
        lineCount++;
    }

    /* close */
    pclose(fp);
}

//probably the most common linux crypto api
void opensslF(){
    unsigned char ciphertext[128];
    unsigned char decryptedtext[128];
    struct timespec start, end;
    int outl;
    uint8_t* cipherEnd;
    EVP_CIPHER_CTX *ctx;
    
    // encrypt - no padding necessary
    ctx = EVP_CIPHER_CTX_new();
    outl = sizeof(output);
    EVP_EncryptInit_ex(ctx, EVP_aes_256_xts(), NULL, key512, tweak); //before or after
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int c = 0; c < ENC_PER_TEST; c++){ //loop time should be negligable
        EVP_EncryptUpdate(ctx, output, &outl, input, sizeof(input));
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    EVP_CIPHER_CTX_free(ctx);
    printf("Encryption Openssl: ");
    printTime(&start, &end);
    
    // decrypt - no padding necessary
    ctx = EVP_CIPHER_CTX_new();
    outl = sizeof(output);
    EVP_DecryptInit_ex(ctx, EVP_aes_256_xts(), NULL, key512, tweak);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int c = 0; c < ENC_PER_TEST; c++){ 
        EVP_DecryptUpdate(ctx, output, &outl, input, sizeof(input));
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    EVP_CIPHER_CTX_free(ctx);
    printf("Decryption Openssl: ");
    printTime(&start, &end);
}

//usefull user crypto api
void tomCryptF(){
    //setup
    struct timespec start, end;
    symmetric_xts xts;

    xts_start(register_cipher(&aes_desc), key512, (key512+32), 32, 0, &xts);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int c = 0; c < ENC_PER_TEST; c++){
        xts_encrypt(input, sizeof(input), output, tweak, &xts);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    printf("Encryption Tomcrypt: ");
    printTime(&start, &end);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int c = 0; c < ENC_PER_TEST; c++){
        xts_decrypt(input, sizeof(input), output, tweak, &xts);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    printf("Decryption Tomcrypt: ");
    printTime(&start, &end);

    xts_done(&xts);
}

int main(int argc, char** argv){
    //setup keys, iv, and plaintext
    getrandom(key512, sizeof(key512), GRND_NONBLOCK);
    getrandom(tweak, sizeof(tweak), GRND_NONBLOCK);
    getrandom(input, sizeof(input), GRND_NONBLOCK);

    printf("Benchmark for aes 256 xts\n");

    //handle timing and printing on their own
    opensslF();
    tomCryptF();
    kernelCryptoApi();
}