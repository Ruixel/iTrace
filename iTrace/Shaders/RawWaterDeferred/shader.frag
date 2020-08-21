#version 330 

layout(location = 0) out vec3 LFNormal;
layout(location = 1) out vec3 WorldPos; 
layout(location = 2) out vec3 HFNormal; 

in vec3 Pos; 

uniform sampler2D WaterNormal; 

void main() {
	LFNormal = texture(WaterNormal, Pos.xz * .25).xzy * 2 - 1; 
	WorldPos = Pos; 
	HFNormal = vec3(0.0,1.0,0.0); 
//	LFNormal = HFNormal; 
}