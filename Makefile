objects = main.o
options = -lncurses
out = quarto

quarto : $(objects)
	cc -o $(out) $(objects) $(options)
main.o : main.c
	cc -c main.c
clean :
	rm $(out) $(objects)
