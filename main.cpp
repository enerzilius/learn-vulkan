#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "src/class/VulkanStuff.h"
#include "src/class/VulkanUtils.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define APP_NAME "VULKAN"

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char** argv)
{
  GLFWwindow* window;

  glfwSetErrorCallback(error_callback);

  if(!glfwInit()){
    std::cout<<"ERROR INITATING GLFW!\n\n";
    exit(EXIT_FAILURE);
  }

  if(!glfwVulkanSupported()) {
    std::cout<<"VULKAN NOT SUPPORTED!\n\n";
    exit(EXIT_FAILURE);
  }
  
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, NULL, NULL); 

  if(!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  vks::VulkanApp app;
  app.Init(APP_NAME);

  std::cout<<"PROJETO COM VULKAN xD\n";

  while(!glfwWindowShouldClose(window)) {
    app.RenderScene();
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
 
  exit(EXIT_SUCCESS); 
}
