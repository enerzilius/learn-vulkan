#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>

#define WINDOW_HEIGHT 1920
#define WINDOW_WIDTH 1080

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLFWwindow* window = NULL;

int main(int argc, char** argv)
{
  if(!glfwInit()){
    std::cout<<"ERROR INITATING GLFW!\n\n";
    return 1;
  }

  if(!glfwVulkanSupported()) {
    std::cout<<"VULKAN NOT SUPPORTED!\n\n";
    return 1;
  }

  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "VULKAN", NULL, NULL);

  if(!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
    return 1;
  }
 
  std::cout<<"PROJETO COM VULKAN xD\n";

  return 0;
}
