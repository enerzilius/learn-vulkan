all: 
	g++ -std=c++11 main.cpp src/class/VulkanStuff.cpp -lvulkan -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
