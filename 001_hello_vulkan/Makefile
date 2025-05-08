.PHONY: clean

easyvk.o:easyvk.cpp
	g++ easyvk.cpp -c -std=c++20 -I include -o easyvk.o

# glfw_general.o:glfw_general.hpp
# 	g++ -c glfw_general.hpp -std=c++20 -I include -o glfw_general.o

main.o:main.cpp
	g++ main.cpp -c -std=c++20 -I include -o main.o



#build:main.o easyvk.o 
#	g++ *.o -I include -L"lib/win64" -lglfw3 -lvulkan-1 -lgdi32 -lopengl32 -o app

build:main.o easyvk.o 
	g++ *.o -I include -L"lib/linux" -lglfw3 -lvulkan -lpthread -lGLU -lGL -lXrandr -lXxf86vm -lXi -lXinerama -lX11 -lrt -ldl -o app

run:build
	./app

clean:
	rm -rf *.o
	rm -rf *.exe
	rm -rf app
