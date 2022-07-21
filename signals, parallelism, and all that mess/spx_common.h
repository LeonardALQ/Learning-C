#ifndef SPX_COMMON_H
#define SPX_COMMON_H

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>

#define EXCHANGE_FIFO "/tmp/spx_exchange_" 
#define TRADER_FIFO "/tmp/spx_trader_"
#define MAX_PIPE 20
#define MAX_MESSAGE 60
#define FEE_PERCENTAGE 1

char* int_to_string(int integer) { 
	int digits; 
	char* string; 
	
	// log10(int) gives number of digits
	if (integer == 0) { 
		digits = 1;
	} else { 
		digits = floor(log10(integer) + 1); 
	}

	// + 1 for null character.  
	string = calloc(digits + 1, sizeof(char));

	sprintf(string, "%d", integer);

	return string; 
}

char* get_exchange_FIFO(int id) { 
	char* id_string = int_to_string(id); 
	char* exchange_fifo = calloc(MAX_PIPE + 
		strlen(id_string), sizeof(char));
	
	strcpy(exchange_fifo, EXCHANGE_FIFO); 
	strcat(exchange_fifo, id_string); 
	free(id_string); 

	return exchange_fifo; 
}

char* get_trader_FIFO(int id) { 
	char* id_string = int_to_string(id); 
	char* trader_fifo = calloc(MAX_PIPE + 
		strlen(id_string), sizeof(char));
	
	strcpy(trader_fifo, TRADER_FIFO); 
	strcat(trader_fifo, id_string); 
	free(id_string); 

	return trader_fifo; 
}


#endif
