#include "spx_trader.h"

pid_t exchange_pid;
int signal_recieved = 0;

void recieve_signal(int sig, siginfo_t* info, void* context) { 
	exchange_pid = info->si_pid;
	signal_recieved = 1;
}

void read_message(char* msg, int exchange_fd) {
	memset(msg, ' ', MAX_MESSAGE); 
	char chr = ' ';
	int k = 0;
	
	while (chr != ';' && k != MAX_MESSAGE) {
		read(exchange_fd, &chr, sizeof(char));
		msg[k++] = chr;
	} msg[k] = 0;
}

int main(int argc, char ** argv) {
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO; 
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = &recieve_signal;
	sigaction(SIGUSR1, &sa, NULL);

    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

	// Orders:  
	int order_num = 0;
	char orders[4][19] = {
		"BUY 0 CPU 30 500;",
		"BUY 1 CPU 30 501;",
		"BUY 2 CPU 30 501;",
		"BUY 3 CPU 30 502;", 
	};

	char msg[MAX_MESSAGE];
    int id = atoi(argv[1]);

	int exchange_fd = open(get_exchange_FIFO(id), O_RDONLY);
	int trader_fd = open(get_trader_FIFO(id), O_WRONLY);
    
	while (1) {
		if (signal_recieved && order_num < 4) { 
			read_message(msg, exchange_fd);
			write(trader_fd, orders[order_num], strlen(orders[order_num])); 
			kill(exchange_pid, SIGUSR1); 
			order_num++;
			signal_recieved = 0;
		} 
		if (order_num == 4) { 
			close(trader_fd);
			exit(0);
		}
	}
}
