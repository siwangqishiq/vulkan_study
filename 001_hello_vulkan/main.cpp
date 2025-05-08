#include "easyvk.h"
#include "glfw_general.hpp"

int main(){
    if(!InitWindow({1280,720})){
        return -1;
    }
    while (!glfwWindowShouldClose(pWindow)) {
        glfwPollEvents();
        TitleFps();
    }
    TerminateWindow();
    return 0;
}//end main
