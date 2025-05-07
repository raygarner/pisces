NAME=pisces
SRC=$(NAME).c
FLAGS=-lm

all: $(SRC)
	$(CC) $(SRC) $(FLAGS) -o $(NAME)

clean:
	rm -f $(NAME)

