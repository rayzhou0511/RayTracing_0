# Compiler
CC = clang++

# Compiler flags
CFLAGS = -std=c++11 -O2 -Wall -Xpreprocessor -fopenmp

# Include path for GLM and stb
INCLUDE_PATH = -I/usr/local/include/glm -I/usr/local/include/opencv4 -I/usr/local/opt/libomp/include

# Libraries
LIBS = -lGL -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -L/usr/local/opt/libomp/lib -lomp

# Source file
SRC = main.cpp graph.cpp

# Output binary
BIN = raytracing

all: $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -o $(BIN) $(SRC) $(LIBS)

clean:
	rm $(BIN)