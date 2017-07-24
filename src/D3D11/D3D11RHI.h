#pragma once
#include "../RenderResource.h"
#include "d3d11.h"

class D3D11Texture2D : public WIPTexture2D
{
public:
	D3D11Texture2D(u32 inw,u32 inh,u32 in_mips,u32 in_samples,void* data,ID3D11Device* device)
		:WIPTexture2D(inw,inh,in_mips,in_samples,data)
	{
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data;

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = w;
		desc.Height = h;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.ArraySize = 1;
		desc.MiscFlags = 0;
		

		HRESULT hr = device->CreateTexture2D(&desc, &initData, &_tex);

		D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
		memset(&srdes, 0, sizeof(srdes));
		srdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srdes.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(_tex, &srdes, &_tex_view);

	}
	~FTexture2D()
	{
		_tex->Release();
		_tex_view->Release();
	}
	virtual void* get_rhi_resource() const
	{
		return (void*)_tex;
	}

public:
	ID3D11Texture2D* _tex;
	ID3D11ShaderResourceView* _tex_view;
};

class D3D11RenderTexture2D : public WIPRenderTexture2D
{
public:
	D3D11RenderTexture2D(u32 inw,u32 inh,u32 in_mips,u32 in_samples,const RBColorf& ccolor):
		WIPRenderTexture2D(inw,inh,in_mips,in_samples,ccolor),_tex(nullptr),_target_view(nullptr),_shader_view(nullptr)
	{
		generate_texture();
	}
	~D3D11RenderTexture2D()
	{
		_tex->Release();
		_target_view->Release();
		_shader_view->Release();
	}
	virtual void clear()
	{

	}
	virtual void begin()
	{

	}
	virtual void end()
	{

	}
	virtual void resize(u32 nw,u32 nh)
	{
		_tex->Release();
		_target_view->Release();
		_shader_view->Release();
		w = nw;
		h = nh;
		generate_texture();
	}
	virtual void* get_rhi_resource() const
	{
		return (void*)_tex;
	}
private:
	void generate_texture(ID3D11Device* device, void *data = 0)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		device->CreateTexture2D(&desc, NULL, &_tex);

		D3D11_RENDER_TARGET_VIEW_DESC rtdes;
		rtdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtdes.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtdes.Texture2D.MipSlice = 0;
		device->CreateRenderTargetView(_tex,&rtdes,&_target_view);

		D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
		memset(&srdes, 0, sizeof(srdes));
		srdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srdes.Texture2D.MostDetailedMip = 0;
		srdes.Texture2D.MipLevels = 1;
		device->CreateShaderResourceView(_tex, &srdes, &_shader_view);
	}

public:
	ID3D11Texture2D* _tex;
	ID3D11ShaderResourceView* _shader_view;
	ID3D11RenderTargetView* _target_view;
};

class D3D11VertexShader : public WIPVertexShader
{
public:
	D3D11VertexShader():_vs(0)
	{
	}
	~D3D11VertexShader()
	{
		_vs->Release();
		_vs = 0;
	}
	virtual bool load_and_compile(const char* text)
	{
		WIPShaderBase::load_and_compile(text);
		int compiled = GL_FALSE;
		_vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(_vs,1,&text,NULL);
		glCompileShader(_vs);
		glGetShaderiv ( _vs, GL_COMPILE_STATUS, &compiled );
		if (!compiled)
		{
			GLint infoLen = 0;

			glGetShaderiv ( _vs, GL_INFO_LOG_LENGTH, &infoLen );

			if ( infoLen > 1 )
			{
				char *infoLog = (char*)malloc ( sizeof ( char ) * infoLen );

				glGetShaderInfoLog (_vs, infoLen, NULL, infoLog );
				printf( "Error compiling shader:\n%s\n", infoLog );

				free ( infoLog );
			}

			glDeleteShader ( _vs );
			_vs = 0;
			printf("vs_source failed!\n");
			return false;
		}
		return true;
	}

	virtual void set_uniform_texture(const char* uniform_name,const WIPBaseTexture* texture)
	{

	}
	virtual void* get_rhi_resource() const
	{
		return (void*)_vs;
	}


private:
	ID3D11VertexShader *_vs;
};

class D3D11PixelShader : public WIPPixelShader
{
public:
	D3D11PixelShader():_ps(0)
	{
	}
	~D3D11PixelShader()
	{
		glDeleteShader(_ps);
		_ps = 0;
	}
	virtual bool load_and_compile(const char* text)
	{
		WIPShaderBase::load_and_compile(text);
		return true;
	}
	virtual void set_uniform_texture(const char* uniform_name,const WIPBaseTexture* texture)
	{
	}
	virtual void* get_rhi_resource() const
	{
		return (void*)_ps;
	}
private:
	ID3D11PixelShader *_ps;
};

class D3D11BoundShader : public WIPBoundShader
{
public:
	D3D11BoundShader(WIPVertexShader* shader_v,WIPPixelShader* shader_p):_program(0)
	{
		this->shader_p = shader_p;
		this->shader_v = shader_v;
		

	}
	~D3D11BoundShader()
	{
	}
	WIPPixelShader* shader_p;
	WIPVertexShader* shader_v;
	void set_vertex_uniform_texture(const char* uniform_name,const WIPBaseTexture* texture)
	{
		shader_v->set_uniform_texture(uniform_name,texture);
	}
	void set_pixel_uniform_texture(const char* uniform_name,const WIPBaseTexture* texture)
	{
		shader_p->set_uniform_texture(uniform_name,texture);
	}

	virtual void bind_attribute(unsigned int location, const string & name)
	{
	}
	virtual bool compile()
	{
		return true;
	}

	virtual void* get_rhi_resource() const
	{
		return (void*)_program;
	}
private:
	unsigned int _program;
};

class D3D11DynamicRHI : public WIPDynamicRHI
{
public:
	D3D11DynamicRHI():_device(nullptr),_context(nullptr){}
	virtual void init() 
	{
		if (!D3D11ShaderCompiler::load_compiler("D3Dcompiler_47-x64.dll")) 
		{
			printf("cannot load d3dcompiler\n");
			getchar();
		}

	}
	virtual void post_init()
	{
	}
	virtual void shutdown()
	{
		D3D11ShaderCompiler::unload();
	}
	virtual FTexture2D* RHICreateTexture2D(uint32 SizeX, uint32 SizeY, void* data,uint8 Format=0, uint32 NumMips=0, uint32 NumSamples=0, uint32 Flags=0)
	{
		
	}
	virtual WIPVertexShader*  RHICreateVertexShader(const char* text) const
	{
	}
	virtual WIPPixelShader*  RHICreatePixelShader(const char* text) const
	{
	}
	virtual WIPBoundShader*  RHICreateBoundShader(WIPVertexShader* vs,WIPPixelShader* ps) const
	{

	}

	virtual void set_uniform_texture(const char* uniform_name,int tex_loc,const WIPBaseTexture* texture)
	{

	}

	virtual void set_shader(const WIPBoundShader* shader)
	{

	}

private:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
};