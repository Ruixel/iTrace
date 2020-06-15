#version 420

in vec2 TexCoord; 

layout(location = 0) out float Frame;

uniform sampler2D PreviousFrame; 
uniform sampler2D MotionVectors; 


void main() {



	vec2 MotionVectors = texture(MotionVectors, TexCoord).xy; 
	float _f = max(texture(PreviousFrame, TexCoord + MotionVectors).x,0.0); 

	float l = length(MotionVectors); 

	if(MotionVectors.x <= -.99 || MotionVectors.y <= -.99)  {
		_f = 0.0; 
	}
	Frame = _f + 1.0;  
	
}