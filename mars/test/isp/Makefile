CFLAGS += -lm -g -DENV_EMU

go_test: *.c cli/*.c
	gcc -o $@ *.c cli/*.c $(CFLAGS)

clean:
	rm go_test
