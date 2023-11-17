/**
 * Currently, FEMU does not actually seem to send real data to the SSD
 * So, we will just encrypt random data, and investigate the time it takes
 * for data to run through encryption/decryption
 * 
 * First adoption will be to ecrypt with AES-ECB
*/

#include <stdint.h>
#include "tomcrypt.h"
#include <unistd.h>

#define NUM_TRIALS 10000000


/**
 * Will return the time it takes to perform encryption
*/
double sed_aes_ebc_encrypt(){

    const unsigned char pt[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01};
    unsigned char * ct = malloc(sizeof(char) * 16);
    const unsigned char key[16] ={0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01};
    symmetric_key skey;
    int keylen = 16;
    aes_keysize(&keylen);
    aes_setup(key, 16, 10, &skey);

    clock_t t;
    t = clock();

    for(int i = 0; i < 256; i++){
        aes_ecb_encrypt(pt, ct, &skey);
    }
    t = clock() - t;

    return t/(double)CLOCKS_PER_SEC * ((double)1000000/NUM_TRIALS); //convert to us
} 

/**
 * Will return the time it takes to perform decryption
*/
double sed_aes_ebc_decrypt(){

    const unsigned char pt[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01};
    unsigned char * ct = malloc(sizeof(char) * 16);
    const unsigned char key[16] ={0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01};
    symmetric_key skey;
    int keylen = 16;
    aes_keysize(&keylen);
    aes_setup(key, 16, 10, &skey);

    clock_t t;
    t = clock();

    int counter = 0;
    while(counter < NUM_TRIALS){
        aes_ecb_decrypt(pt, ct, &skey);
        counter++;
    }
    t = clock() - t;

    return t/(double)CLOCKS_PER_SEC * ((double)1000000/NUM_TRIALS); //convert to us

}

int main(){
    double ebc_encrypt_time = sed_aes_ebc_encrypt();
    double ebc_decrypt_time = sed_aes_ebc_decrypt();
    printf("Time of AES-EBC encryption: %lfus, \n", ebc_encrypt_time);
    printf("Time of AES-EBC decryption: %lfus, \n", ebc_decrypt_time);
}


