#include "Framebuffer.h"
#include <iostream>


GLenum GetCorresponding(int x) {

	if (x == GL_RGBA32UI) {
		return GL_RGBA_INTEGER;
	}
	if (x == GL_RGBA32F || x == GL_RGBA16F || x == GL_RGBA8)
		return GL_RGBA;
	if (x == GL_RGB32F || x == GL_RGB16F || x == GL_RGB8)
		return GL_RGB;
	if (x == GL_RG32F || x == GL_RG16F || x == GL_RG8)
		return GL_RG;
	if (x == GL_R32F || x == GL_R16F || x == GL_R8)
		return GL_RED;

}

std::string GetFormatText(int X) {

	if (X == GL_RGBA32F)
		return "GL_RGBA32F";
	if (X == GL_RGBA16F)
		return "GL_RGBA16F";
	if (X == GL_RGBA8)
		return "GL_RGBA8";
	if (X == GL_RGB32F)
		return "GL_RGB32F";
	if (X == GL_RGB16F)
		return "GL_RGB16F";
	if (X == GL_RGB8)
		return "GL_RGB8";
	if (X == GL_R32F)
		return "GL_R32F";
	if (X == GL_R16F)
		return "GL_R16F";
	if (X == GL_R8)
		return "GL_R8";
	if (X == GL_RGBA32UI)
		return "GL_RGBA32UI";

	if (X == GL_RGBA)
		return "GL_RGBA";
	if (X == GL_RGB)
		return "GL_RGB";
	if (X == GL_RED)
		return "GL_RED";
	if (X == GL_RGBA_INTEGER)
		return "GL_RGBA_INTEGER";

}


iTrace::Rendering::FrameBufferObject::FrameBufferObject(glm::ivec2 Resolution, int Format, bool HasDepth, bool generatemip)
	: GenerateMip(generatemip), FrameBuffer(0), ColorBuffer(0), DepthBuffer(0), Resolution(Resolution) {
	glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenTextures(1, &ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, Format, Resolution.x, Resolution.y, 0, GetCorresponding(Format), Format == GL_RGBA32UI ? GL_UNSIGNED_INT : GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generatemip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer, 0); //attach it to the frame buffer
																								 //and depth buff
	if (HasDepth) {
		glGenTextures(1, &DepthBuffer);
		glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generatemip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthBuffer, 0);
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

iTrace::Rendering::FrameBufferObject::FrameBufferObject()
	: Resolution(0), FrameBuffer(0), ColorBuffer(0), DepthBuffer(0), GenerateMip(false) {
}

void iTrace::Rendering::FrameBufferObject::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glViewport(0, 0, Resolution.x, Resolution.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void iTrace::Rendering::FrameBufferObject::UnBind(Window Window) {
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	if (GenerateMip) {
		glBindTexture(GL_TEXTURE_2D, ColorBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, NULL);
	}
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);
}

void iTrace::Rendering::FrameBufferObject::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	if (GenerateMip) {
		glBindTexture(GL_TEXTURE_2D, ColorBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, NULL);
	}
}

iTrace::Rendering::MultiPassFrameBufferObject::MultiPassFrameBufferObject(glm::ivec2 Resolution, int stages, std::vector<int> Formats, bool HasDepth, bool generatemip) :
	Resolution(Resolution), FrameBuffer(0), DepthBuffer(0), GenerateMip(generatemip), ColorBuffers{} {
	glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	//now for the color buffer 
	for (int i = 0; i < stages; i++) {
		ColorBuffers.push_back(0);
		glGenTextures(1, &ColorBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, ColorBuffers[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, Formats[i], Resolution.x, Resolution.y, 0, GetCorresponding(Formats[i]), GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generatemip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, ColorBuffers[i], 0); //attach it to the frame buffer
	}
	//and depth buffer



	if (HasDepth) {
		glGenTextures(1, &DepthBuffer);
		glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthBuffer, 0);
	}

	GLenum DrawBuffers[30];
	for (int i = 0; i < stages; i++)
		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

	glDrawBuffers(stages, DrawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

iTrace::Rendering::MultiPassFrameBufferObject::MultiPassFrameBufferObject() :
	Resolution(0), FrameBuffer(0), DepthBuffer(0), GenerateMip(false), ColorBuffers{} {
}

void iTrace::Rendering::MultiPassFrameBufferObject::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glViewport(0, 0, Resolution.x, Resolution.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void iTrace::Rendering::MultiPassFrameBufferObject::UnBind(Window Window) {
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);

	if (GenerateMip) {
		glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, NULL);

		for (int buffer = 0; buffer < ColorBuffers.size(); buffer++) {


			glBindTexture(GL_TEXTURE_2D, ColorBuffers[buffer]);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, NULL);

		}

		glBindTexture(GL_TEXTURE_2D, NULL);
	}
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);

}

void iTrace::Rendering::MultiPassFrameBufferObject::UnBind()
{
	if (GenerateMip) {
		glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, NULL);

		for (int buffer = 0; buffer < ColorBuffers.size(); buffer++) {


			glBindTexture(GL_TEXTURE_2D, ColorBuffers[buffer]);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, NULL);

		}

		glBindTexture(GL_TEXTURE_2D, NULL);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
}

unsigned int PPQuadVBO, PPQuadVAO, PPCubeVBO, PPCubeVAO, WaterVAO, WaterVBO, PlayerVAO, PlayerVBO;

float vertices[] = {
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

float WaterVertices[] = {
	-100.0f, 40.0f, 100.0f, 0.0f, 1.0f, 0.0, 1.0,0.0,
	-100.0f, 40.0f, -100.0f, 0.0f, 0.0f, 0.0, 1.0,0.0,
	100.0f, 40.0f, 100.0f, 1.0f, 1.0f, 0.0, 1.0, 0.0,
	100.0f, 40.0f,-100.0f, 1.0f, 0.0f, 0.0, 1.0, 0.0
};

float CubeVertices[] = {
	// back face

	//	VERTEX				NORMAL				TEX COORD
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
	 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
	// front face
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	// left face
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	// right face
	 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
	 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
	// bottom face
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
	 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	// top face
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
	 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
};

void iTrace::Rendering::PreparePostProcess() {
	

	glGenVertexArrays(1, &PPQuadVAO);
	glGenBuffers(1, &PPQuadVBO);
	glBindVertexArray(PPQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, PPQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
		reinterpret_cast<void*>(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	glGenVertexArrays(1, &PPCubeVAO);
	glGenBuffers(1, &PPCubeVBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, PPCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(PPCubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &WaterVAO);
	glGenBuffers(1, &WaterVBO);

	glBindBuffer(GL_ARRAY_BUFFER, WaterVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(WaterVertices), WaterVertices, GL_STATIC_DRAW);

	glBindVertexArray(WaterVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	std::vector<float> PlayerVertexData = std::vector<float>(0);

	auto AddCube = [&](Vector3f Origin, Vector3f Size) {
		for (int Tri = 0; Tri < 288; Tri += 8) {

			int _idx = Tri;

			Vector3f Vertex = Vector3f(CubeVertices[_idx++], CubeVertices[_idx++], CubeVertices[_idx++]);
			Vector3f Normal = Vector3f(CubeVertices[_idx++], CubeVertices[_idx++], CubeVertices[_idx++]);
			Vector2f TexCoord = Vector2f(CubeVertices[_idx++], CubeVertices[_idx++]);

			Vertex = Vertex * Size + Origin;

			PlayerVertexData.push_back(Vertex.x);
			PlayerVertexData.push_back(Vertex.y);
			PlayerVertexData.push_back(Vertex.z);
			PlayerVertexData.push_back(Normal.x);
			PlayerVertexData.push_back(Normal.y);
			PlayerVertexData.push_back(Normal.z);
			PlayerVertexData.push_back(TexCoord.x);
			PlayerVertexData.push_back(TexCoord.y);



		}
	}; 

	//LEGS: 
	AddCube(Vector3f(0.014693, 0.302329, 0.176809), Vector3f(0.1,0.3,0.1));
	AddCube(Vector3f(0.014693, 0.302329, -0.176809), Vector3f(0.1, 0.3, 0.1));

	//BODY: 
	AddCube(Vector3f(0.025274, 0.988038,0.0), Vector3f(0.203, 0.4, 0.326));

	//ARMS: 

	AddCube(Vector3f(0.014693, 0.718496, 0.436037), Vector3f(0.121, 0.522, 0.121));
	AddCube(Vector3f(0.014693, 0.718496, -0.436037), Vector3f(0.121, 0.522, 0.121));

	//HEAD: 

	AddCube(Vector3f(0.0, 1.56763, 0.0), Vector3f(0.177));


	glGenVertexArrays(1, &PlayerVAO);
	glGenBuffers(1, &PlayerVBO);

	glBindBuffer(GL_ARRAY_BUFFER, PlayerVBO);
	glBufferData(GL_ARRAY_BUFFER, PlayerVertexData.size() * sizeof(float), PlayerVertexData.data(), GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(PlayerVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);




}

void iTrace::Rendering::DrawPostProcessQuad() {
	glBindVertexArray(PPQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
void iTrace::Rendering::DrawPostProcessCube() {
	glBindVertexArray(PPCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void iTrace::Rendering::DrawPlayerModel()
{
	glBindVertexArray(PlayerVBO);
	glDrawArrays(GL_TRIANGLES, 0, 216);
	glBindVertexArray(0);
}

void iTrace::Rendering::DrawWaterQuad() {
	glBindVertexArray(WaterVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void iTrace::Rendering::DrawPostProcessQuadInstanced(unsigned int InstanceCount)
{
	glBindVertexArray(PPQuadVAO);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, InstanceCount);
	glBindVertexArray(0);
}

iTrace::Rendering::FrameBufferObjectPreviousData::FrameBufferObjectPreviousData(Vector2i Resolution, int Format, bool HasDepth, bool generatemip) :
	Buffers{ FrameBufferObject(Resolution,Format,HasDepth,generatemip),FrameBufferObject(Resolution,Format,HasDepth,generatemip) }, Buffer(0) {

}

void iTrace::Rendering::FrameBufferObjectPreviousData::Bind(bool Swap) {
	if (Swap)
		Buffer = !Buffer;

	Buffers[Buffer].Bind();
}

void iTrace::Rendering::FrameBufferObjectPreviousData::BindImage(int Target) {
	Buffers[Buffer].BindImage(Target);
}

void iTrace::Rendering::FrameBufferObjectPreviousData::BindDepthImage(int Target) {
	Buffers[Buffer].BindDepthImage(Target);
}

void iTrace::Rendering::FrameBufferObjectPreviousData::BindImagePrevious(int Target) {

	Buffers[!Buffer].BindImage(Target);
}

void iTrace::Rendering::FrameBufferObjectPreviousData::BindDepthImagePrevious(int Target) {
	Buffers[!Buffer].BindDepthImage(Target);
}

void iTrace::Rendering::FrameBufferObjectPreviousData::UnBind(Window& Window) {
	Buffers[Buffer].UnBind(Window);
}

void iTrace::Rendering::FrameBufferObjectPreviousData::Swap() {
	Buffer = !Buffer;
}


void iTrace::Rendering::CubeMultiPassFrameBufferObject::Bind() {
	glViewport(0, 0, Resolution.x, Resolution.y);

	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
}

void iTrace::Rendering::CubeMultiPassFrameBufferObject::UnBind(Window Window) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);

	for (int CalcMipIdx = 0; CalcMipIdx < CalculateMips.size(); CalcMipIdx++) {

		if (CalculateMips[CalcMipIdx]) {

			glBindTexture(GL_TEXTURE_CUBE_MAP, Texture[CalcMipIdx]); 
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP); 
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		}


	}



}

void iTrace::Rendering::CubeMultiPassFrameBufferObject::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	for (int CalcMipIdx = 0; CalcMipIdx < CalculateMips.size(); CalcMipIdx++) {

		if (CalculateMips[CalcMipIdx]) {

			glBindTexture(GL_TEXTURE_CUBE_MAP, Texture[CalcMipIdx]);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		}


	}
}


iTrace::Rendering::CubeMultiPassFrameBufferObject::CubeMultiPassFrameBufferObject(Vector2i Resolution, int Targets, std::vector<int> Formats, bool HasDepth, std::vector<bool> CalculateMips)
{
	this->Resolution = Resolution;
	this->CalculateMips = CalculateMips; 
	glGenFramebuffers(1, &FrameBuffer);
	// create depth cubemap texture


	std::vector<unsigned int> ColorBuffers;

	if (HasDepth) {
		glGenTextures(1, &DepthTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, DepthTexture);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	}

	for (int j = 0; j < Targets; j++) {
		Texture.push_back(0);
		glGenTextures(1, &Texture[j]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, Texture[j]);

		bool CalculateMip = false; 
		if (j < CalculateMips.size())
			CalculateMip = CalculateMips[j]; 

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, CalculateMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, CalculateMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, Formats[j], Resolution.x, Resolution.y, 0, GetCorresponding(Formats[j]), GL_FLOAT, NULL);

			//std::cout << GetFormatText(Formats[j]) << ' ' << GetFormatText(GetCorresponding(Formats[j])) << '\n';
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, Texture[j], 0); //attach it to the frame buffer

		ColorBuffers.push_back(GL_COLOR_ATTACHMENT0 + j);

	}



	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenRenderbuffers(1, &DepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y);
	glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		DepthBuffer);



	glDrawBuffers(ColorBuffers.size(), &ColorBuffers[0]);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void iTrace::Rendering::CubeFrameBufferObject::Bind() {
	glViewport(0, 0, Resolution.x, Resolution.y);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
}

void iTrace::Rendering::CubeFrameBufferObject::UnBind(Window Window) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Window.GetResolution().x, Window.GetResolution().y);
}

iTrace::Rendering::CubeFrameBufferObject::CubeFrameBufferObject(Vector2i Resolution, int Format, bool HasDepth) {
	this->Resolution = Resolution;
	glGenFramebuffers(1, &FrameBuffer);
	// create depth cubemap texture

	if (HasDepth) {
		glGenTextures(1, &DepthTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, DepthTexture);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	}

	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, Format, Resolution.x, Resolution.y, 0, GetCorresponding(Format), GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0); //attach it to the frame buffer
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);



	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenRenderbuffers(1, &DepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Resolution.x, Resolution.y);
	glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		DepthBuffer);



	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::MultiPassFrameBufferObjectPreviousData(glm::ivec2 Resolution, int stages, std::vector<int> Formats, bool HasDepth, bool generatemip) :

	Buffers{ MultiPassFrameBufferObject(Resolution,stages,Formats,HasDepth,generatemip),MultiPassFrameBufferObject(Resolution,stages,Formats,HasDepth,generatemip) }


{
}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::Bind(bool Swap) {
	if (Swap)
		Buffer = !Buffer;

	Buffers[Buffer].Bind();
}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::BindImage(int Target, int TargetImage) {
	Buffers[Buffer].BindImage(Target, TargetImage);
}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::BindDepthImage(int Target) {
	Buffers[Buffer].BindDepthImage(Target);
}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::BindImagePrevious(int Target, int TargetImage) {
	Buffers[!Buffer].BindImage(Target, TargetImage);
}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::BindDepthImagePrevious(int Target) {
	Buffers[!Buffer].BindDepthImage(Target);
}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::UnBind(Window& Window) {
	Buffers[Buffer].UnBind(Window);
}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::UnBind()
{

}

void iTrace::Rendering::MultiPassFrameBufferObjectPreviousData::Swap() {
	Buffer = !Buffer;
}

iTrace::Rendering::FrameBufferObjectShadow::FrameBufferObjectShadow(glm::ivec2 Resolution)
	: FrameBuffer(0), DepthBuffer(0), Resolution(Resolution) {
	glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenTextures(1, &ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Resolution.x, Resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &DepthBuffer);
	glBindTexture(GL_TEXTURE_2D, DepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Resolution.x, Resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthBuffer, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

iTrace::Rendering::FrameBufferObjectShadow::FrameBufferObjectShadow()
	: Resolution(0), FrameBuffer(0), DepthBuffer(0) {
}

void iTrace::Rendering::FrameBufferObjectShadow::Bind()
{
	glViewport(0, 0, Resolution.x, Resolution.y);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
}

void iTrace::Rendering::FrameBufferObjectShadow::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
