#version 330 

in vec2 TexCoord; 

layout(location = 0) out vec2 RefractedTC; 
layout(location = 1) out vec4 PackedUpscaleData; 

uniform sampler2D LFNormal; //<- low frequency normal 
uniform sampler2D WorldPosition; //<- world position 
uniform sampler2D Depth; //WATER depth 
uniform sampler2D TraceDepth; 
uniform mat4 ViewMatrix; 
uniform mat4 ProjectionMatrix; 
uniform vec3 CameraPosition; 

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
}

#define WATER_MQ  

#ifdef WATER_HQ 

const int TraceSteps = 10; 
const int BinarySearchSteps = 5; 

#endif 

#ifdef WATER_MQ 

//water setting is "medium" this means water tracing is still done in quarter res, but no upscaling back to full res 
//and steps are also decreased 

const int TraceSteps = 8;
const int BinarySearchSteps = 2; 

#endif 

#ifdef WATER_LQ 

//water setting is "low", this means that the water tracing is now done at 1/16th res, with no upscaling and decreased steps 
//this should make the water run on pretty much any hardware. 

const int TraceSteps = 4;
const int BinarySearchSteps = 1; //<- minimal binary search for low water 

#endif 

float SolvePartial1Pos(float a, float b, float c, float p, float q) {
	return -( a * c + p ) / ( b * c + q); 
} 

float SolvePartial1Neg(float a, float b, float c, float p, float q) {
	return (p - a * c) / (b * c - q); 
}

float SolvePartial2Pos(float a, float b, float c, float p) {
	return - (a * c + a + p) / (b * c + b); 
}

float SolvePartial2Neg(float a, float b, float c, float p) {
	return (-a * c + a - p) / (b * c - b); 
}

vec2 SolvePartial1(float a, float b, float c, float p, float q) {
	return vec2(SolvePartial1Pos(a, b, c, p, q), SolvePartial1Neg(a, b, c, p, q)); 
}
vec2 SolvePartial2(float a, float b, float c, float p) {
	return vec2(SolvePartial2Pos(a,b,c,p), SolvePartial2Neg(a,b,c,p)); 
}

float CorrectTraverse(float a, float b) {
	return a < 0 ? b : b < 0 ? a : min(a,b); //properly handles non-correct intersections (i:e t<0) 
}

float CorrectTraverse3(float a, float b, float c) {
	return CorrectTraverse(CorrectTraverse(a,b),c); 
}

float SolveIntersectionLength(vec3 Direction, vec3 Origin, mat4 ProjectionMatrix) { //analytical solution to the traversal length 

	vec2 Tx = SolvePartial1(Origin.x, Direction.x, ProjectionMatrix[0][0], Origin.z, Direction.z); 
	vec2 Ty = SolvePartial1(Origin.y, Direction.y, ProjectionMatrix[1][1], Origin.z, Direction.z); 
	vec2 Tz = SolvePartial2(Origin.z, Direction.z, ProjectionMatrix[2][2], ProjectionMatrix[2][3]); 

	return CorrectTraverse3(CorrectTraverse(Tx.x, Tx.y), CorrectTraverse(Ty.x, Ty.y), CorrectTraverse(Tz.x, Tz.y)); 

}

//The base depth is used to compute expected ray thickness, thus allowing for more accurate ray intersection on the primary step -> -> 
vec2 ScreenSpaceTrace(vec3 Direction, vec3 Origin) {

	
	//project everything to view space 

	vec3 ViewDirection = vec3(mat4(mat3(ViewMatrix)) * vec4(Direction, 1.0)); 
	vec3 ViewOrigin = vec3(ViewMatrix * vec4(Origin, 1.0)); 
	//^ I think these are okay? 

	float MaxTraversal = 10.0; //<- for now, hardcoded. 
	//						^ this constant will be changed to the analytical intersection later 

	vec3 RayStep = (ViewDirection * MaxTraversal) / vec3(TraceSteps); 

	vec3 RayPoint = ViewOrigin + RayStep; 



	vec4 BaseProj = ProjectionMatrix * vec4(ViewOrigin, 1.0); 

	float PreviousZ = LinearDepth((BaseProj.z / BaseProj.w) * 0.5 + 0.5);
	
	for(int Step = 0; Step < TraceSteps; Step++) {
	


		vec4 ProjectedPoint = ProjectionMatrix * vec4(RayPoint, 1.0); 
		ProjectedPoint.xyz /= ProjectedPoint.w; 

		if(abs(ProjectedPoint.x) > 1.0 || abs(ProjectedPoint.y) > 1.0) 
			return TexCoord; //<- no suitable intersection point found 
		
		float FetchedZ = texture(TraceDepth, ProjectedPoint.xy * 0.5 + 0.5).x; 
		float CurrentZ = LinearDepth(ProjectedPoint.z * 0.5 + 0.5); 


		if(FetchedZ < CurrentZ  && abs(FetchedZ-CurrentZ) < abs(PreviousZ-CurrentZ)) {
		
			//do a bit of a binary search! 

			vec3 BinarySearchStep = 0.5 * RayStep; 

			RayPoint -= BinarySearchStep; //<- given by the previous boolean statement 

			for(int BinaryStep = 0; BinaryStep < BinarySearchSteps; BinaryStep++) {
			
				BinarySearchStep *= 0.5; 

				ProjectedPoint = ProjectionMatrix * vec4(RayPoint, 1.0); 
				ProjectedPoint.xyz /= ProjectedPoint.w;  

				float FetchedZ = texture(TraceDepth, ProjectedPoint.xy * 0.5 + 0.5).x; 
				float CurrentZ = LinearDepth(ProjectedPoint.z * 0.5 + 0.5); 

				if(FetchedZ < CurrentZ) 
					RayPoint -= BinarySearchStep; 
				else 
					RayPoint += BinarySearchStep; 
				


			}

			return ProjectedPoint.xy * 0.5 + 0.5; 

		}

		PreviousZ = CurrentZ; 

		RayPoint += RayStep; 

	}

	return TexCoord; 

}


void main() {

	vec3 Normal = texture(LFNormal, TexCoord).xyz; 
	vec3 WorldPos = texture(WorldPosition, TexCoord).xyz; 
	vec3 Incident = normalize(WorldPos - CameraPosition); 
	vec3 Direction = refract(Incident, Normal, 1.0/1.1); 
	
	RefractedTC = ScreenSpaceTrace(Direction, WorldPos); 
}