main: *.c
	gcc -O2 -Wall -MMD -std=c11 -masm=intel $^ -o $@
debug: *.c
	gcc -g -Og -Wall -MD -std=c11 -masm=intel $^ -o $@
.PHONY:run
run: main
	./$<
.PHONY:clean
clean:
	rm -f main main.d debug debug.d
