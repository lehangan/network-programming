CC = g++
CFLAGS = -Wall

SRC1 = exercise1.cpp
OUT1 = resolver

SRC2 = exercise2.cpp
OUT2 = resolver2

all: $(OUT1) $(OUT2)

$(OUT1): $(SRC1)
	$(CC) $(CFLAGS) $(SRC1) -o $(OUT1)

$(OUT2): $(SRC2)
	$(CC) $(CFLAGS) $(SRC2) -o $(OUT2)

clean:
	rm -f $(OUT1) $(OUT2) 