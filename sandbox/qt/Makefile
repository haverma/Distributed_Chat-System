all: test

test: main.o
	g++ -o test test.h -I /usr/lib/qt4/include -L /usr/lib/qt4/lib -lQtCore -lQtGui main.cpp

clean:
	rm *o test



