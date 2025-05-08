#version 450

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 color;

layout(location = 0)out vec3 aColor;

void main(){
    gl_Position = vec4(pos, 0.0f , 1.0f);
    aColor = color;
}
