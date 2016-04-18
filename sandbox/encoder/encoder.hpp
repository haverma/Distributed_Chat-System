#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

namespace ENCODER
{
    void encrypt(const char* message, int messagelen, const char* key, char** result)
    {
        // int messagelen = strlen(message);
        int keylen = strlen(key);
        
        for (int i=0; i<messagelen+1; i++)
            (*result)[i] = '\0';
         for (int i=0; i<messagelen; i++)
        {
            (*result)[i] = message[i];
             for (int j=0; j<keylen; j++)
            {
                if (j < keylen/2)
                {
                    if (j%2 == 0) (*result)[i] += key[i] / 3;
                    else (*result)[i] -= key[i] / 2;
                }
                else
                {
                    if (j%2 == 0) (*result)[i] += key[i];
                    else (*result)[i] -= key[i];
                }
            }
        }
    }
     void decrypt(const char* message, int messagelen, const char* key, char** result)
    {
        // int messagelen = strlen(message);
        int keylen = strlen(key);
        
        for (int i=0; i<messagelen+1; i++)
            (*result)[i] = '\0';
         for (int i=0; i<messagelen; i++)
        {
            (*result)[i] = message[i];
             for (int j=0; j<keylen; j++)
            {
                if (j < keylen/2)
                {
                    if (j%2 == 0) (*result)[i] -= key[i] / 3;
                    else (*result)[i] += key[i] / 2;
                }
                else
                {
                    if (j%2 == 0) (*result)[i] -= key[i];
                    else (*result)[i] += key[i];
                }
            }
        }
    }
} // end namespace ENCODER


