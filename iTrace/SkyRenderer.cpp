#include "SkyRenderer.h"
#include "AtmosphereRenderer.h"
#include <iostream>
#include "Profiler.h"

namespace iTrace {

	namespace Rendering {

		void Move(Vector3f& Pos, float Speed, float RotationX, float RotationY) {
			Pos.x -= (cos(RotationY * (PI / 180.)) * cos(RotationX * (PI / 180.))) * Speed;
			Pos.y += sin(RotationX * (PI / 180.)) * Speed;
			Pos.z -= (sin(RotationY * (PI / 180.)) * cos(RotationX * (PI / 180.))) * Speed;
		}

		void SkyRendering::PrepareSkyRenderer(Window& Window)
		{
			SkyIncident = MultiPassFrameBufferObject(Window.GetResolution() / 16, 2, { GL_RGB16F, GL_RGB16F }, false, false);
			SkyCube = CubeMultiPassFrameBufferObject(Vector2i(16), 2, { GL_RGB16F, GL_RGB16F }, false, { false, true });



			SkyIncidentShader = Shader("Shaders/SkyIncidentShader");
			SkyCubeShader = Shader("Shaders/SkyCubeShader");
			ShadowDeferred = Shader("Shaders/ShadowDeferred"); 
			ShadowDeferredPlayer = Shader("Shaders/PlayerModelShader");
			ShadowTransparentDeferred = Shader("Shaders/DeferredTransparent"); 

			//HemisphericalShadowMapCopy = Shader("Shaders/HemisphericalShadowMapCopy"); 
			
			for (int i = 0; i < 5; i++) {
				ShadowMaps[i] = FrameBufferObject(Vector2i(SHADOWMAP_RES) * 2,GL_R32F);
				RefractiveShadowMaps[i] = FrameBufferObject(Vector2i(SHADOWMAP_RES) * 2, GL_RGBA8,false); 
				RefractiveShadowMapsDepth[i] = FrameBufferObject(Vector2i(SHADOWMAP_RES) * 2, GL_RGBA8);

				ProjectionMatrices[i] = Core::ShadowOrthoMatrix(Ranges[i], 100.f, 2500.f);
				ProjectionMatricesRaw[i] = ProjectionMatrices[i]; 
			}

			TemporaryHemiSphericalShadowMap = FrameBufferObject(Vector2i(SHADOWMAP_RES), GL_R8); 

			glGenTextures(1, &HemiSphericalShadowMap);
			glBindTexture(GL_TEXTURE_2D_ARRAY, HemiSphericalShadowMap);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, SHADOWMAP_RES, SHADOWMAP_RES, TotalSplits, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

			for (int i = 0; i < TotalSplits; i++) {

				glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
					0,
					0, 0, i,
					SHADOWMAP_RES, SHADOWMAP_RES, 1,
					GL_DEPTH_ATTACHMENT,
					GL_FLOAT,
					NULL);

			}

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			glFinish();


			glGetError(); 
			
			glGenFramebuffers(1, &HemiSphericalShadowFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, HemiSphericalShadowFBO);
		
			/*glGenRenderbuffers(1, &HemiSphericalShadowMapDepthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, HemiSphericalShadowMapDepthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, SHADOWMAP_RES, SHADOWMAP_RES);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
				HemiSphericalShadowMapDepthBuffer);
				*/
			glDrawBuffer(GL_NONE); 
			glReadBuffer(GL_NONE);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			std::cout << "Error: " << glGetError() << '\n'; 

			CreateDirectionMatrices(); 

			HemiProjectionMatrix = Core::ShadowOrthoMatrix(250.0f, 100.f, 2500.f);

			ShadowTransparentDeferred.Bind(); 

			ShadowTransparentDeferred.SetUniform("TextureData", 0);
			ShadowTransparentDeferred.SetUniform("TextureExData", 1);
			ShadowTransparentDeferred.SetUniform("OpacityTextures", 2);

			ShadowTransparentDeferred.UnBind(); 

		}

		void SkyRendering::RenderSky(Window& Window, Camera& Camera, WorldManager& World, Shader& RefractiveShader)
		{

		

			glEnable(GL_DEPTH_TEST); 

			if(Window.GetFrameCount()%2)
				UpdateHemisphericalShadowMap(Window, Camera, World); 
			else 
				UpdateShadowMap(Window, Camera, World, RefractiveShader);

			glDisable(GL_DEPTH_TEST); 

			Profiler::SetPerformance("Shadow map updates"); 


			SkyCube.Bind();

			SkyCubeShader.Bind();

			SkyCubeShader.SetUniform("SunDirection", Orientation);

			for (int i = Window.GetFrameCount()%6; i < (Window.GetFrameCount() % 6)+1; i++) {


				SkyCubeShader.SetUniform("ViewMatrix", CubeProjection * CubeViews[i]);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, SkyCube.Texture[0], 0);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, SkyCube.Texture[1], 0);

				DrawPostProcessCube();

			}

			SkyCube.UnBind(Window);

			SkyCubeShader.UnBind();

			SkyIncidentShader.Bind();

			SkyIncident.Bind();

			SkyIncidentShader.SetUniform("SunDirection", Orientation);

			SkyIncidentShader.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));

			DrawPostProcessQuad();

			SkyIncident.UnBind(Window);

			SkyIncidentShader.UnBind();

			Profiler::SetPerformance("Sky updates");


		}

		void SkyRendering::ReloadSky()
		{
			SkyIncidentShader.Reload("Shaders/SkyIncidentShader");
			SkyCubeShader.Reload("Shaders/SkyCubeShader");
		}

		void SkyRendering::SetTimeOfDay(float TimeOfDay)
		{

/*		float TimeNormalized = glm::fract(TimeOfDay / 86400.); 

			//TimeNormalized = int(TimeNormalized * 32) / 32.f; 

			float TimeAngles = 360. * TimeNormalized;
		
			Direction.x = TimeAngles; 
			Direction.y = TimeAngles / 2.f;

			Orientation = Vector3f(0.0); 

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);

			SunColor = 9.0f * Atmospheric::GetSunColor(Orientation); 


			SkyColor = (Atmospheric::GetSkyColor(Vector3f(0.0, 1.0, 0.0))); */

		}
		void SkyRendering::GetTimeOfDayDirection(float TimeOfDay, Vector2f& Direction, Vector3f& Orientation)
		{
			float TimeNormalized = glm::fract(TimeOfDay / 86400.);

			float TimeAngles = 360. * TimeNormalized;

			Direction.x = TimeAngles;
			Direction.y = TimeAngles / 2.f;

			Orientation = Vector3f(0.0);

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);
		}
		void SkyRendering::UpdateDirection()
		{
			

			Orientation = Vector3f(0.0);

			Move(Orientation, 1.0, Direction.x, Direction.y - 90.0);

			SunColor = 9.0f * Atmospheric::GetSunColor(Orientation); 

			SkyColor = (Atmospheric::GetSkyColor(Orientation, Vector3f(0.0, 1.0, 0.0)));
		}
		void SkyRendering::UpdateShadowMap(Window& Window, Camera& Camera, WorldManager& World, Shader& RefractiveShader)
		{

			iTrace::Camera ShadowCamera; 

			int ToUpdate = UpdateQueue[(Window.GetFrameCount()/2) % 13];
			
			if (ToUpdate != 4)
				ViewMatrices[ToUpdate] = Core::ViewMatrix(Camera.Position + Orientation * 500.0f, Vector3f(Direction.x, Direction.y, 0.));
			else
				ViewMatrices[ToUpdate] = Core::ViewMatrix(Camera.Position + Vector3f(0.0, 500.0, 0.0), Vector3f(90.0, 0.0, 0.0)); 



			ProjectionMatrices[ToUpdate] = ProjectionMatricesRaw[ToUpdate];

			ShadowCamera.Project = ProjectionMatrices[ToUpdate]; 
			ShadowCamera.View = ViewMatrices[ToUpdate]; 
			

			ShadowMaps[ToUpdate].Bind();

			ShadowDeferred.Bind(); 

			World.RenderWorld(ShadowCamera, ShadowDeferred);

			ShadowDeferred.UnBind();

			ShadowDeferredPlayer.Bind(); 

			ShadowDeferredPlayer.SetUniform("CameraPosition", Camera.Position); 
			ShadowDeferredPlayer.SetUniform("IdentityMatrix", ShadowCamera.Project * ShadowCamera.View); 

			//DrawPostProcessCube(); 

			ShadowDeferredPlayer.UnBind();

			ShadowTransparentDeferred.Bind();

			ShadowTransparentDeferred.SetUniform("Time", Window.GetTimeOpened());

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockDataTexture());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetTextureExtensionData());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(6));

			World.RenderWorldTransparent(ShadowCamera, ShadowTransparentDeferred);

			ShadowTransparentDeferred.UnBind();

			ShadowMaps[ToUpdate].UnBind();

			RefractiveShader.Bind();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(0));

			ShadowMaps[ToUpdate].BindDepthImage(1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockDataTexture());

			RefractiveShader.SetUniform("IsPrimary", true); 

			RefractiveShadowMapsDepth[ToUpdate].Bind(); 

			World.RenderWorldRefractive(ShadowCamera, RefractiveShader);

			RefractiveShadowMapsDepth[ToUpdate].UnBind(); 


			RefractiveShadowMapsDepth[ToUpdate].BindDepthImage(3);


			RefractiveShader.SetUniform("IsShadow", true);
			RefractiveShader.SetUniform("zNear", 100.0f); 
			RefractiveShader.SetUniform("zFar", 2500.0f);
			RefractiveShader.SetUniform("CameraPos", Camera.Position + Orientation * 500.0f); 


			glClearColor(1.0, 1.0, 1.0, 1.0);

			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);

			glBlendFunc(GL_DST_COLOR, GL_ZERO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetCausticTextureArray());

			RefractiveShadowMaps[ToUpdate].Bind();

			World.RenderWorldRefractive(ShadowCamera, RefractiveShader);

			RefractiveShadowMaps[ToUpdate].UnBind();

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_BLEND);


			glClearColor(0.0, 0.0, 0.0, 0.0);

			RefractiveShader.UnBind();

			RefractiveShader.SetUniform("IsShadow", false);




		}
		void SkyRendering::UpdateHemisphericalShadowMap(Window& Window, Camera& Camera, WorldManager& World)
		{
			int ToUpdate = (Window.GetFrameCount()/2) % TotalSplits;

			glEnable(GL_DEPTH_TEST);

			
			iTrace::Camera ShadowCamera;
			
			HemiViewMatrices[ToUpdate] = glm::translate(RawHemiViewMatrices[ToUpdate], -Camera.Position); 

			ShadowCamera.Project = HemiProjectionMatrix;
			ShadowCamera.View = HemiViewMatrices[ToUpdate];

			ShadowCamera.View = HemiViewMatrices[ToUpdate];


			glClearDepth(1.0); 

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 

			ShadowDeferred.Bind();


			glBindFramebuffer(GL_FRAMEBUFFER, HemiSphericalShadowFBO); 

			glViewport(0, 0, SHADOWMAP_RES, SHADOWMAP_RES);

			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, HemiSphericalShadowMap, 0, ToUpdate);
			glClear(GL_DEPTH_BUFFER_BIT);


			World.RenderWorld(ShadowCamera, ShadowDeferred);

			//DrawPostProcessQuad(); 


			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			ShadowDeferred.UnBind();

			glClearDepth(1.0);

		}
		void SkyRendering::CreateDirectionMatrices()
		{
			float BaseRotationY = 20.0;
			float RotationAddonY = (75.0 - BaseRotationY) / float(YSplits);

			for (int RotationY = 0; RotationY < YSplits; RotationY++) {

				float YRotation = BaseRotationY + RotationAddonY * RotationY;

				float RotationAddonX = (360.0) / float(XSplits[RotationY]);

				for (int RotationX = 0; RotationX < XSplits[RotationY]; RotationX++) {

					float XRotation = RotationAddonX * RotationX;

					Vector3f Direction = Core::SphericalCoordinate(YRotation, XRotation, true);

					HemiDirections.push_back(Direction);

					Matrix4f ViewMatrix = Core::ViewMatrix(Direction * 500.f, Vector3f(YRotation, XRotation + 90.0, 0.));

					RawHemiViewMatrices.push_back(ViewMatrix);
					HemiViewMatrices.push_back(ViewMatrix); 
				}

			}
		}
	}
}

