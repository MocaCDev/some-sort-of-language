.PHONY: run
.PHONY: clean

FLAGS = -Wall -fsanitize=leak -o
SC_FILES := $(shell find $(language_backend) -name '*.c')

run:
	@gcc main.c $(FLAGS) bin/main.o

clean:
	rm -rf bin/*