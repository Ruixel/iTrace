#version 420

in vec2 TexCoord; 

out float CausticFactor; //<- the caustic factor 

uniform float Time; //<- fract, 0 -> start of anim, 1 -> end of anim
uniform float Divisor; //the common divisor 

const int INTERATIONS = 3; 
const float Pi = 3.14159265358929; 

void main() {

	float AnimSpan = (2.0 * Pi * Time) / Divisor; 
	
	vec2 Point = TexCoord * 2.0 * Pi - 250.0; 
	
	vec2 i = Point; 
	CausticFactor = 1.0; 
	float inten = .005; 

	for (int n = 0; n < INTERATIONS; n++) 
	{
		float t = AnimSpan * Divisor * float(n+1);
		i = Point + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		CausticFactor += 1.0/length(vec2(Point.x / (sin(i.x+t)/inten),Point.y / (cos(i.y+t)/inten)));
	}
    
   
    
	CausticFactor /= float(INTERATIONS);
	CausticFactor = 1.15-(CausticFactor*CausticFactor);
	CausticFactor = abs(CausticFactor); 
}