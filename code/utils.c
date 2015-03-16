#include "utils.h"

unsigned __aeabi_uidiv(unsigned numerator, unsigned denominator){    
    int quotient = 0;
    while(numerator > denominator){
        quotient++;
        numerator -= denominator;
    }
    return quotient;
}


int strcmp(char* str1, char* str2){
    while(*str1 && *str2){
        if(*str1 == *str2){
            str1++;
            str2++;
        }
        else{
            return (*str2 - *str1);
        }
    }
    return (*str2 - *str1);
} 

char* strcpy(char* destination, char* source){
    while(*source){
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0';
    return destination;
}


volatile char* imgcpy(volatile char* destination, char* source, int num_bytes){
    int bytes_copied = 0;
    while(bytes_copied <= num_bytes){
        *destination = *source;
        destination++;
        source++;
        bytes_copied++;
    }
    return destination;
}
