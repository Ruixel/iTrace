#version 330
in vec2 TexCoord; 
out vec4 Result; 

uniform sampler2D BackDrop; 
uniform sampler2DArray ItemTextures; 
uniform int Texture; 

void main() {

	vec4 BG = texture(BackDrop, TexCoord); 

	vec4 Item = vec4(0); 
	if(Texture > 0)
		Item = texture(ItemTextures, vec3(TexCoord + vec2(0.0,0.06),Texture-1)); 
	
	Result = mix(BG, Item, Item.w); 
}