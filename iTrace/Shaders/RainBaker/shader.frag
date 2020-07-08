#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec4 Data;

uniform float Time; 

float GetPuddle(vec2 uv,float time) {
    float distancePuddle = length(uv*2.-1.); 
    
    float t = fract(time) * .8; 
    
	if(distancePuddle > t  && distancePuddle < t+0.2) {
    	float h = 1.0-abs(((distancePuddle-fract(time)) * 5.0) * 2. - 1.); 
    	return h * pow(1.0-t,2.0); 
    }
    return 0.0; 
}

vec3 GetPuddleNormal(vec2 uv,float time,vec3 norm) {
	float addon = 0.05; 
    
    float intensity = GetPuddle(uv,time); 
    float PuddleR = GetPuddle(uv-vec2(addon,0.),time); 
    float PuddleL = GetPuddle(uv+vec2(addon,0.),time); 
    float PuddleU = GetPuddle(uv+vec2(0.,addon),time); 
    float PuddleD = GetPuddle(uv-vec2(0.,addon),time); 
    
    return mix(normalize(vec3(PuddleR-PuddleL, 1., PuddleU-PuddleD)),vec3(norm),1.0-intensity); 

}


void main() {
	Data.xyz = GetPuddleNormal(TexCoord, Time, vec3(0.0,1.0,0.0)).xzy * 0.5 + 0.5;	
	Data.w = GetPuddle(TexCoord, Time); 

}