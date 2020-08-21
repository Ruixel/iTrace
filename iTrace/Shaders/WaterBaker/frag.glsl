#version 420

in vec2 TexCoord; 

out vec4 Color; 

#define ITERATIONS_NORMAL 48

uniform int Frame; 

vec2 wavedx(vec2 position, vec2 direction, float speed, float frequency, float timeshift) {
    float x = dot(direction, position) * frequency + timeshift * speed;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return vec2(wave, -dx);
}

float getwaves(vec2 position, int iterations, float Time){
	float iter = 0.0;
    float phase = 6.0;
    float speed = 2.0;
    float weight = 1.0;
    float w = 0.0;
    float ws = 0.0;
    for(int i=0;i<iterations;i++){
        vec2 p = vec2(sin(iter), cos(iter));
        vec2 res = wavedx(position, p, speed, phase,Time);
        position += normalize(p) * res.y * weight * 0.05;
        w += res.x * weight;
        iter += 12.0;
        ws += weight;
        weight = mix(weight, 0.0, 0.2);
        phase *= 1.18;
        speed *= 1.07;
    }
    return w / ws;
}

float H = 0.0;
vec4 normal(vec2 pos, float e, float depth, float Time){
    vec2 ex = vec2(e, 0);
    H = getwaves(pos.xy * 0.1, ITERATIONS_NORMAL,Time) * depth;
    vec3 a = vec3(pos.x, H, pos.y);
    vec3 R= normalize(cross(normalize(a-vec3(pos.x - e, getwaves(pos.xy * 0.1 - ex.xy * 0.1, ITERATIONS_NORMAL,Time) * depth, pos.y)), 
                           normalize(a-vec3(pos.x, getwaves(pos.xy * 0.1 + ex.yx * 0.1, ITERATIONS_NORMAL,Time) * depth, pos.y + e))));
	return vec4(R,H); 
}

float Zoom = 24.0; 
float Power = 4.0; 

int TIME = 120; 
float TimeMultiplier = 8.0; 

void main()
{
    
     
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = TexCoord;

    // Time varying pixel color
    
    uv = fract(uv) * Zoom; 
    
    float Time = float(Frame%TIME) / float(TIME); 
    float TimeSecondary = float((Frame)%TIME+TIME) / float(TIME); 
    
    vec4 EdgeNegativeXP = normal((uv) - vec2(Zoom,0.), 0.01,2.1,Time*TimeMultiplier);
    vec4 EdgeNegativeYP = normal((uv) - vec2(0.0,Zoom), 0.01,2.1,Time*TimeMultiplier);
    vec4 EdgeNegativeXYP = normal((uv) - vec2(Zoom,Zoom), 0.01,2.1,Time*TimeMultiplier);
    
    vec4 EdgeNegativeXS = normal((uv) - vec2(Zoom,0.), 0.01,2.1,TimeSecondary*TimeMultiplier);
    vec4 EdgeNegativeYS = normal((uv) - vec2(0.0,Zoom), 0.01,2.1,TimeSecondary*TimeMultiplier);
    vec4 EdgeNegativeXYS = normal((uv) - vec2(Zoom,Zoom), 0.01,2.1,TimeSecondary*TimeMultiplier);

  	vec4 Primary = mix(normal(uv,0.01,2.1,Time*TimeMultiplier),EdgeNegativeXP , pow((uv.x/Zoom),Power)); 
    Primary = mix(Primary, mix(EdgeNegativeYP,EdgeNegativeXYP,pow((uv.x/Zoom),Power)), pow((uv.y/Zoom),Power)); 

    vec4 Secondary = mix(normal(uv,0.01,2.1,TimeSecondary*TimeMultiplier),EdgeNegativeXS , pow((uv.x/Zoom),Power)); 
    Secondary = mix(Secondary, mix(EdgeNegativeYS,EdgeNegativeXYS,pow((uv.x/Zoom),Power)), pow((uv.y/Zoom),Power)); 
    
    Primary.xyz = normalize(Primary.xyz); 
    Secondary.xyz = normalize(Secondary.xyz); 

    
    Color = mix(Secondary,Primary,Time); 
    
    Color.xyz = normalize(Color.xyz); 
    Color.xz = Color.xz * 0.5 + 0.5; 
    Color.w *= 0.476190476; 
    


    //fragColor.xyz = fragColor.www; 
    
  	//fragColor.xyz = mix(EdgePositiveX, fragColor.xyz, uv.x); 

    // Output to screen
    //fragColor.xyz = normal(fract(uv * 2.)*3.14, 0.01, 2.1);
}