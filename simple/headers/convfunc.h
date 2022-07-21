#ifndef CONVFUNC_H_
#define CONVFUNC_H_

#include "mathfunc.h"

int hex_to_decimal(char* hex_string) {    
    int ascii; 
    int decimal = 0; 
    int len = length(hex_string) - 3; 

    for (int i = 2; hex_string[i] != 0; i++) { 
        if (hex_string[i] >= '0' && hex_string[i] <= '9') { 
            ascii = hex_string[i] - 48; 
        }
        else if (hex_string[i] >= 'a' && hex_string[i] <= 'f') { 
            ascii = hex_string[i] - 97 + 10; 
        }
        else if (hex_string[i] >= 'A' && hex_string[i] <= 'F') { 
            ascii = hex_string[i] - 65 + 10; 
        }
        decimal += ascii * power(16, len);
        len--; 
    }

    return decimal; 
}

char* hex_to_binary(char* hex_string) {
    char* binary[22] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1010", "1011", "1011", "1100", "1100", "1101", "1101", "1110", "1110", "1111", "1111"};    
    char hex_characters[22] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F'};
    static char converted_number[9]; 
    converted_number[8] = '\0';      

    for (int i = 2; hex_string[i] != '\0'; i++) {  
        for (int j = 0; j < 22; j++) {
            if (hex_string[i] == hex_characters[j]) {
                converted_number[(i - 2)*4] = binary[j][0]; 
                converted_number[(i - 2)*4 + 1] = binary[j][1]; 
                converted_number[(i - 2)*4 + 2] = binary[j][2]; 
                converted_number[(i - 2)*4 + 3] = binary[j][3]; 
            }
        }
    }

    return converted_number;  
}


int binary_to_decimal(char* binary_string) { 
    int len = length(binary_string); 
    int results = 0; 
    int j = 0; 

    for (int i = len - 1; i >= 0; i--) { 
        if (binary_string[i] == '1')
            results += power(2, j); 
        j++; 
    }

    return results; 
}

#endif