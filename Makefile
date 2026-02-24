all: 
	g++ -std=c++20 main.cpp src/class/VulkanStuff.cpp -lvulkan -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -o main

helloTriangle:
	g++ --std=c++20 src/applications/helloTriangleApp.cpp -lvulkan -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -o helloTriangle

