#pragma once
#include "Core.h"
#include <vector>

namespace iTrace {
	namespace Rendering {
		unsigned int LoadShader(unsigned int ShaderType, const std::string& File, unsigned int& Buffer, unsigned int BaseID, bool ReCreate = true, std::string Inject = "");

		struct Shader {
			unsigned int VertexShader = 0;
			unsigned int FragmentShader = 0;
			unsigned int GeometryShader = 0;

			unsigned int VertexBuffer = 0, FragmentBuffer = 0, GeometryBuffer = 0;
			unsigned int VertexID = 0, FragmentID = 0, GeometryID = 0;

			unsigned int ShaderID = 0;

			Shader(const std::string& vertex, const std::string& fragment, std::string Inject, int _x);
			Shader(const std::string& vertex, const std::string& geometry, const std::string& fragment);
			Shader(const std::string& Directory, bool HasGeometryShader = false, std::string Inject = "");
			Shader();
			void Bind();
			void UnBind();

			void SetUniform(const std::string& Name, int Value) const;
			void SetUniform(const std::string& Name, size_t Value) const;
			void SetUniform(const std::string& Name, float Value) const;
			void SetUniform(const std::string& Name, bool Value) const;
			void SetUniform(const std::string& Name, Vector2f Value) const;
			void SetUniform(const std::string& Name, Vector3f Value) const;
			void SetUniform(const std::string& Name, Vector4f Value) const;
			void SetUniform(const std::string& Name, Vector2i Value) const;
			void SetUniform(const std::string& Name, Vector3i Value) const;
			void SetUniform(const std::string& Name, Vector4i Value) const;
			void SetUniform(const std::string& Name, Matrix4f Value, bool Transpose = false) const;
			void SetUniform(const std::string& Name, Matrix3f Value, bool Transpose = false) const;
			
			void SetUniformImageArray(const std::string& Name, int BaseImage, int Length);
			template<typename T> 
			void SetUniformArray(const std::string& Name, std::vector<T> Objects) {
			
				for (int i = 0; i < Objects.size(); i++) {
					std::string IndexName = Name + "[" + std::to_string(i) + "]"; 
					SetUniform(IndexName, Objects[i]);	
				}
			
			}

			void Reload(const std::string& vertex, const std::string& fragment, std::string Inject, int _x);
			void Reload(const std::string& vertex, const std::string& fragment, const std::string& geometry);

			void Reload(const std::string& Directory, std::string Inject = "");
			void Reload(const std::string& Directory, bool HasGeometryShader);

			~Shader();
		};

	}
}
