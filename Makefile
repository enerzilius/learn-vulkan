.PHONY: all vkStuff helloTriangle utils main clean

CC=g++ --std=c++20
ARGS=-lvulkan -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -o

APPS_PATH=src/applications
UTILS_PATH= src/utils

UTILS=utils
TRIANGLE=helloTriangleApp

PROGS=$(UTILS) $(TRIANGLE)

all: $(PROGS)

$(TRIANGLE): $(UTILS)
	$(CC) $(APPS_PATH)/$(TRIANGLE).cpp $(UTILS).o $(ARGS) $(TRIANGLE).out

$(UTILS):
	$(CC) -c $(UTILS_PATH)/$(UTILS).cpp -o $(UTILS).o

clean:
	rm -rf *.out *.o
