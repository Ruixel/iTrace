#version 420

in vec2 TexCoord; 

layout(location = 0) out float Frame;

uniform sampler2D PreviousFrame; 
uniform sampler2D MotionVectors; 


void main() {



	vec3 MotionVectors = texture(MotionVectors, TexCoord).xyz; 
	float _f = max(texture(PreviousFrame, TexCoord + MotionVectors.xy).x,0.0); 

	float l = length(MotionVectors); 

	if(MotionVectors.x <= -.99 || MotionVectors.y <= -.99 || MotionVectors.z <= -.99)  {
		_f = 0.0; 
	}
	Frame = _f + 1.0;  
	
}