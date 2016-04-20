#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "encoder.hpp"

int main (int argc, char* argv[])
{

    char original[] = "{this is a test}\0{message}";
    char key[] = "team BLOOPERS";
    
    char* encrypted = new char[1024];
    char* decrypted = new char[1024];

    
    // encrypt and decrypt message
    ENCODER::encrypt(original, 1024, key, &encrypted);
    ENCODER::decrypt(encrypted, 1024, key, &decrypted);

    // print original first and second null terminated strings
    printf("\noriginal1:  %s\n", original);
    printf("original2:  %s\n", &(original)[17]);
    
    // print encrypted string null terminations lost
    printf("\nencrypted message: \n");
    printf("%s\n", encrypted);
    
    // print decripted first and second null terminated strings
    printf("\ndecrypted1: %s\n", decrypted);
    printf("decrypted2: %s\n", &(decrypted)[17]);

    delete [] encrypted;
    delete [] decrypted;

    return 0;
}
