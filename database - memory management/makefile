output:
	gcc -o ymirdb ymirdb.c -fsanitize=address -g -Werror

clean: 
	rm ymirdb

valgrind: 
	gcc -o a ymirdb.c
	valgrind --leak-check=full -v ./a

test: 
	make
	./test.sh

push: 
	git add .
	git commit -m "t"
	git push
