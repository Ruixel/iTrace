#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;
out vec2 TexCoord;  
uniform int Character; 
uniform vec2 Position; 
uniform vec2 Size; 

void main(void) {
	TexCoord.x = Texc.x * 0.0075 + 0.00875 * float(Character); 
	TexCoord.y = Texc.y; 
	gl_Position = vec4(Vert.xy * Size + Position,1.0,1.0); 
}