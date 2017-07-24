#pragma once
#include "RenderResource.h"
#include <map>
#include <string>
#include "FileSystem.h"
#include "ResourceManager.h"
#include "Logger.h"
#include "RBMath/Inc/Vector2.h"


using std::map;
using std::string;

enum class BufferType
{
	E_STATIC_DRAW,
	E_DYNAMIC_DRAW,
	E_STREAM_DRAW
};

class WIPDynamicRHI
{
protected:
	map<string,WIPVertexShader*> _vertex_shaders;
	map<string,WIPPixelShader*> _pixel_shaders;

	WIPViewPort* _active_view_port;

	int _debug_buffer_size;
	unsigned char* _debug_vertex_buffer;
	int _debug_update_count;
public:

	WIPVertexShader* get_vertex_shader(const string& name);
	WIPPixelShader* get_pixel_shader(const string& name);

public:
	static WIPDynamicRHI* get_rhi();

/*
	WIPDynamicRHI(){}
	virtual ~WIPDynamicRHI()=0;
*/
	virtual void init() = 0;
	virtual void post_init(){}
	virtual void shutdown() = 0;


	virtual WIPTexture2D* RHICreateTexture2D(uint32 SizeX, uint32 SizeY,  void* data,uint8 Format=0, uint32 NumMips=0, uint32 NumSamples=0, uint32 Flags=0) = 0;
	virtual WIPVertexShader*  RHICreateVertexShader(const char* text) const=0;
	virtual WIPPixelShader*  RHICreatePixelShader(const char* text) const=0;
	virtual WIPBoundShader*  RHICreateBoundShader(WIPVertexShader* vs,WIPPixelShader* ps) const=0;
	virtual WIPVertexBuffer* RHICreateVertexBuffer(unsigned int size,void* data,BufferType tp)=0;
	virtual void* lock_vertex_buffer(WIPVertexBuffer* buffer) const=0;
	virtual void unlock_vertex_buffer(WIPVertexBuffer* buffer) const=0;
	virtual WIPIndexBuffer* RHICreateIndexBuffer(unsigned int size,void* data,BufferType tp)=0;
	virtual void* lock_index_buffer(WIPIndexBuffer* buffer) const=0;
	virtual void unlock_index_buffer(WIPIndexBuffer* buffer) const=0;
	virtual WIPVertexFormat* RHICreateVertexFormat()=0;
	virtual WIPViewPort* RHICreateViewPort(int x, int y, int w, int h) = 0;
	virtual WIPViewPort* change_viewport(WIPViewPort* viewport) = 0;


	virtual void set_uniform_texture(const char* uniform_name,int loc,const WIPBaseTexture* texture)=0;
	virtual void set_shader(const WIPBoundShader* shader)=0;
	virtual void set_vertex_format(const WIPVertexFormat* vf) const=0;
	virtual void set_vertex_buffer(const WIPVertexBuffer* vb) const=0;
	virtual void set_index_buffer(const WIPIndexBuffer* ib) const=0;
	virtual void draw_triangles(int count,int offset)const=0;
	virtual void enable_depth_test()const = 0;
	virtual void disable_depth_test()const = 0;
	virtual void set_depth_write(bool val) const = 0;
	virtual void enable_blend() const=0;
	virtual void disable_blend() const = 0;
	virtual void set_blend_function() const = 0;

	virtual void begin_debug_context() = 0;
	virtual void change_debug_color(const RBColorf& color) = 0;
	virtual void debug_draw_aabb2d(const RBVector2& minp,const RBVector2& maxp,int w,int h) =0;
	virtual void debug_submit()=0;
	virtual void end_debug_context() = 0;


};


extern WIPDynamicRHI* g_rhi;

class WIPCamera;
class WIPScene;
class WIPSprite;

class WIPRender
{
public:
	virtual void init()=0;
	virtual void render(WIPCamera* cam)=0;
	virtual void destroy()=0;
};

class WorldRender : public WIPRender
{
public:
	virtual void init();
	void set_world(const WIPScene* scene);
	virtual void render(WIPCamera* cam);
	virtual void destroy();
	void culling(const WIPCamera* cam);
	int _pack_sprites_blend(void *mem, int n, int offset_n, const WIPCamera& cam, bool& change_texture);
	int _pack_sprites_opaque(void *mem, int n, int offset_n, const WIPCamera& cam, bool& change_texture);
	/*
	03
	12
	*/
	void _pack_index(void *mem, int n)
	{
		unsigned int* p = (unsigned int*)mem;
		unsigned int s[6];
		int k = -1;
		int off = 0;
		while (n--)
		{
			++k;
			s[0] = 0 + off;
			s[1] = 1 + off;
			s[2] = 3 + off;
			s[3] = 1 + off;
			s[4] = 2 + off;
			s[5] = 3 + off;
			off += 4;
			memcpy(p, s, 6 * sizeof(unsigned int));
			p += 6;
		}
	}
	static bool comp_less(const WIPSprite* lhs, const WIPSprite* rhs);
	static bool comp_greater(const WIPSprite* lhs, const WIPSprite* rhs);
	void sort_by_texture();
	void sort_by_zorder();
	WIPIndexBuffer* index_buffer;
	WIPVertexBuffer* vertex_buffer;
	WIPVertexFormat* vertex_format;

	const WIPScene* scene_ref;

	int vertex_buffer_size;
	//maybe use for multithread!
	unsigned char* cpu_vertex_buffer;

	vector<const WIPSprite*> opaque_objects;
	vector<const WIPSprite*> blend_objects;

	float* pack_mem;

	WIPTexture2D* _pre_texture;

	class WIPBoundShader* bound_shader_opaque;
	class WIPBoundShader* bound_shader_translucent;
	class WIPBoundShader* bound_shader_mask;
};

class DebugRender :public WIPRender
{
public:
	virtual void init();
	virtual void render(WIPCamera* cam);
	virtual void destroy();
};