#ifndef MATHFUNC_H_
#define MATHFUNC_H_

int power(int base, int exponent)
{
    int result = 1;

    for (; exponent > 0; exponent--) {
        result = result * base;
    }

    return result;
}

double average(int* array, double len) { 
    double sum = 0; 
    for (int i = 0; i < len; i++) { 
        sum += array[i]; 
    }
    return sum / len; 
}


int length(char* string) { 
    int length = 0; 
    while (*string) { 
        string++; 
        length++; 
    }

    return length;
}

#endif