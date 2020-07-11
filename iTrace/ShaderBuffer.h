#pragma once

#include <vector>
#include "DependenciesRendering.h"

namespace iTrace {

	namespace Rendering {



		template<typename T> 
		struct ShaderBuffer {
			T* Data; 
			size_t Size; 
			unsigned int Buffer; 

			static ShaderBuffer<T> Create(size_t size); 
			void Bind(int idx); 
			void UnMap(); 
			T* GetData(); 

		};



		template<typename T>
		inline ShaderBuffer<T> ShaderBuffer<T>::Create(size_t size)
		{
			
			ShaderBuffer<T> ReturnData; 
			ReturnData.Size = size; 
			glCreateBuffers(1, &ReturnData.Buffer);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ReturnData.Buffer);
			glNamedBufferData(ReturnData.Buffer, size, nullptr, GL_STREAM_READ);

			return ReturnData; 
		}

		template<typename T>
		inline void ShaderBuffer<T>::Bind(int idx)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, idx, Buffer); 
		}

		template<typename T>
		inline void ShaderBuffer<T>::UnMap()
		{
			glUnmapNamedBuffer(Buffer); 
		}

		template<typename T>
		inline T* ShaderBuffer<T>::GetData()
		{

			

			auto Data = reinterpret_cast<T*>(glMapNamedBufferRange(Buffer, 0, Size, GL_MAP_READ_BIT));
			return Data; 
		}

	}

}


