CC = g++
FILES1 = encoder.hpp encoder_test.cpp

OUT_BIN1 = encoder_test


build: $(FILES1) $(FILES2)
	$(CC) -o $(OUT_BIN1) $(FILES1)

clean:
	rm -f *.o core

rebuild: clean build
