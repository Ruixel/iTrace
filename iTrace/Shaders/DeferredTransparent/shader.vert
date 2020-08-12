#version 400
layout(location=0) in vec4 RawPos; 
layout(location=1) in vec3 RawTexCoord;
layout(location=2) in vec3 Normal;
layout(location=3) in vec3 Tangent;

uniform mat4 IdentityMatrix; 
uniform mat4 CombinedMatrix; 
uniform float Time; 

out vec4 Pos; 
out vec3 TexCoord; 
out vec3 InNormal; 
out vec3 InTangent; 

vec3 GetWave(vec3 Vertex) {

	//spherical coordinates 
	float Pitch = cos(Time+.31232123) * 3.1415 + 3.1415; 
	float Yaw = sin(Time-12.123123) * 3.1415 + 3.1415; 

	float CosPitch = cos(Pitch); 

	vec3 Coord; 
	Coord.x = cos(Yaw) * CosPitch; 
	Coord.y = sin(Pitch); 
	Coord.z = sin(Yaw) * CosPitch; 

	return Coord * 0.05; 

}

void main(void) {	
	Pos = RawPos ;
//	Pos.xyz += GetWave(Pos.xyz); 
	TexCoord = RawTexCoord; 
	gl_Position = CombinedMatrix * vec4(Pos.xyz ,1.0); 
	InNormal = Normal; 
	InTangent = Tangent; 
}