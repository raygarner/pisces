NAME=pisces
SRC=$(NAME).c
FLAGS=-lm -O2

all: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(NAME)

clean:
	rm -f $(NAME)

