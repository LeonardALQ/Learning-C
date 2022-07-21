CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak
LDFLAGS=-lm
BINARIES=spx_exchange spx_trader

all: $(BINARIES)

.PHONY: clean
clean:
	rm -f $(BINARIES)

exchange:
	gcc -o spx_exchange spx_exchange.c -Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak -lm

traders:
	gcc -o trader0 spx_trader.c -Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak -lm
	gcc -o trader1 spx_trader1.c -Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak -lm

run:
	./spx_exchange products.txt ./trader0 ./trader1

a:
	make exchange
	make traders
	make run

push:
	git add .
	git commit -m "T"
	git push

