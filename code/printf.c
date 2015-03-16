/*
 * Rishi Bedi
 * CS107E - Assignment 3
 * String Formatting Library
 *
 */

#include "timer.h"
#include "gpio.h"
#include "uart.h"
#include "printf.h"

typedef int bool;
enum {false, true};


static char hexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


/* 
 *  char * buf: buffer to write converted number into
 *  int n: max size of buffer
 *  int val: value to stringify and put in buf
 *  int width: min width of stringified val
 *  int base: 2 (binary), 10 (decimal), or 16 (hex)
 *  returns pointer to beginning of buffer
 *
 */
char* intobase(char *buf, int n, int val, int width, int base){
    char revStr[32];
    int strSize = 0;
    bool isNegDec = false;
    int i;
    if(val < 0 && base == 10){
        val = -val;
        isNegDec = true;
    }

    if(base==10){
        while(val > 0){
            revStr[strSize] = ('0' + val % 10);
            val = val / 10;
            strSize++;
        }  
    }
    else if(base==2){
        for(i=0; i < 32; i++){
            revStr[strSize] = ('0' + (val & 0x1));
            val = val >> 1;
            strSize++;
        }  
    }
    else if(base==16){
        for(i=0; i < 8; i++){
            revStr[strSize] = hexChars[val & 0xF];
            val = val >> 4;
            strSize++;
        }        
    }

    if(isNegDec){
        revStr[strSize] = '-';
        strSize++;
    }

    if(strSize >= width){
        // no need to pad with 0s
        // ensure we aren't trying to write more chars than exist in the binary string
        n = n > strSize ? strSize : (n-1);
        for(i=0; i < n; i++){
            buf[i] = revStr[strSize - 1 - i];
        }
        buf[n] = '\0';
    } 
    else{
        // must pad with 0s
        // ensure width is not longer than the size of the buffer
        width = width > (n-1) ? (n-1) : width;
        for(i = 0; i < (width - strSize); i++){
            buf[i] = '0';
        }
        for(i = 0; i < strSize; i++){
            buf[(width - strSize) + i] = revStr[strSize - 1 - i];
        }
        buf[width] = '\0';
    }
    
    return buf;

}


/* 
 * Converts int to string decimal equivalent,
 * using two's complement representation for the integer.
 * Same first four parameters as intobase.
 *
 */
char* intodec(char *buf, int n, int val, int width){
    return intobase(buf, n, val, width, 10);
}


/* 
 * Converts int to string binary equivalent,
 * using unsigned representation for the integer.
 * Same first four parameters as intobase.
 *
 */
char* intobin(char *buf, int n, int val, int width){
    return intobase(buf, n, val, width, 2);
}


/* 
 * Converts int to string hexadecimal equivalent,
 * using unsigned representation for the integer.
 * Same first four parameters as intobase.
 *
 */
char* intohex(char *buf, int n, int val, int width){
    return intobase(buf, n, val, width, 16);
}

/*
 * char *s: buffer to write into
 * int n: size of buffer to write into
 * const char *format: format string
 * var args: variables/literals used in format string
 * returns total size of expanded format string
 *
 */
int snprintf(char *s, int n, const char *format, ...){
    va_list ap;
    va_start(ap, format);
    int retVal = snprintf_ap(s, n, format, ap);
    va_end(ap);
    return retVal;
}

/**
 * const char *format: format string to get length of
 * va_list ap: list of variables/literals to expand in format
 * returns total length of expanded format string
 *
 */
int getLength(const char *format, va_list ap){

    int totalLength = 0;
    char nextChar;
    char tempBuf[33];
    char* tempBufPtr = tempBuf;
    while(*format){
        
        if(*format != '%'){
            totalLength++;
        }
        else{
            nextChar = *(++format);
            int width = 1;
            bool firstDigit = true;
            while(nextChar >= '0' && nextChar <= '9'){

               if(firstDigit){
                    width *= nextChar - '0';
                    firstDigit = false;
               } 
               else{
                    width *= 10;
                    width += nextChar - '0';
               }
               nextChar = *(++format);
            }
            char* stringToPrint;
            int intToPrint;
            switch(nextChar){
                case '%':
                    totalLength++;
                    break;
                case 'c':
                    totalLength++;
                    break;
                case 's':
                    stringToPrint = va_arg(ap, char*);
                    while(*stringToPrint){
                        totalLength++;
                        stringToPrint++;
                    }
                    break;
                case 'd':
                    intToPrint = va_arg(ap, int);
                    intodec(tempBuf, 16, intToPrint, width); 
                    while(*tempBufPtr){
                        tempBufPtr++;
                        totalLength++;
                    } 
                    tempBufPtr = tempBuf;
                    break;
                case 'x':
                    intToPrint = va_arg(ap, int);
                    intohex(tempBuf, 16, intToPrint, width); 
                    while(*tempBufPtr){
                        tempBufPtr++;
                        totalLength++;
                    } 
                    tempBufPtr = tempBuf;
                    break;
                case 'b':
                    intToPrint = va_arg(ap, int); 
                    intobin(tempBuf, 33, intToPrint, width); 
                    while(*tempBufPtr){
                        tempBufPtr++;
                        totalLength++;
                    } 
                    tempBufPtr = tempBuf;
                    break;
            }
        }
        format++;
    }
    totalLength++;
    return totalLength;
}


/*
 * char *s: buffer to write into
 * int n: size of buffer to write into
 * const char *format: format string
 * va_list ap: va_list representation of variables used in format string
 * returns total size of expanded format string
 *
 */
int snprintf_ap(char *s, int n, const char *format, va_list ap){

    char fullString[n];
    int curPos = 0;
    char nextChar;
    while(*format){
        
        if(*format != '%'){
            fullString[curPos] = *format;
            curPos++;
        }
        else{
            nextChar = *(++format);
            int width = 1;
            bool firstDigit = true;
            while(nextChar >= '0' && nextChar <= '9'){

               if(firstDigit){
                    width *= nextChar - '0';
                    firstDigit = false;
               } 
               else{
                    width *= 10;
                    width += nextChar - '0';
               }
               nextChar = *(++format);
            }
            char* stringToPrint;
            int intToPrint;
            char charToPrint;
            switch(nextChar){
                case '%':
                    fullString[curPos] = '%';
                    curPos++;
                    break;
                case 'c':
                    charToPrint = va_arg(ap, int);
                    fullString[curPos] = charToPrint;
                    curPos++;
                    break;
                case 's':
                    stringToPrint = va_arg(ap, char*);
                    while(*stringToPrint){
                        fullString[curPos] = *stringToPrint;
                        curPos++;
                        stringToPrint++;
                    }
                    break;
                case 'd': ;
                    char* decStartPos = fullString + curPos;
                    intToPrint = va_arg(ap, int);
                    intodec(decStartPos, 16, intToPrint, width); 
                    while(*decStartPos){
                        decStartPos++;
                        curPos++;
                    } 
                    break;
                case 'x': ;
                    char* hexStartPos = fullString + curPos;
                    intToPrint = va_arg(ap, int);
                    intohex(hexStartPos, 16, intToPrint, width); 
                    while(*hexStartPos){
                        hexStartPos++;
                        curPos++;
                    } 
                    break;
                case 'b': ;
                    char* binStartPos = fullString + curPos;
                    intToPrint = va_arg(ap, int); 
                    intobin(binStartPos, 33, intToPrint, width); 
                    while(*binStartPos){
                        binStartPos++;
                        curPos++;
                    } 
                    break;
            }
        }
        format++;
    }
    // add null-terminator to end of buffer
    fullString[curPos] = '\0';
    int bufPos = 0;
    char* fullStringPtr = fullString;
    while(*fullStringPtr && bufPos < (n-1)){
        s[bufPos] = *fullStringPtr;
        fullStringPtr++;
        bufPos++;
    }
     
    while(*fullStringPtr){
        bufPos++;
        fullStringPtr++;
    }
    
    s[bufPos] = '\0'; 
    bufPos++;
    return bufPos;
}

/*
 * const char* format: format string
 * var args: variables/literals used in format string
 * returns total number of chars written
 */

int printf ( const char * format, ... ){
   
    va_list ap;
    va_start(ap, format);

    int bufSize = getLength(format, ap);
    char buf[bufSize];
    buf[0] = '\0';
    snprintf_ap(buf, bufSize, format, ap);
    va_end(ap);
    
    char* bufToPrint = buf;
    while(*bufToPrint){
        uart_putc(*bufToPrint);
        bufToPrint++;
    }
    return bufSize;
    
}


char* console_printf ( const char * format, ... ){
   
    va_list ap;
    va_start(ap, format);

    int bufSize = getLength(format, ap);
    char buf[bufSize];
    buf[0] = '\0';
    snprintf_ap(buf, bufSize, format, ap);
    va_end(ap);
    
    return buf; 
}
