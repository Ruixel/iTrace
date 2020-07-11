#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;

uniform mat4 ViewMatrix; 
uniform mat4 IdentityMatrix; 
uniform sampler2D InstanceData; 
out vec2 TexCoord;  

uniform int Resolution; 

mat4 ModelMatrixFromPosition(vec3 Position) {
	mat4 Matrix = mat4(0.0); 

	for(int x=0;x<3;x++) {
		for(int y=0;y<3;y++) {
			Matrix[x][y] = ViewMatrix[y][x]; 
		}
	}

	Matrix[3] = vec4(Position, 1.); 
	
	return Matrix; 
}


void main(void) {

	ivec2 Index = ivec2(gl_InstanceID % Resolution, gl_InstanceID / Resolution); 

	vec4 Data = texelFetch(InstanceData, Index, 0); 

	mat4 Matrix = ModelMatrixFromPosition(Data.xyz); 


	vec2 Size = vec2(0.0175,0.07); 

	TexCoord = Texc; 
	gl_Position = IdentityMatrix * Matrix * vec4(Vert.xy * Size,0.f,1.0); 
}