CC=g++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=
SOURCES=main.cpp common.cpp opfinder.cpp opcounter.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=OptimalThresholdFinder

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -f ${OBJECTS}
