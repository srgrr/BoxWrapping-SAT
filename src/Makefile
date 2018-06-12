CC=g++
GCC_FLAGS=-std=c++17 -O3 -Wall -Wno-shadow

encodings.o: encodings.cc encodings.h
	${CC} -c -o encodings.o encodings.cc ${GCC_FLAGS}

test_encodings: encodings.o test_encodings.cc
	${CC} -o test_encodings test_encodings.cc encodings.o ${GCC_FLAGS}

main: encodings.o main.cc
	${CC} -o main main.cc encodings.o ${GCC_FLAGS}

clean:
	rm -f *.o test_encodings
