#version 420

in vec2 TexCoord; 

layout(location = 0) out float Frame;
layout(location = 1) out float SpecularFrame;

uniform sampler2D PreviousFrame; 
uniform sampler2D MotionVectors; 

uniform sampler2D PreviousSpecularFrame; 
uniform sampler2D SpecularMotionVectors; 


void main() {



	vec3 MotionVectors = texture(MotionVectors, TexCoord).xyz; 
	vec3 MotionVectorsSpec = texture(SpecularMotionVectors, TexCoord).xyz; 
	float _f = max(texture(PreviousFrame, TexCoord + MotionVectors.xy).x,0.0); 
	float _sf = max(texture(PreviousSpecularFrame, TexCoord + MotionVectorsSpec.xy).x,0.0); 

	if(MotionVectors.x <= -.99 || MotionVectors.y <= -.99 || MotionVectors.z <= -.99)  {
		_f = 0.0; 
	}
	Frame = _f + 1.0;  


	if(MotionVectorsSpec.x <= -.99 || MotionVectorsSpec.y <= -.99 || MotionVectorsSpec.z <= -.99)  {
		_sf = 0.0; 
	}
	SpecularFrame = _sf + 1.0;  
	
}