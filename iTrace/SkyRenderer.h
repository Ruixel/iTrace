#pragma once

#include "FrameBuffer.h"
#include "Shader.h"
#include "Camera.h"
#include "WorldManager.h"

namespace iTrace {


	namespace Rendering {

		const float Ranges[4] = { 7.5, 30.0, 90.0, 300.0 };
		const int SHADOWMAP_RES = 1024; 
		const unsigned char UpdateQueue[] = { 0,1,0,2,0,1,0,2,0,1,0,3 };
		const int YSplits = 3; 
		const int XSplits[] = { 24, 16, 8 }; 
		const int TotalSplits = 48; 



		struct SkyRendering {


			MultiPassFrameBufferObject SkyIncident; 
			CubeMultiPassFrameBufferObject SkyCube; 
			FrameBufferObject ShadowMaps[4];
			Matrix4f ViewMatrices[4], ProjectionMatrices[4], ProjectionMatricesRaw[4]; 
			FrameBufferObject TemporaryHemiSphericalShadowMap;

			Shader SkyIncidentShader, SkyCubeShader, ShadowDeferred, ShadowTransparentDeferred, ShadowDeferredPlayer, HemisphericalShadowMapCopy; 

			std::vector<Vector3f> HemiDirections;
			std::vector<Matrix4f> HemiViewMatrices, RawHemiViewMatrices;
			Matrix4f HemiProjectionMatrix;

			unsigned int HemiSphericalShadowMap;
			unsigned int HemiSphericalShadowFBO, HemiSphericalShadowMapDepthBuffer; 

			void PrepareSkyRenderer(Window & Window); 
			void RenderSky(Window& Window, Camera & Camera, WorldManager & World); 
			void ReloadSky(); 




			//TimeOfDay is measured in seconds 
			void SetTimeOfDay(float TimeOfDay); 
			void GetTimeOfDayDirection(float TimeOfDay, Vector2f& Direction, Vector3f& Orientation); 
			void UpdateDirection(); 

			Vector2f Direction; 
			Vector3f Orientation; 
			Vector3f SunColor; 
			Vector3f SkyColor; 
		private: 

			void UpdateShadowMap(Window & Window, Camera& Camera, WorldManager& World);
			void UpdateHemisphericalShadowMap(Window& Window, Camera& Camera, WorldManager& World); 
			void CreateDirectionMatrices(); 

		};

	}

}