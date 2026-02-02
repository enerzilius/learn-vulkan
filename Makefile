all: 
	g++ -std=c++11 main.cpp -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
