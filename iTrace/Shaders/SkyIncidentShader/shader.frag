#version 330

layout(location = 0) out vec3 Sky; 
layout(location = 1) out vec3 SkyRayleigh; 

in vec2 TexCoord;  
uniform mat4 IncidentMatrix; 
uniform vec3 SunDirection; 

//test implementation 

const vec3 PlayerOrigin = vec3(0,6372e3,0); 
const float PlanetRadius = 6371e3; 
const float AtmosphereRadius = 6471e3; 
const int CloudSteps = 128; //temporary, way too large



//current implementation is from: 
//https://github.com/wwwtyro/glsl-atmosphere/blob/master/index.glsl
//this was strictly so that I could get the system up and running 
//the plan is for this to be replaced with a custom solution asap 

#define PI 3.141592
#define iSteps 16
#define jSteps 8

vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}


vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g, out vec3 rayleigh) {
    // Normalize the sun and view directions.

	
    pSun = normalize(pSun);
    r = normalize(r);

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) return vec3(0.);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0,0,0);
    vec3 totalMie = vec3(0,0,0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++) {

        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        // Increment the primary ray time.
        iTime += iStepSize;

    }

    // Calculate and return the final color.
	    rayleigh = iSun * 9000.0 * mix(vec3(1, 0.721568627, 0.0745098039),vec3(1),0.55f)* pow(pMie/3.0,140.0)*2000.0 * kMie * totalMie; 

  //  rayleigh = iSun * (pow(pMie * kMie,1.3) * totalMie)*12.0; 
   // rayleigh = pow(rayleigh/30.0, vec3(4.0))*30.0; 
    return iSun * (pRlh * kRlh * totalRlh); 


}







void main() {

	vec3 Incident = normalize(vec3(IncidentMatrix * vec4(TexCoord * 2. - 1., 1., 1.))); 
	Sky = atmosphere(
       Incident,           // normalized ray direction
        PlayerOrigin,               // ray origin
        SunDirection,                        // position of the sun
        100.0,                           // intensity of the sun
        PlanetRadius,                         // radius of the planet in meters
        AtmosphereRadius,                         // radius of the atmosphere in meters
        vec3(5.5e-6, 13.0e-6, 22.4e-6) * vec3(0.2,0.3,1.0), // Rayleigh scattering coefficient
        21e-6,                          // Mie scattering coefficient
        8e3,                            // Rayleigh scale height
        1.2e3,                          // Mie scale height
        0.758,                           // Mie preferred scattering direction
        SkyRayleigh
    );
   
}