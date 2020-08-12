#version 400

layout(location = 0) out vec4 TexCoordSideBlockType; 
layout(location = 1) out vec4 Normal; 
layout(location = 2) out vec4 Tangent; 

in vec4 Pos; 
in vec3 TexCoord;
in vec3 InNormal; 
in vec3 InTangent; 

uniform sampler2DArray DiffuseTextures; 

float mip_map_level(in vec2 texture_coordinate)
{
    vec2  dx_vtc        = dFdx(texture_coordinate);
    vec2  dy_vtc        = dFdy(texture_coordinate);
    float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));
    return 0.5 * log2(delta_max_sqr);
}

void main() {	
	TexCoordSideBlockType.xy = fract(TexCoord.xy); 
	TexCoordSideBlockType.zw = vec2(TexCoord.z,Pos.w); 
	Normal.xyz = InNormal;
	Normal.w = mip_map_level(TexCoord.xy * vec2(512)); 
	Tangent.xyz = InTangent; 
	//Tangent.w = textureQueryLod(DiffuseTextures, TexCoord.xy).x; 
	//gl_FragDepth = 0.999999; 
}