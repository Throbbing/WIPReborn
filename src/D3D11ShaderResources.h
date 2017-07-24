#pragma once

#ifdef D3D11

#include "d3d11.h"
#include "d3dcompiler.h"
#include "ResourceManager.h"
#include "logger.h"
#include <fstream>


typedef HRESULT (WINAPI *pD3DReflect)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
_In_ SIZE_T SrcDataSize,
_In_ REFIID pInterface,
_Out_ void** ppReflector);

class D3D11ShaderCompiler
{
private:
  static void show_error_message(ID3D10Blob* error, const char* filename)
  {
    char* e = (char*)(error->GetBufferPointer());
    unsigned long bs = error->GetBufferSize();
    std::ofstream fout;

    fout.open("error.txt");


    fout << filename << " compile error : " << std::endl;
    for (unsigned long i = 0; i < bs; ++i)
    {
      fout << e[i];
    }

    fout << std::endl << "==================" << std::endl;

    error->Release();
    error = 0;

  }

public:
  static bool load_compiler(const char* compiler_path)
  {
    CompilerDLL = LoadLibrary(compiler_path);
    if (CompilerDLL)
    {
      compile_shader = (pD3DCompile)(void*)GetProcAddress(CompilerDLL, "D3DCompile");
      if (!compile_shader)
      {
        g_logger->debug_print(WIP_ERROR, "load function D3DCompile from %s failed!", compiler_path);
        return false;
      }
      reflect_shader = (pD3DReflect)(void*)GetProcAddress(CompilerDLL, "D3DReflect");
      if (!reflect_shader)
      {
        g_logger->debug_print(WIP_ERROR, "load function D3DReflect from %s failed!", compiler_path);
        return false;
      }
    }
    else
    {
      g_logger->debug_print(WIP_ERROR, "LoadLibrary %s failed!", compiler_path);
      return false;
    }
    return true;
  }

  static ID3DBlob* compile(ID3D11Device* device, const char* shader_path, const char* entry, const char* shader_model)
  {
    if (!CompilerDLL)
    {
      g_logger->debug(WIP_WARNING, "Shader编译器没有初始化，请勿使用！");
      getchar();
    }
    if (!device) return nullptr;
    /** load shaders */
    {
      unsigned int CompileFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY
        | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
      if (0)//是否开启Debug
        CompileFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
      else
        if (1)//是否标准优化
          CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL1;
        else
          CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
      if (1)//CFLAG_PreferFlowControl or CFLAG_AvoidFlowControl
        if (0)
          CompileFlags |= D3D10_SHADER_PREFER_FLOW_CONTROL;
        else
          CompileFlags |= D3D10_SHADER_AVOID_FLOW_CONTROL;

      auto res_handle = g_res_manager->load_resource(shader_path, WIPResourceType::TEXT);
      if (!res_handle)
      {
        g_logger->debug_print(WIP_ERROR, "load file %s failed!", shader_path);
        return nullptr;
      }
      ID3DBlob* Shader;
      ID3DBlob* Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle->ptr)->data(),
          ((std::string*)res_handle->ptr)->length(),
          shader_path,
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          entry,
          shader_model,
          CompileFlags,
          0,
          &Shader,
          &Errors
          );



        if (FAILED(Result))
        {
          if (Errors)
          {
            show_error_message(Errors, shader_path);
          }
		  printf(!"shader compile failed!");
        }
        else
        {
          return Shader;
        }

      }
      else
      {
        g_logger->debug_print(WIP_ERROR, "compile_shader failed!\n");
      }
      g_res_manager->free(res_handle, res_handle->size);
    }
	return nullptr;
  }

  static void unload()
  {
    if (CompilerDLL)
      FreeLibrary(CompilerDLL);
    CompilerDLL = 0;
    compile_shader = nullptr;
  }



  static   pD3DCompile compile_shader;
  static pD3DReflect reflect_shader;
  static HMODULE CompilerDLL;

};

#endif