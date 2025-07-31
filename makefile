CXX       := gcc
CXX_FLAGS := -ggdb

BIN     := bin
SRC     := src
INCLUDE := include
LIBRARIES   := -lws2_32
EXECUTABLE  := main

all: $(BIN)/$(EXECUTABLE)

run: clean all
	cls
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.c
	$(CXX) $(CXX_FLAGS) -I $(INCLUDE)  $^ $(LIBRARIES) -o $@
	
clean:
	del /Q $(BIN)\*