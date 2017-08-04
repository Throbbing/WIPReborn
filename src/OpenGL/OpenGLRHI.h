#pragma once
#include "../RenderResource.h"
#include "../Render.h"
#include "../thirdpart/glad/include/glad/glad.h"

class GLIndexBuffer : public WIPIndexBuffer {
public:
  GLIndexBuffer();
  virtual void *get_rhi_resource() const;
  unsigned int ibo;
};

class GLVertexBuffer : public WIPVertexBuffer {
public:
  GLVertexBuffer();
  virtual void *get_rhi_resource() const;
  unsigned int vbo;
};

class GLVertexFormat : public WIPVertexFormat 
{
public:
  GLVertexFormat();
  virtual void add_float_vertex_attribute(int count);
  virtual void add_int_vertex_attribute(int count);
  virtual void *get_rhi_resource() const;
};

class GLTexture2D : public WIPTexture2D {
public:
  GLTexture2D(u32 inw, u32 inh, u32 in_mips, u32 in_samples, void *data,int flag);
  virtual void *get_rhi_resource() const;

public:
  GLint _gl_internalformat;
  GLenum _gl_format;
  GLenum _gl_type;
  unsigned int _texture;
};

class GLRenderTexture2D : public WIPRenderTexture2D {
public:
	struct GLRenderTexture2DRHI
	{
		GLuint _frame_buffer;
		GLuint _texture;
	};
  GLRenderTexture2D(u32 inw, u32 inh, u32 in_mips, u32 in_samples,int flag,
                    const RBColorf &ccolor);
  ~GLRenderTexture2D();
  virtual void *get_rhi_resource() const;
  void resize(u32 nw, u32 nh);
private:
  void generate_texture(void *data = 0);
public:
  GLint _gl_internalformat;
  GLenum _gl_format;
  GLenum _gl_type;
  GLRenderTexture2DRHI _rhi_res;
};

class GLVertexShader : public WIPVertexShader {
public:
  GLVertexShader();
  ~GLVertexShader();
  virtual bool load_and_compile(const char *text);
  virtual void set_uniform_texture(const char *uniform_name,
                                   const WIPBaseTexture *texture);
  virtual void *get_rhi_resource() const;

private:
  unsigned int _vs;
};

class GLPixelShader : public WIPPixelShader {
public:
  GLPixelShader();
  ~GLPixelShader();
  virtual bool load_and_compile(const char *text);
  virtual void set_uniform_texture(const char *uniform_name,
                                   const WIPBaseTexture *texture);
  virtual void *get_rhi_resource() const;

private:
  unsigned int _ps;
};

class GLBoundShader : public WIPBoundShader 
{
public:
  GLBoundShader(WIPVertexShader *shader_v, WIPPixelShader *shader_p);
  ~GLBoundShader();
  WIPPixelShader *shader_p;
  WIPVertexShader *shader_v;
  void set_vertex_uniform_texture(const char *uniform_name,
                                  const WIPBaseTexture *texture);
  void set_pixel_uniform_texture(const char *uniform_name,
                                 const WIPBaseTexture *texture);

  virtual void bind_attribute(unsigned int location, const string &name);
  virtual bool compile();

  virtual void *get_rhi_resource() const;

private:
  unsigned int _program;
};


class GLDynamicRHI : public WIPDynamicRHI
{
public:
/*
  ~GLDynamicRHI();
	GLDynamicRHI();
  */
private:
	int _gl_version;
	int _debug_vertex_buffer_size;
	int get_gl_version(const GLubyte* ver);
	void load_shaders();
public:

	int get_gl_version(){ return _gl_version; }

	virtual void init();
	
	virtual void post_init();
	
	virtual void shutdown();
	
	virtual WIPRenderTexture2D* RHICreateRenderTexture2D(uint32 SizeX, uint32 SizeY, const RBColorf& data, uint8 Format = 0, 
		uint32 NumMips = 0, uint32 NumSamples = 0, uint32 Flags = 0);

	virtual WIPTexture2D* RHICreateTexture2D(uint32 SizeX, uint32 SizeY, void* data,uint8 Format=0, uint32 NumMips=0, uint32 NumSamples=0, uint32 Flags=0);
	
	virtual void update_texture(WIPTexture2D* texture, void* data) const;

	virtual void update_texture(WIPTexture2D* texture, int chanel, void* data) const;

	virtual void update_subrect_texture(WIPTexture2D* texture, int x, int y, int w, int h, void* data) const;

	virtual WIPVertexShader*  RHICreateVertexShader(const char* text) const;
	
	virtual WIPPixelShader*  RHICreatePixelShader(const char* text) const;
	
	virtual WIPBoundShader*  RHICreateBoundShader(WIPVertexShader* vs,WIPPixelShader* ps) const;
	
    virtual WIPVertexBuffer* RHICreateVertexBuffer(unsigned int size,void* data,BufferType tp);
	
	virtual void* lock_vertex_buffer(WIPVertexBuffer* buffer) const;
	
	virtual void unlock_vertex_buffer(WIPVertexBuffer* buffer) const;
	
	virtual WIPIndexBuffer* RHICreateIndexBuffer(unsigned int size,void* data,BufferType tp);
	
	virtual void* lock_index_buffer(WIPIndexBuffer* buffer) const;
	
	virtual void unlock_index_buffer(WIPIndexBuffer* buffer) const;
	
	virtual WIPVertexFormat* RHICreateVertexFormat();

	virtual WIPViewPort* RHICreateViewPort(int x, int y, int w, int h);

	virtual WIPViewPort* change_viewport(WIPViewPort* viewport);

	virtual void set_back_buffer(const WIPRenderTexture2D* render_texture) const;

	virtual void set_main_back_buffer() const;

	virtual void clear_back_buffer(const RBColorf& c) const;

	virtual void set_uniform4f(const char* uniform_name, const RBColorf& c);
	
	virtual void set_uniform_texture(const char* uniform_name,int tex_loc,const WIPBaseTexture* texture);

	virtual void set_uniform_texture(const char* uniform_name, int loc, const WIPRenderTexture2D* texture);

	virtual void set_shader(const WIPBoundShader* shader);
	
	virtual void set_vertex_format(const WIPVertexFormat* vf) const;
	
	virtual void set_vertex_buffer(const WIPVertexBuffer* vb) const;
	
	virtual void set_index_buffer(const WIPIndexBuffer* ib) const;
	
	virtual void draw_triangles(int vertex_count,int offset_add) const;

	virtual void enable_depth_test()const;

	virtual void disable_depth_test()const;

	//false test but not write depth
	virtual void set_depth_write(bool val) const;
	
	virtual void enable_blend() const;

	virtual void disable_blend() const;

	virtual void set_blend_function() const;

	virtual void begin_debug_context();

	virtual void change_debug_color(const RBColorf& color);

	virtual void debug_draw_aabb2d(const RBVector2& window_minp, const RBVector2& window_maxp, const class WIPCamera* cam);

	//lb rb rt lt
	virtual void debug_draw_box(const RBVector2* v, const class WIPCamera* cam);

	virtual void debug_submit();

	virtual void end_debug_context();
private:
	GLuint _bound_shader;
	WIPBoundShader* _debug_shader;

	WIPVertexBuffer* _debug_vb;
	WIPIndexBuffer* _debug_ib;
	WIPVertexFormat* _debug_vf;
};