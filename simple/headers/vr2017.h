#ifndef VR2017_H_
#define VR2017_H_

#include "mathfunc.h"
#include "convfunc.h"

int is_valid_delimiter(char* hex_string) { 
    char hex_characters[22] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F'}; 
    int valid_first_character = 0; 
    int valid_second_character = 0;
 
    // Check that the length of the string is 4 characters long, and starts with "0x":   
    if (hex_string[0] != '0' && hex_string[1] != 'x') { 
        return -2; 
    }

    if (length(hex_string) != 4) { 
        return -1; 
    }
        
    for (int i = 0; i < 22; i++) {         
        if (hex_string[2] == hex_characters[i]) { 
            valid_first_character = 1; 
        }
        if (hex_string[3] == hex_characters[i]) { 
            valid_second_character = 1; 
        }
    }

    if (!(valid_first_character && valid_second_character)) { 
        return 0; 
    }

    return 1;
}


char* compute_parity_byte(int length, char** binary_strings) {  \
    static char parity_byte[9];
    parity_byte[8] = 0;
    int count_ones[8] = {0, 0, 0, 0, 0, 0, 0, 0}; 

    for (int i = length - 3; i < length; i++) { 
        for (int j = 0; j < 8; j++) { 
            if (hex_to_binary(binary_strings[i])[j] == '1') { 
                count_ones[j] += 1; 
            }
        }
    }

    for (int i = 0; i < 8; i++) { 
        if (count_ones[i] % 2 == 0) { 
            parity_byte[i] = '0'; 
        } else { 
            parity_byte[i] = '1'; 
        }
    }

    return parity_byte; 
}


char* compute_parity_checksum(unsigned char b0, unsigned char b1, unsigned char b2, unsigned char swizzle)  { 
    // Converts input to binary string: 
    char binary_strings[4][8] = {"00000000", "00000000", "00000000", "00000000"};  
    int inputs[4] = {(int)b0, (int)b1, (int)b2, (int)swizzle}; 

    for (int i = 0; i < 4; i++) { 
        for (int j = 0; j < 8; j++) { 
            if ( (inputs[i] % 2) == 1) { 
                binary_strings[i][j] = '1'; 
            } 
            inputs[i] /= 2; 
        } 
    }

    static char parity_byte[9];
    parity_byte[8] = 0;
    int count_ones[8] = {0, 0, 0, 0, 0, 0, 0, 0}; 

    for (int i = 0; i < 4; i++) { 
        for (int j = 0; j < 8; j++) { 
            if (binary_strings[i][j] == '1') { 
                count_ones[j] += 1; 
            }
        }
    }

    for (int i = 0; i < 8; i++) { 
        if (count_ones[i] % 2 == 0) { 
            parity_byte[i] = '0'; 
        } else { 
            parity_byte[i] = '1'; 
        }
    }
    
    int size = 8; 
    for (int i = 0; i < 4; i++)
    {
        char temp = parity_byte[i];
        parity_byte[i] = parity_byte[size - 1 - i];
        parity_byte[size - 1 - i] = temp;
    }

    return parity_byte; 
}

char* compute_swizzle(int swizzle, unsigned char* b0_ptr, unsigned char* b1_ptr, unsigned char* b2_ptr) { 
    unsigned char temp; 
    switch(swizzle) { 
        case 1: 
            return "XYZ"; 
            break; 
        case 2:
            // b2 <--> b1 
            temp = *b2_ptr; 
            *b2_ptr = *b1_ptr; 
            *b1_ptr = temp; 
            return "XZY"; 
            break; 
        case 3:
            // b0 <--> b1 
            temp = *b0_ptr; 
            *b0_ptr = *b1_ptr; 
            *b1_ptr = temp; 
            return "YXZ"; 
            break; 
        case 4:
            // b2 <--> b0
            temp = *b2_ptr; 
            *b2_ptr = *b0_ptr; 
            *b0_ptr = temp; 

            // b1 <--> b2
            temp = *b1_ptr; 
            *b1_ptr = *b2_ptr; 
            *b2_ptr = temp; 
            return "YZX"; 
            break; 
        case 5:
            // b1 <--> b0
            temp = *b1_ptr; 
            *b1_ptr = *b0_ptr; 
            *b0_ptr = temp; 

            // b1 <--> b2
            temp = *b2_ptr; 
            *b2_ptr = *b1_ptr; 
            *b1_ptr = temp;
            return "ZXY"; 
            break; 
        case 6:
            // b0 <--> b2
            temp = *b2_ptr; 
            *b2_ptr = *b0_ptr; 
            *b0_ptr = temp;
            return "ZYX"; 
            break; 
        default: 
            return "FAILED"; 
            break; 
    }
}


#endif