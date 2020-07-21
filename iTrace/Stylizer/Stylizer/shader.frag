#version 330
in vec2 TexCoord; 
out vec4 Result; 

uniform sampler2D Texture; 
uniform sampler2D LowResTexture; 

void main() {
	Result = mix(texture(Texture, TexCoord), texture(LowResTexture, TexCoord), 0.875); 
}