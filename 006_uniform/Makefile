.PHONY: clean

CC := g++
DIR := .
BUILD_DIR := build
INCLUDE_DIR := include
ASSETS_DIR := shader
GLSLC := glslc
STD := c++20

build_dir:
	mkdir -p ${BUILD_DIR}

compile: build_dir \
		${BUILD_DIR}/main.o \
		${BUILD_DIR}/hello_triangle_app.o \
		${BUILD_DIR}/vert.spv \
		${BUILD_DIR}/frag.spv

${BUILD_DIR}/vert.spv: ${ASSETS_DIR}/vert.glsl
	${GLSLC} -fshader-stage=vertex ${ASSETS_DIR}/vert.glsl -o ${BUILD_DIR}/vert.spv

${BUILD_DIR}/frag.spv: ${ASSETS_DIR}/frag.glsl
	${GLSLC} -fshader-stage=fragment ${ASSETS_DIR}/frag.glsl -o ${BUILD_DIR}/frag.spv

${BUILD_DIR}/main.o: main.cpp 
	${CC} -std=${STD} -I ${INCLUDE_DIR} -c main.cpp -o ${BUILD_DIR}/main.o

${BUILD_DIR}/hello_triangle_app.o: hello_triangle_app.cpp hello_triangle_app.h
	${CC} -std=${STD} -I ${INCLUDE_DIR} -c hello_triangle_app.cpp -o ${BUILD_DIR}/hello_triangle_app.o

# Windows
build:${BUILD_DIR}/main.o compile \
	${BUILD_DIR}/hello_triangle_app.o 
	g++ ${BUILD_DIR}/*.o -I ${INCLUDE_DIR} -L"lib/win64" -lglfw3 -lvulkan-1 -lgdi32 -lopengl32 -o ${BUILD_DIR}/app

# Linux
# build:${BUILD_DIR}/main.o compile ${BUILD_DIR}/hello_triangle_app.o 
# 	g++ ${BUILD_DIR}/*.o -I include -L"lib/linux" -lglfw3 -lvulkan -lpthread -lGLU -lGL -lXrandr -lXxf86vm -lXi -lXinerama -lX11 -lrt -ldl -o ${BUILD_DIR}/app

# macos arm64
# build:${BUILD_DIR}/main.o compile ${BUILD_DIR}/hello_triangle_app.o 
# 	g++ ${BUILD_DIR}/*.o -I include -L"lib/mac_arm64" -lglfw3 \
# 	-lvulkan \
# 	-Wl,-rpath,/Users/wenmingyan/vulkan_sdk/sdk1_4/macOS/lib \
# 	-framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL \
# 	-o ${BUILD_DIR}/app

run:build
	${BUILD_DIR}/app

clean:
	rm -rf ${BUILD_DIR}/*.o
	rm -rf ${BUILD_DIR}/*.exe
	rm -rf ${BUILD_DIR}/*.spv
	rm -rf ${BUILD_DIR}/app
