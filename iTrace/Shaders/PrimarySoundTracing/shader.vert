#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;


void main(void) {
gl_Position = vec4(Vert.xy,0.f,1.0); 
}