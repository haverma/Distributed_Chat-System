#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "encoder.hpp"

int main (int argc, char* argv[])
{

    char original[] = "0 ~- {this is a test message}";
    char key[] = "test key for encrypting 1234";
    
    char* encrypted = new char[1024];
    char* decrypted = new char[1024];

    ENCODER::encrypt(original, key, &encrypted);
    ENCODER::decrypt(encrypted, key, &decrypted);

    
    printf("original:  %s\n", original);
    printf("encrypted: %s\n", encrypted);
    printf("decrypted: %s\n", decrypted);

    delete [] encrypted;
    delete [] decrypted;

    return 0;
}
