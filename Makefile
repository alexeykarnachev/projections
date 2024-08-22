all:
	gcc \
	-Wall \
	-o ./projections \
	-Wl,-rpath=./deps/lib/ \
	./src/*.c \
	-I ./deps/include/ -L ./deps/lib/ \
	-lraylib -lcimgui -pthread -lm -ldl
