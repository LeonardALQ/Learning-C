#include "spx_exchange.h"
#include <limits.h>
#include <poll.h>

pid_t sender_pid;
int signal_recieved = 0;
int pipe_closed = 0;

char* read_file(char* file_name) { 
	FILE* products_file;
	char chr; 
	int file_size;
	char* products;

	products_file = fopen(file_name, "r");
	if (products_file == NULL) 
		return "File cannot be opened.\n";

	// Get File Length to allocate array: 
	fseek(products_file, 0, SEEK_END);
	file_size = ftell(products_file);
	fseek(products_file, 0, SEEK_SET);

	products = (char*) calloc(file_size + 1, sizeof(char));
	// Set the last character as a null for strtol() function 
	products[file_size] = 0;

	// Save file content into array. 
	// Change new line characters to space so it's easier to print. 
	for (int i = 0; i < file_size; i++) {
		chr = fgetc(products_file);
		if (chr == '\n') { 
			products[i] = ' ';
		} else { 
			products[i] = chr;
		}
	} fclose(products_file);

	// Ensure there are no extra space at the end
	if (products[file_size - 1] == ' ') { 
		products[file_size - 1] = 0;
	}

	return products;
}

void store_products(char* file_content, char*** products_list,
	int no_products) {
	char* products; 
	char* product; 
	strtol(file_content, &products, 10);

	strsep(&products, " ");
	for (int i = 0; i < no_products; i++) { 
		product = strsep(&products, " "); 
		(*products_list)[i] = product;
	}
}

int get_number_of_products(char* file_content) { 
	int number_of_products; 
	char* ptr; 

	number_of_products = strtol(file_content, &ptr, 10);

	return number_of_products; 
}

void create_FIFO(int id) { 
	char* trader_fifo; 
	char* exchange_fifo;

	// TRADER FIFO (TRADER -> EXCHANGE)
	trader_fifo = get_trader_FIFO(id); 
	if (mkfifo(trader_fifo, 0666) == -1 && errno != EEXIST) { 
		printf("Could not create Trader FIFO.\n"); 
		return; 
	} else { 
		printf("%s Created FIFO %s\n", LOG_PREFIX, trader_fifo);
	}

	// EXCHANGE FIFO (EXCHANGE -> TRADER) 
	exchange_fifo = get_exchange_FIFO(id); 
	if (mkfifo(exchange_fifo, 0666) == -1 && errno != EEXIST) { 
		printf("Could not create Trader FIFO.\n"); 
		return; 
	} else { 
		printf("%s Created FIFO %s\n", LOG_PREFIX, exchange_fifo);
	}

	free(trader_fifo);
	free(exchange_fifo);  
}

void connect_FIFO(int id, int* exchange_fd, int* trader_fd) { 
	char* exchange_fifo = get_exchange_FIFO(id); 
	char* trader_fifo = get_trader_FIFO(id); 

	exchange_fd[id] = open(exchange_fifo, O_WRONLY); 
	printf("%s Connected to %s\n", LOG_PREFIX, exchange_fifo); 
	trader_fd[id] = open(trader_fifo, O_RDONLY);
	printf("%s Connected to %s\n", LOG_PREFIX, trader_fifo); 

	free(trader_fifo);
	free(exchange_fifo);
}

void execute_trader(char* trader, int id, pid_t* pids, 
	int* exchange_fd, int* trader_fd) { 
	char* id_string; 
	pid_t fork_pid; 
	pid_t exec_pid; 
	
	fork_pid = fork(); 
	if (fork_pid < 0) { 
		perror("Fork has failed."); 
		exit(EXIT_FAILURE);
	} else if (fork_pid == 0) { 
		id_string = int_to_string(id);	
		char* args[3] = {trader, id_string, NULL};

		printf("%s Starting trader %d (%s)\n", LOG_PREFIX, id, trader);
		exec_pid = execv(trader, args);
		if (exec_pid < 0) { 
			free(id_string);
			perror("Trader execution has failed"); 
			exit(EXIT_FAILURE);
		}
	} else { 
		connect_FIFO(id, exchange_fd, trader_fd);
		pids[id] = fork_pid;
	}
}

void market_open(int id, pid_t pid, int* exchange_fd) { 
	int written;
	char* msg = "MARKET OPEN;";
	int success; 
	
	written = write(exchange_fd[id], msg, strlen(msg));
	if (written != strlen(msg)) { 
		perror("Write FAILED\n"); 
		exit(EXIT_FAILURE);
	}
	
	success = kill(pid, SIGUSR1);
	if (success == -1) { 
		perror("SIGNAL WAS NOT SENT"); 
		exit(EXIT_FAILURE);
	}
}

void announce_order(char* order, int fd, pid_t pid) { 
	char announcement[MAX_MESSAGE];
	memset(announcement, 0, MAX_MESSAGE);
	strcpy(announcement, "MARKET ");
	strcat(announcement, order);
	announcement[strlen(announcement)] = ';';

	write(fd, announcement, strlen(announcement));
	kill(pid, SIGUSR1);
}

void recieve_signal(int sig, siginfo_t* info, void* context) { 
	signal_recieved = 1;
	sender_pid = info->si_pid;
}

void close_trader(int sig, siginfo_t* info, void* context) { 
	pipe_closed = 1;
	sender_pid = info->si_pid;
}

void store_order(char* order, int* n, char*** book,
	int** t_array, int* t) { 
	char* copy = strdup(order); 
	(*t)++;
	(*n)++; 
	(*book) = realloc((*book), sizeof(char*) * (*n)); 
	(*t_array) = realloc((*t_array), sizeof(char*) * (*n));
	(*book)[(*n) - 1] = copy;
	(*t_array)[(*n) - 1] = (*t);
}

void respond_to_order(char* order, int fd, pid_t pid) { 
	char* order_copy; 
	char* og_address; 
	char response[MAX_MESSAGE];  
	char* command; 
	char* order_id;
	int written; 

	memset(response, 0, MAX_MESSAGE);

	// Make a copy of message to slice: 
	order_copy = strdup(order); 
	og_address = order_copy; 
	command = strsep(&order_copy, " "); 

	if (!strcmp(command, "BUY") || !strcmp(command, "SELL")) { 
		order_id = strsep(&order_copy, " "); 

		strcpy(response, "ACCEPTED "); 
		strcat(response, order_id);
	} else if (!strcmp(command, "AMEND")) { 
		order_id = strsep(&order_copy, " ");

		strcpy(response, "AMENDED "); 
		strcat(response, order_id);
	} else if (!strcmp(command, "CANCEL")) { 
		order_id = strsep(&order_copy, " ");

		strcpy(response, "CANCELLED "); 
		strcat(response, order_id);
	} else { 
		return;
	}

	response[strlen(response)] = ';';
	free(og_address);
	written = write(fd, response, strlen(response));
	if (written != strlen(response)) { 
		printf("respond_to_order has FAILED\n"); 
		exit(EXIT_FAILURE);
	}
	kill(pid, SIGUSR1);
}

int find_sender_id(int argc, pid_t* traders) { 
	int sender_id; 

	for (int i = 2; i < argc; i++) { 
		if (traders[i - 2] == sender_pid) { 
			sender_id = i - 2;
		}
	}

	return sender_id; 
}

int read_order(int id, int* trader_fd, char* order) { 
	memset(order, 0, MAX_MESSAGE);
	char chr; 
	int k = 0;

	while (chr != ';' && k != MAX_MESSAGE - 1) { 
		read(trader_fd[id], &chr, sizeof(char)); 
		order[k++] = chr;
	} order[k] = 0;

	// Buy Order - 500 
	// Sell Order - 400 
	// Neither - 300
	int type = 300; 
	char* command; 
	char* order_copy; 
	char* og_address; 
	order_copy = strdup(order);
	og_address = order_copy; 
	command = strsep(&order_copy, " "); 
	if (!strcmp(command, "BUY")) { 
		type = 500; 
	} else if (!strcmp(command, "SELL")) { 
		type = 400; 
	}
	free(og_address); 

	return type; 
}

void remove_order_id(int** group, int group_size, int id_pos) { 
	// Copy list: 
	int* group_copy = calloc(group_size, sizeof(int)); 
	int j = 0;

	for (int i = 0; i < group_size; i++) { 
		if (i == id_pos) { 
			continue; 
		}
		group_copy[j++] = (*group)[i]; 
	}

	// Realloc 
	(*group) = realloc((*group), sizeof(int) * group_size); 

	for (int i = 0; i < group_size; i++) { 
		(*group)[i] = group_copy[i]; 
	}

	free(group_copy);
}

void remove_order(char*** order_book, int* size, char* order,
	int** group, int** t_array, int t_value) { 

	char** book_copy = calloc((*size), sizeof(char*)); 
	int* t_copy = calloc((*size), sizeof(int));
	int j = 0; 
	int order_i = 0;

	// Make a copy of the list without the order 
	for (int i = 0; i < (*size); i++) { 
		if (!strcmp(order, (*order_book)[i])) { 
			order_i = i; 
			continue; 
		}
		book_copy[j++] = (*order_book)[i]; 
	}

	j = 0;
	for (int i = 0; i < (*size); i++) { 
		if ((*t_array)[i] == t_value) { 
			continue; 
		}
		t_copy[j++] = (*t_array)[i];
	}
	
	(*size)--;
	(*order_book) = realloc((*order_book), sizeof(char*) * (*size));
	(*t_array) = realloc((*t_array), sizeof(int) * (*size));

	// Copy over content: 
	for (int i = 0; i < (*size); i++) { 
		(*order_book)[i] = book_copy[i]; 
	} 
	for (int i = 0; i < (*size); i++) { 
		(*t_array)[i] = t_copy[i];
	}

	remove_order_id(group, *size, order_i); 

	free(book_copy); 
}

void amend_order(char*** order_book, int size,
	char* order, char* new_order) { 
	int order_i; 
	char* amended = strdup(new_order); 

	
	for (int i = 0; i < size; i++) { 
		if (!strcmp((*order_book)[i], order)) { 
			order_i = i; 
			break; 
		}
	}

	free((*order_book)[order_i]); 
	(*order_book)[order_i] = amended; 
	
}

void add_product_to_trader(int id, char** products,
	char* product, int*** trader_holdings, int no_products, 
	int amount, int*** trader_money_spent, int cost, int fees) { 
		for (int i = 0; i < no_products; i++) { 
			if (!strcmp(product, products[i])) { 
				(*trader_holdings)[id][i] += amount; 
				(*trader_money_spent)[id][i] -= ((cost*amount) - fees);
			}
		}
	}

void remove_product_from_trader(int id, char** products,
	char* product, int*** trader_holdings, int no_products, 
	int amount, int*** trader_money_spent, int cost, int fees) { 
		for (int i = 0; i < no_products; i++) { 
			if (!strcmp(product, products[i])) { 
				(*trader_holdings)[id][i] -= amount; 
				(*trader_money_spent)[id][i] += ((cost*amount) - fees); 
			}
		}
	}

int increasing_cmp(const void * p1, const void * p2) {
	return (*(int*)p1 - *(int*)p2);
}

int decreasing_cmp(const void * p1, const void * p2) {
	return (*(int*)p2 - *(int*)p1);
}

int increasing_map(const void *a, const void *b) {
    return ((time *)a)->price - ((time *)b)->price;
}

int decreasing_map(const void *a, const void *b) {
    return ((time *)b)->price - ((time *)a)->price;
}

void sort_time(int** temp, int k, time** time_map) { 
	time* tmp = calloc(k, sizeof(time));
	int* values = calloc(k, sizeof(int));
	for (int i = 0; i < k; i++) { 
		values[i] = (*time_map)[(*temp)[i]].time;
	}

	for (int i = 0; i < k; i++) { 
		tmp[i].time = (*temp)[i]; 
		tmp[i].price = values[i];
	}

	qsort(tmp, k, sizeof(int), increasing_map);
	for (int i = 0; i < k; i++) { 
		values[i] = (*time_map)[tmp[i].time].time;
	}

	for (int i = 0; i < k; i++) { 
		(*time_map)[(*temp)[i]].time = values[i];
	} 

	free(tmp);
	free(values); 
}

time* sort_book(char*** order_book, int n, int increasing) { 
	int* prices = calloc(n, sizeof(int));

	// used to store temp values in it for sorting 
	int* temp = calloc(n, sizeof(int));
	int k = 1; 

	// Time map sorts the order of the book based on prices first:
	time* time_map = calloc(n, sizeof(time));

	char* order;
	char* og_address;
	char* price; 

	// Get all prices and ids:
	for (int i = 0; i < n; i++) { 
		order = strdup((*order_book)[i]);
		og_address = order; 

		strsep(&order, " "); 
		strsep(&order, " ");
		strsep(&order, " "); 
		strsep(&order, " "); 
		price = strsep(&order, " ");

		prices[i] = atoi(price); 

		free(og_address);
	}

	for (int i = 0; i < n; i++) { 
		time_map[i].time = i; 
		time_map[i].price = prices[i];
	}

	if (increasing) {
		qsort(prices, n, sizeof(int), increasing_cmp); 
		qsort(time_map, n, sizeof(time), increasing_map);
	} else { 
		qsort(prices, n, sizeof(int), decreasing_cmp); 
		qsort(time_map, n, sizeof(time), decreasing_map);
	}

	// Compares all values, and then if there are any prices
	// that are the same, isolates all those prices into a list. 
	// Then re-arrange the time list such that the earliest orders
	// are first. 
	for (int i = 0; i < n; i++) { 
		for (int j = i + 1; j < n; j++) { 
			if (prices[i] == prices[j]) { 
				temp[0] = i;
				temp[k++] = j;	
			} else { 
				if (k != 1) { 
					sort_time(&temp, k, &time_map);
					memset(temp, 0, n);
					k = 1; 
				}
			}
		}
	}

	free(prices);

	return time_map; 
}

void match_orders(int* no_buy_orders, int* no_sell_orders,
	char*** buy_order_book, char*** sell_order_book, 
	int** buyers, int** sellers, pid_t* traders_pid, 
	int* traders_fd, int no_products, int*** trader_holdings, 
	char** products, int*** trader_money_spent, int* t_buy, 
	int* t_sell, matches** matched_orders, int* k, int re, 
	int* total_fees) { 

	// If either book is empty, return. 
	if (!((*no_buy_orders) > 0 && (*no_sell_orders) > 0)) { 
		return; 
	}

	if (re) { 
		(*matched_orders) = realloc((*matched_orders), sizeof(matches) * 
			((*no_buy_orders) + (*no_sell_orders)));
		for (int i = 0; i < ((*no_buy_orders) + (*no_sell_orders)); i++) { 
		(*matched_orders)[i].old_order_id = -1; 
		(*matched_orders)[i].old_trader_id = -1; 
		(*matched_orders)[i].new_order_id = -1; 
		(*matched_orders)[i].new_trader_id = -1;
		(*matched_orders)[i].value = -1;
		(*matched_orders)[i].fee = -1;
	}
	}

	char* buy_order, *sell_order;
	char* buy_id, *sell_id; 
	char* buy_qty, *sell_qty; 
	char* buy_price, *sell_price; 
	char* buy_product, *sell_product; 
	char* og_address_buy, *og_address_sell;  
	char* buy_order_dup, *sell_order_dup; 
	char amended_sell_order[MAX_MESSAGE], 
		amended_buy_order[MAX_MESSAGE]; 
	char* remaining; 
	char buy_response[MAX_MESSAGE], 
		sell_response[MAX_MESSAGE];
	int restart = 0;
	int buyer_id, seller_id; 
	int buyer_pays_fees = 0;
	int buyer_fees, seller_fees = 0;

	time* buy_orders = sort_book(buy_order_book, *no_buy_orders, 0);
	time* sell_orders = sort_book(sell_order_book, *no_sell_orders, 1);

	for (int i = 0; i < (*no_buy_orders); i++) { 
		buy_order = strdup((*buy_order_book)[buy_orders[i].time]);
		buy_order_dup = strdup((*buy_order_book)[buy_orders[i].time]);
		og_address_buy = buy_order;  

		// Skip the command 
		strsep(&buy_order, " ");

		buy_id = strsep(&buy_order, " "); 
		buy_product = strsep(&buy_order, " ");
		buy_qty = strsep(&buy_order, " ");
		buy_price = strsep(&buy_order, " ");

		buyer_id = (*buyers)[i];

		for (int j = 0; j < (*no_sell_orders); j++) { 
			buyer_pays_fees = 0;
			buyer_fees = 0; 
			seller_fees = 0;
			sell_order = strdup((*sell_order_book)[sell_orders[j].time]);
			sell_order_dup = strdup((*sell_order_book)[sell_orders[j].time]);
			og_address_sell = sell_order;  

			strsep(&sell_order, " ");

			sell_id = strsep(&sell_order, " ");
			sell_product = strsep(&sell_order, " ");
			sell_qty = strsep(&sell_order, " ");
			sell_price = strsep(&sell_order, " ");

			seller_id = (*sellers)[j]; 

			if (!strcmp(buy_product, sell_product)) { 
				memset(buy_response, 0, MAX_MESSAGE);
				memset(sell_response, 0, MAX_MESSAGE);
				memset(amended_buy_order, 0, MAX_MESSAGE);
				memset(amended_sell_order, 0, MAX_MESSAGE);
				strcpy(buy_response, "FILL "); 
				strcpy(sell_response, "FILL ");

				if (atoi(buy_price) >= atoi(sell_price)) {  
					strcat(buy_response, buy_id); 
					strcat(sell_response, sell_id);
					sell_response[strlen(sell_response)] = ' ';
					buy_response[strlen(buy_response)] = ' '; 

					// Who should pay the fees: 
					if (t_buy[i] > t_sell[j]) { 
						buyer_pays_fees = 1;
					}

					if (atoi(buy_qty) > atoi(sell_qty)) { 
						// Remove Sell Order: 
						remove_order(sell_order_book, no_sell_orders, sell_order_dup, 
							sellers, &t_sell, t_sell[sell_orders[j].time]); 
						remaining = int_to_string(atoi(buy_qty) - atoi(sell_qty)); 

						// Amend Order: 
						strcpy(amended_buy_order, "BUY ");
						strcat(amended_buy_order, buy_id); 
						amended_buy_order[strlen(amended_buy_order)] = ' ';
						strcat(amended_buy_order, buy_product); 
						amended_buy_order[strlen(amended_buy_order)] = ' ';
						strcat(amended_buy_order, remaining);
						amended_buy_order[strlen(amended_buy_order)] = ' ';
						strcat(amended_buy_order, buy_price);
						amend_order(buy_order_book, *no_buy_orders, 
							buy_order_dup, amended_buy_order);  

						// Create response message: 
						strcat(buy_response, sell_qty);  
						strcat(sell_response, sell_qty); 
						
						free(remaining); 

						if (buyer_pays_fees) { 
							buyer_fees = round(atoi(sell_qty)*atoi(buy_price)*0.01);
							(*matched_orders)[(*k)].fee = buyer_fees;
						} else { 
							seller_fees = round(atoi(sell_qty)*atoi(buy_price)*0.01);
							(*matched_orders)[(*k)].fee = seller_fees;
						}

						// Edit product and money: 
						add_product_to_trader(buyer_id, products, 
							buy_product, trader_holdings, no_products, 
							atoi(sell_qty), trader_money_spent, atoi(buy_price), 
							buyer_fees); 
						remove_product_from_trader(seller_id, products, 
							sell_product, trader_holdings, no_products, 
							atoi(sell_qty), trader_money_spent, atoi(buy_price), 
							seller_fees);
						
						(*matched_orders)[(*k)].value = atoi(buy_price)*atoi(sell_qty); 
						restart = 1;
					} else if (buy_qty == sell_qty) { 
						remove_order(buy_order_book, no_buy_orders, buy_order, 
							buyers, &t_buy, t_buy[buy_orders[i].time]);
						remove_order(sell_order_book, no_sell_orders, sell_order, 
							sellers, &t_sell, t_sell[sell_orders[j].time]);
						strcat(buy_response, sell_qty);  
						strcat(sell_response, sell_qty);

						if (buyer_pays_fees) { 
							buyer_fees = round(atoi(sell_qty)*atoi(buy_price)*0.01);
							(*matched_orders)[(*k)].fee = buyer_fees;
						} else { 
							seller_fees = round(atoi(sell_qty)*atoi(buy_price)*0.01);
							(*matched_orders)[(*k)].fee = seller_fees;
						}

						add_product_to_trader(buyer_id, products, 
							buy_product, trader_holdings, no_products, 
							atoi(buy_qty), trader_money_spent, atoi(buy_price), 
							buyer_fees); 
						remove_product_from_trader(seller_id, products, 
							sell_product, trader_holdings, no_products, 
							atoi(buy_qty), trader_money_spent, atoi(buy_price), 
							seller_fees);
						
						(*matched_orders)[(*k)].value = atoi(buy_price)*atoi(buy_qty); 
					} else { 
						remove_order(buy_order_book, no_buy_orders, buy_order_dup, 
							buyers, &t_buy, t_buy[sell_orders[i].time]); 
						remaining = int_to_string(atoi(sell_qty) - atoi(buy_qty)); 
						strcpy(amended_sell_order, "SELL ");
						strcat(amended_sell_order, sell_id); 
						amended_sell_order[strlen(amended_sell_order)] = ' ';
						strcat(amended_sell_order, sell_product); 
						amended_sell_order[strlen(amended_sell_order)] = ' ';
						strcat(amended_sell_order, remaining);
						amended_sell_order[strlen(amended_sell_order)] = ' ';
						strcat(amended_sell_order, sell_price);

						amend_order(sell_order_book, *no_sell_orders, 
							sell_order_dup, amended_sell_order);  

						strcat(buy_response, buy_qty);  
						strcat(sell_response, buy_qty);

						free(remaining); 

						if (buyer_pays_fees) { 
							buyer_fees = round(atoi(buy_qty)*atoi(buy_price)*0.01);
							(*matched_orders)[(*k)].fee = buyer_fees;
						} else { 
							seller_fees = round(atoi(buy_qty)*atoi(buy_price)*0.01);
							(*matched_orders)[(*k)].fee = seller_fees;
						}

						add_product_to_trader(buyer_id, products, 
							buy_product, trader_holdings, no_products, 
							atoi(buy_qty), trader_money_spent, atoi(buy_price), 
							buyer_fees); 
						remove_product_from_trader(seller_id, products, 
							sell_product, trader_holdings, no_products, 
							atoi(buy_qty), trader_money_spent, atoi(buy_price),
							seller_fees);
						
						(*matched_orders)[(*k)].value = atoi(buy_price)*atoi(buy_qty); 
						
						restart = 1;
					} 
					buy_response[strlen(buy_response)] = ';';
					sell_response[strlen(sell_response)] = ';';
					write(traders_fd[buyer_id], buy_response, 
						strlen(buy_response)); 
					write(traders_fd[seller_id], sell_response, 
						strlen(sell_response)); 
					kill(traders_pid[buyer_id], SIGUSR1); 
					kill(traders_pid[seller_id], SIGUSR1); 
					(*total_fees) += buyer_fees; 
					(*total_fees) += seller_fees; 

					if (buyer_pays_fees) { 
						(*matched_orders)[(*k)].old_order_id = atoi(sell_id); 
						(*matched_orders)[(*k)].old_trader_id = seller_id; 
						(*matched_orders)[(*k)].new_order_id = atoi(buy_id); 
						(*matched_orders)[(*k)].new_trader_id = buyer_id;
					} else { 
						(*matched_orders)[(*k)].old_order_id = atoi(buy_id); 
						(*matched_orders)[(*k)].old_trader_id = buyer_id; 
						(*matched_orders)[(*k)].new_order_id = atoi(sell_id); 
						(*matched_orders)[(*k)].new_trader_id = seller_id;
					} (*k)++;
					if (restart) { 
						match_orders(no_buy_orders, no_sell_orders, buy_order_book, 
							sell_order_book, buyers, sellers, traders_pid, traders_fd, 
							no_products, trader_holdings, products, trader_money_spent,
							t_buy, t_sell, matched_orders, k, 0, total_fees);
						restart = 0;
					}
				}
			}
			free(og_address_sell);
			free(sell_order_dup); 
		}
		free(og_address_buy); 
		free(buy_order_dup); 
	}
	free(buy_orders); 
	free(sell_orders); 
}

void add_to_group(int** group, int n, int id) { 
	(*group) = realloc((*group), sizeof(int) * n); 
	(*group)[n - 1] = id;
}

void remove_from_group(int** group, int n, int id) { 
	int* copy = calloc(n, sizeof(int));

	for (int i = 0; i < n + 1; i++) { 
		if ((*group)[i] == id) { 
			continue; 
		}
		copy[i] = (*group)[i];
	}

	(*group) = realloc((*group), sizeof(int) * n); 

	for (int i = 0; i < n; i++)
		(*group)[i] = copy[i];
}

int count_duplicates(char** order_book, char* order, int size) { 
	int num = 0; 
	char* order_copy = strdup(order);
	char* og_address_order = order_copy;  
	strsep(&order_copy, " "); 
	strsep(&order_copy, " "); 
	char* product = strsep(&order_copy, " "); 
	char* qty = strsep(&order_copy, " "); 
	char* price = strsep(&order_copy, " "); 

	char* cmp_order; 
	char* og_address_cmp; 
	char* cmp_product; 
	char* cmp_qty; 
	char* cmp_price;
	char* rest; 

	for (int i = 0; i < size; i++) { 
		cmp_order = strdup(order_book[i]); 
		rest = cmp_order;
		og_address_cmp = cmp_order; 

		strtok_r(rest, " ", &rest); 
		strtok_r(rest, " ", &rest);  
		cmp_product = strtok_r(rest, " ", &rest); 
		cmp_qty = strtok_r(rest, " ", &rest);  
		cmp_price = strtok_r(rest, " ", &rest);   

		if (!strcmp(cmp_product, product) 
		&&  !strcmp(cmp_qty, qty)
		&&  !strcmp(cmp_price, price)) { 
			num++; 
		}

		free(og_address_cmp); 
	}
	free(og_address_order);
	
	return num; 
}

int count_levels(char** order_book, char* product, int size) {
	int no_orders = 0; 
	int num = 0; 
	int printed = 0;

	char* order_copy; 
	char* cmp_product;
	char* og_address;

	for (int j = 0; j < size; j++) { 
		
		order_copy = strdup(order_book[j]);
		og_address = order_copy; 
		strsep(&order_copy, " "); 
		strsep(&order_copy, " "); 
		cmp_product = strsep(&order_copy, " "); 

		if (!strcmp(cmp_product, product)) { 
			no_orders = count_duplicates(order_book, order_book[j], 
			size);	
			// TODO: FIX

			if (no_orders > 1 && printed) { 
				continue; 
			} else if (no_orders > 1 && !printed){ 
				num++; 
				printed = 1; 
			} else { 
				num++;
				printed = 0;
			}
		}
		free(og_address);
	}

	return num;
}

void report(int id, char** products, int no_products,
	char* order, int no_buy_orders, char** buyer_book, 
	int no_sell_orders, char** seller_book, int argc, 
	int** trader_holdings, int** trader_money_spent, 
	matches* matched, int k) { 

	char* current_order; 
	char* og_address; 
	int no_orders = 0; 
	int buy_levels; 
	int sell_levels; 
	int price = 0; 
	int qty = 0;
	int printed = 0;

	order[strlen(order) - 1] = 0;
	printf("%s [T%d] Parsing command: <%s>\n", LOG_PREFIX, id, order);

	for (int i = 0; i < k; i++) { 
		printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%d, fee: $%d.\n", 
			LOG_PREFIX, matched[i].old_order_id, matched[i].old_trader_id, 
			matched[i].new_order_id, matched[i].new_trader_id, 
			matched[i].value, matched[i].fee);
	}
	
	printf("%s\t--ORDERBOOK--\n", LOG_PREFIX);

	for (int i = 0; i < no_products; i++) {
		buy_levels = count_levels(buyer_book, 
			products[(no_products - 1) - i], no_buy_orders); 
		sell_levels = count_levels(seller_book, 
			products[(no_products - 1) - i], no_sell_orders);
		printf("%s\tProduct: %s; Buy levels: %d; Sell levels: %d\n", 
			LOG_PREFIX, products[(no_products - 1) - i], 
			buy_levels, sell_levels); 
		if (buy_levels == 0 && sell_levels == 0) {
			continue; 
		}

		for (int j = 0; j < no_buy_orders; j++) { 
			current_order = strdup(buyer_book[(no_buy_orders - 1) - j]); 
			og_address = current_order; 
			strsep(&current_order, " "); 
			strsep(&current_order, " ");
			strsep(&current_order, " ");
			qty = atoi(strsep(&current_order, " "));
			price = atoi(strsep(&current_order, " "));
			no_orders = count_duplicates(buyer_book, 
				buyer_book[(no_buy_orders - 1) - j], no_buy_orders);

			if (no_orders > 1 && printed) { 
				continue; 
			} else if (no_orders > 1 && !printed){ 
				printf("%s\t\tBUY %d @ $%d (%d orders)\n", 	
					LOG_PREFIX, qty*no_orders, price, no_orders); 
				printed = 1; 
			} else { 
				printf("%s\t\tBUY %d @ $%d (%d order)\n", 	
					LOG_PREFIX, qty*no_orders, price, no_orders);
			}

			free(og_address); 
		}
			
		for (int j = 0; j < no_sell_orders; j++) { 
			current_order = strdup(seller_book[(no_sell_orders - 1) - j]); 
			og_address = current_order; 
			strsep(&current_order, " "); 
			strsep(&current_order, " ");
			strsep(&current_order, " ");
			qty = atoi(strsep(&current_order, " "));
			price = atoi(strsep(&current_order, " "));
			no_orders = count_duplicates(seller_book, 
				seller_book[(no_sell_orders - 1) - j], no_sell_orders);
			
			if (no_orders > 1 && printed) { 
				continue; 
			} else if (no_orders > 1 && !printed){ 
				printf("%s\t\tSELL %d @ $%d (%d orders)\n", 	
					LOG_PREFIX, qty*no_orders, price, no_orders); 
				printed = 1; 
			} else { 
				printf("%s\t\tSELL %d @ $%d (%d order)\n", 	
					LOG_PREFIX, qty*no_orders, price, no_orders);
			}

			free(og_address); 
		}
	}

	printf("%s\t--POSITIONS--\n", LOG_PREFIX);

	for (int i = 0; i < argc - 2; i++) { 
		printf("%s\tTrader %d: ", LOG_PREFIX, i); 
		for (int j = 0; j < no_products; j++) { 
			printf("%s %d ($%d)", products[j], trader_holdings[i][j], 
				trader_money_spent[i][j]);
			if (j != no_products - 1) { 
				printf(", "); 
			} else { 
				printf("\n"); 
			}
		}
	}
}

int main(int argc, char **argv) {
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO; 
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = &recieve_signal;
	sigaction(SIGUSR1, &sa, NULL);

	// Error Check: 2 arguments needed minimum
	if (argc < 3) { 
		printf("Not enough arguments.\n"); 
		return -1;
	}

	struct pollfd* pfds = calloc(argc - 2, sizeof(struct pollfd));

	char* file_content; 
	int* exchange_fd;
	int* trader_fd;
	pid_t* traders_pid;
	char** sell_order_book;
	char** buy_order_book;
	int no_sell_orders = 0;
	int no_buy_orders = 0;
	int id; 
	char order[MAX_MESSAGE];
	int type; 
	int* sellers;
	int* buyers; 
	char** products; 
	int** trader_holdings;
	int** trader_money_spent; 
	int no_products; 
	int t = 0; 
	int* t_buy; 
	int* t_sell; 
	matches* matched_orders;  
	int k = 0; 
	int revents; 
	int all_fds_closed; 
	int total_fees = 0;

	// They are allocated 1 just so that we can realloc it in the function later. 
	matched_orders = malloc(1); 
	sellers = malloc(1); 
	buyers = malloc(1);
	t_buy = malloc(1); 
	t_sell = malloc(1);
	sell_order_book = malloc(1);
	buy_order_book = malloc(1);
	traders_pid = calloc(argc - 2, sizeof(pid_t));
	trader_fd = calloc(argc - 2, sizeof(int));
	exchange_fd = calloc(argc - 2, sizeof(int));
	file_content = read_file(argv[1]); 
	trader_holdings = calloc(argc - 2, sizeof(int*));

	// Print Products: 
	no_products = get_number_of_products(file_content);
	products = calloc(no_products, sizeof(char*)); 
	store_products(file_content, &products, no_products);
	printf("%s Starting\n", LOG_PREFIX);
	printf("%s Trading %d products: ", LOG_PREFIX, 
		no_products);
	for (int i = 0; i < no_products; i++) { 
		printf("%s", products[i]);
		if (i == no_products - 1) { 
			printf("\n"); 
		} else { 
			printf(" ");
		}
	}

	for (int i = 0; i < argc - 2; i++) { 
		trader_holdings[i] = calloc(no_products, sizeof(int));
	}
	trader_money_spent = calloc(argc - 2, sizeof(int*));
	for (int i = 0; i < argc - 2; i++) { 
		trader_money_spent[i] = calloc(no_products, sizeof(int));
	}
	
	// Create FIFOs & Execute Trader Binaries: 
	for (int i = 2; i < argc; i++) { 
		create_FIFO(i - 2);
		execute_trader(argv[i], i - 2, traders_pid, 
			exchange_fd, trader_fd);
	}

	for (int i = 0; i < argc - 2; i++) { 
		pfds[i].fd = trader_fd[i]; 
		pfds[i].events = POLLHUP;
	}

	// Announce Market is open to all tarders:  
	for (int i = 2; i < argc; i++) { 
		market_open(i - 2, traders_pid[i - 2], exchange_fd);
	}

	while (1) { 
		if (signal_recieved) { 
			id = find_sender_id(argc, traders_pid); 
			type = read_order(id, trader_fd, order); 
			if (type == 500) { 
				store_order(order, &no_buy_orders, &buy_order_book, 
					&t_buy, &t);
				add_to_group(&buyers, no_buy_orders, id); 
			} else if (type == 400) { 
				store_order(order, &no_sell_orders, &sell_order_book, 
					&t_sell, &t); 
				add_to_group(&sellers, no_sell_orders, id);
			}
			match_orders(&no_buy_orders, &no_sell_orders, &buy_order_book, 
				&sell_order_book, &buyers, &sellers, traders_pid, trader_fd, 
				no_products, &trader_holdings, products, &trader_money_spent,
				t_buy, t_sell, &matched_orders, &k, 1, &total_fees);  
			report(id, products, no_products, order, no_buy_orders, buy_order_book, 
			no_sell_orders, sell_order_book, argc, trader_holdings, trader_money_spent, 
			matched_orders, k);
			signal_recieved = 0;
			k = 0; 

			for (int i = 0; i < argc - 2; i++) { 
				if (i == id)
					respond_to_order(order, exchange_fd[i], traders_pid[i]);
				else 
					announce_order(order, exchange_fd[i], traders_pid[i]);
			}
		}

		poll(pfds, argc - 2, -1);
		for (int i = 0; i < argc - 2; i++) { 
			revents = pfds[i].revents; 
			if (revents & POLLHUP) { 
				printf("Trader %d disconnected\n", i);
				unlink(get_exchange_FIFO(i));
				unlink(get_trader_FIFO(i));
				close((pfds[i].fd));
				pfds[i].fd *= -1;
			} 
		} 

		all_fds_closed = 1; 
		for (int i = 0; i < argc - 2; i++) { 
			if (pfds[i].fd > 0) {
				all_fds_closed = 0;
			}
		}

		if (all_fds_closed) { 
			printf("Trading completed\n");
			printf("Exchange fees collected: $%d\n", total_fees);
			sleep(1);
			break; 
		}
	}

	free(file_content);
	free(exchange_fd); 
	free(trader_fd);
	free(traders_pid);
	free(buy_order_book);
	free(sell_order_book);

	for (int i = 0; i < no_buy_orders; i++) { 
		free(buy_order_book[i]);
	}

	for (int i = 0; i < no_sell_orders; i++) { 
		free(sell_order_book[i]);
	}

	return 0;
}