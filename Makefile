# mazerunner.makefile
mygcc = gcc -Wall -pedantic -std=c11

all: maze_runner testing

maze_runner: ./src/AMStartup.o ./src/maze.o ./src/list.o ./src/move.o ./src/maze_array.o ./src/file.o
	$(mygcc) -o maze_runner ./src/AMStartup.o ./src/maze.o ./src/list.o ./src/move.o ./src/maze_array.o ./src/file.o -lpthread

testing: ./test/unit_test.o ./src/maze_array.o ./src/list.o ./src/move.o
	$(mygcc) -o testing ./test/unit_test.o ./src/maze_array.o ./src/list.o ./src/move.o

AMStartup.o: ./src/AMStartup.c ./src/maze.h ./src/amazing.h ./src/file.h
	gcc -Wall -pedantic -std=c11 -c AMStartup.c

maze.o: ./src/maze.c ./src/maze.h ./src/amazing.h
	gcc -Wall -pedantic -std=c11 -c ./src/maze.c

list.o:	./src/list.c ./src/list.h ./src/common.h
	gcc -Wall -pedantic -std=c11 -c ./src/list.c

move.o:	./src/move.c ./src/maze.h ./src/amazing.h
	gcc -Wall -pedantic -std=c11 -c ./src/move.c

file.o: ./src/file.c ./src/file.h
	gcc -Wall -pedantic -std=c11 -c ./src/file.c

maze_array.o: ./src/maze_array.c ./src/maze.h ./src/amazing.h ./src/list.h
	gcc -Wall -pedantic -std=c11 -c ./src/maze_array.c

unit_test.o: ./src/unit_test.c ./src/list.c ./src/move.c ./maze_array.c ./src/list.h ./src/maze.h ./src/amazing.h
	gcc -Wall -pedantic -std=c11 -c ./src/unit_test.c

clean:
	rm -f ./src/*~
	rm -f ./src/*#
	rm -f ./src/*.o
	rm -f maze_runner
	rm -f testing

