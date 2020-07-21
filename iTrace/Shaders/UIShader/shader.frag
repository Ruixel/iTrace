#version 330
in vec2 TexCoord; 
out vec4 Result; 

uniform sampler2D Texture; 

void main() {

	Result = texture(Texture, TexCoord); 

	if(Result.a < 0.5) 
		discard; 

}