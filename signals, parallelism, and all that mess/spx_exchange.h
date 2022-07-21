#ifndef SPX_EXCHANGE_H
#define SPX_EXCHANGE_H

#include "spx_common.h"

#define LOG_PREFIX "[SPX]"
#define MAX_ACCEPTED 15
#define MAX_QTY 6

typedef struct {
    int time;
    int price;
} time;

typedef struct { 
    int old_order_id;
    int old_trader_id;  
    int new_order_id; 
    int new_trader_id;
    int value; 
    int fee;
} matches; 

#endif
