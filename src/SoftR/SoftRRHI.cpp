#include "SoftRRHI.h"

SoftRIndexBuffer::SoftRIndexBuffer()
{
}

void * SoftRIndexBuffer::get_rhi_resource() const
{
	return nullptr;
}

SoftRVertexBuffer::SoftRVertexBuffer()
{
}

void * SoftRVertexBuffer::get_rhi_resource() const
{
	return nullptr;
}

SoftRVertexFormat::SoftRVertexFormat()
{
}

void SoftRVertexFormat::add_float_vertex_attribute(int count)
{
}

void SoftRVertexFormat::add_int_vertex_attribute(int count)
{
}

void * SoftRVertexFormat::get_rhi_resource() const
{
	return nullptr;
}

SoftRTexture2D::SoftRTexture2D(u32 inw, u32 inh, u32 in_mips, u32 in_samples, void * data, int flag)
{
}

void * SoftRTexture2D::get_rhi_resource() const
{
	return nullptr;
}

SoftRRenderTexture2D::SoftRRenderTexture2D(u32 inw, u32 inh, u32 in_mips, u32 in_samples, int flag, const RBColorf & ccolor)
{
}

SoftRRenderTexture2D::~SoftRRenderTexture2D()
{
}

void * SoftRRenderTexture2D::get_rhi_resource() const
{
	return nullptr;
}

void SoftRRenderTexture2D::resize(u32 nw, u32 nh)
{
}

void SoftRRenderTexture2D::generate_texture(void * data)
{
}

SoftRVertexShader::SoftRVertexShader()
{
}

SoftRVertexShader::~SoftRVertexShader()
{
}

bool SoftRVertexShader::load_and_compile(const char * text)
{
	return false;
}

void SoftRVertexShader::set_uniform_texture(const char * uniform_name, const WIPBaseTexture * texture)
{
}

void * SoftRVertexShader::get_rhi_resource() const
{
	return nullptr;
}

SoftRPixelShader::SoftRPixelShader()
{
}

SoftRPixelShader::~SoftRPixelShader()
{
}

bool SoftRPixelShader::load_and_compile(const char * text)
{
	return false;
}

void SoftRPixelShader::set_uniform_texture(const char * uniform_name, const WIPBaseTexture * texture)
{
}

void * SoftRPixelShader::get_rhi_resource() const
{
	return nullptr;
}

SoftRBoundShader::SoftRBoundShader(WIPVertexShader * shader_v, WIPPixelShader * shader_p)
{
}

SoftRBoundShader::~SoftRBoundShader()
{
}

void SoftRBoundShader::set_vertex_uniform_texture(const char * uniform_name, const WIPBaseTexture * texture)
{
}

void SoftRBoundShader::set_pixel_uniform_texture(const char * uniform_name, const WIPBaseTexture * texture)
{
}

void SoftRBoundShader::bind_attribute(unsigned int location, const string & name)
{
}

bool SoftRBoundShader::compile()
{
	return false;
}

void * SoftRBoundShader::get_rhi_resource() const
{
	return nullptr;
}





//////////////////////////


#include "Shader/WIPShaderPS.h"
#include "Shader/WIPShaderVS.h"

void SoftRDynamicRHI::load_shaders() 
{
	std::vector<std::string> ress_v;
	std::vector<std::string> ress_p;
	string path;
	if (_gl_version >= 33)
		path = "./shader/330/";
	else
		path = "./shader/120/";
	g_filesystem->scan_dir(ress_v, path, ".vs", SCAN_FILES, true);
	g_filesystem->scan_dir(ress_p, path, ".ps", SCAN_FILES, true);

	for (size_t i = 0; i < ress_v.size(); ++i) {
		auto shader_text_handler = g_res_manager->load_resource(
			(path + ress_v[i]).c_str(), WIPResourceType::TEXT);
		const char *shader_text = (((string *)shader_text_handler->ptr)->c_str());
		WIPVertexShader *wvs = RHICreateVertexShader(shader_text);
		if (!wvs)
			continue;
		string filename = WIPFileSystem::get_filename(ress_v[i]);
		_vertex_shaders[filename] = wvs;
	}

	for (size_t i = 0; i < ress_p.size(); ++i) {
		auto shader_text_handler = g_res_manager->load_resource(
			(path + ress_p[i]).c_str(), WIPResourceType::TEXT);
		const char *shader_text = (((string *)shader_text_handler->ptr)->c_str());
		WIPPixelShader *wps = RHICreatePixelShader(shader_text);
		if (!wps)
			continue;
		string filename = WIPFileSystem::get_filename(ress_p[i]);
		_pixel_shaders[filename] = wps;
	}
}

void SoftRDynamicRHI::init() {
	_gl_version = -1;
	_debug_shader = 0;
	_bound_shader = 0;

	_cur_ib = nullptr;
	_cur_vb = nullptr;
	_cur_vf = nullptr;

	const GLubyte *name = glGetString(GL_VENDOR);
	const GLubyte *biaoshifu = glGetString(GL_RENDERER);
	const GLubyte *OpenGLVersion = glGetString(GL_VERSION);
	LOG_INFO("GL Driver from %s\n", name);
	LOG_INFO("Render flag : %s\n", biaoshifu);
	LOG_INFO("GL implementation version %s\n", OpenGLVersion);

	_gl_version = ((OpenGLVersion[0] - '0') * 10 + (OpenGLVersion[2] - '0'));




	load_shaders();

	post_init();
}

void SoftRDynamicRHI::post_init()
{
	//256k
	_debug_buffer_size = 256 * 1024;
	_debug_vertex_buffer = new unsigned char[_debug_buffer_size];
	_debug_update_count = 0;

	WIPVertexShader* shader_v = get_vertex_shader("debug");
	WIPPixelShader* shader_p = get_pixel_shader("debug");
	_debug_shader = RHICreateBoundShader(shader_v, shader_p);

	_debug_vertex_buffer_size = _debug_buffer_size;
	_debug_vb = RHICreateVertexBuffer(_debug_vertex_buffer_size, 0, BufferType::E_DYNAMIC_DRAW);
	_debug_ib = RHICreateIndexBuffer(1024 * sizeof(uint32), 0, BufferType::E_DYNAMIC_DRAW);
	_debug_vf = RHICreateVertexFormat();
	_debug_vf->add_float_vertex_attribute(3);

	_active_view_port = nullptr;
}



void SoftRDynamicRHI::shutdown()
{
	//record and destroy all resources
	delete[] _debug_vertex_buffer;
}



WIPViewPort* SoftRDynamicRHI::RHICreateViewPort(int x, int y, int w, int h)
{
	return new WIPViewPort(x, y, w, h);
}

WIPViewPort* SoftRDynamicRHI::change_viewport(WIPViewPort* viewport)
{
	if (!viewport)
		return nullptr;
	WIPViewPort* old = _active_view_port;
	glViewport(viewport->x, viewport->y, viewport->w, viewport->h);
	_active_view_port = viewport;
	return old;
}

void SoftRDynamicRHI::set_back_buffer(const WIPRenderTexture2D* render_texture) const
{
	GLRenderTexture2D::GLRenderTexture2DRHI* rhires = (GLRenderTexture2D::GLRenderTexture2DRHI*)render_texture->get_rhi_resource();
	glBindFramebuffer(GL_FRAMEBUFFER, rhires->_frame_buffer);

}

void SoftRDynamicRHI::set_main_back_buffer() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SoftRDynamicRHI::clear_back_buffer(const RBColorf& c) const
{
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void SoftRDynamicRHI::set_uniform4f(const char* uniform_name, const RBColorf& color)
{
	GLint locc = glGetUniformLocation(_bound_shader, uniform_name);
	glUniform4f(locc, color.r, color.g, color.b, color.a);
}

WIPTexture2D *SoftRDynamicRHI::RHICreateTexture2D(uint32 SizeX, uint32 SizeY,
	void *data, uint8 Format,
	uint32 NumMips,
	uint32 NumSamples,
	uint32 Flags) {
	return new GLTexture2D(SizeX, SizeY, NumMips, NumSamples, data, Flags);
}


WIPRenderTexture2D* SoftRDynamicRHI::RHICreateRenderTexture2D(uint32 SizeX, uint32 SizeY, const RBColorf& c, uint8 Format /*= 0*/, uint32 NumMips /*= 0*/, uint32 NumSamples /*= 0*/, uint32 Flags /*= 0*/)
{
	return new GLRenderTexture2D(SizeX, SizeY, NumMips, NumSamples, Flags, c);
}

void SoftRDynamicRHI::update_texture(WIPTexture2D* texture, void* data) const
{
	void *s = texture->get_rhi_resource();
	GLuint &id = *((GLuint *)s);
	GLTexture2D* gl_tex = (GLTexture2D*)texture;
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->get_width(), texture->get_height()
		, gl_tex->_gl_format, gl_tex->_gl_type, data);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void SoftRDynamicRHI::update_texture(WIPTexture2D* texture, int chanel, void* data) const
{
	//not work!
	LOG_WARN("update channel not work!");
	return;
	void *s = texture->get_rhi_resource();
	GLuint &id = *((GLuint *)s);
	GLTexture2D* gl_tex = (GLTexture2D*)texture;
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->get_width(), texture->get_height()
		, GL_RED + chanel, gl_tex->_gl_type, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SoftRDynamicRHI::update_subrect_texture(WIPTexture2D* texture, int x, int y, int w, int h, void* data) const
{
	void *s = texture->get_rhi_resource();
	GLuint &id = *((GLuint *)(s));
	GLTexture2D* gl_tex_ = (GLTexture2D*)texture;
	glBindTexture(GL_TEXTURE_2D, id);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h
		, gl_tex_->_gl_format, gl_tex_->_gl_type, data);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
}


WIPVertexShader *SoftRDynamicRHI::RHICreateVertexShader(const char *text) const {
	auto ret = new GLVertexShader();
	if (!ret->load_and_compile(text)) {
		delete ret;
		ret = nullptr;
	}
	return ret;
}
WIPPixelShader *SoftRDynamicRHI::RHICreatePixelShader(const char *text) const {
	auto ret = new GLPixelShader();
	if (!ret->load_and_compile(text)) {
		delete ret;
		ret = nullptr;
	}
	return ret;
}
WIPBoundShader *SoftRDynamicRHI::RHICreateBoundShader(WIPVertexShader *vs,
	WIPPixelShader *ps) const {
	auto bound_shader = new GLBoundShader(vs, ps);
	bound_shader->bind_attribute(0, "position");
	bound_shader->bind_attribute(1, "color");
	bound_shader->bind_attribute(2, "normal");
	bound_shader->bind_attribute(3, "texcoord");
	bound_shader->compile();
	return bound_shader;
}
WIPVertexBuffer *SoftRDynamicRHI::RHICreateVertexBuffer(unsigned int size,
	void *data,
	BufferType tp) {
	WIPVertexBuffer *buffer = new GLVertexBuffer();
	void *s = buffer->get_rhi_resource();
	GLuint &vbo = *((GLuint *)(s));
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLenum gltp = GL_STATIC_DRAW;
	switch (tp) {
	case BufferType::E_DYNAMIC_DRAW:
		gltp = GL_DYNAMIC_DRAW;
		break;
	case BufferType::E_STREAM_DRAW:
		gltp = GL_STREAM_DRAW;
		break;
	case BufferType::E_STATIC_DRAW:
		gltp = GL_STATIC_DRAW;
		break;
	}
	glBufferData(GL_ARRAY_BUFFER, size, data, gltp);
	return buffer;
}
void *SoftRDynamicRHI::lock_vertex_buffer(WIPVertexBuffer *buffer) const {
	void *s = buffer->get_rhi_resource();
	GLuint &vbo = *((GLuint *)(s));
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo);
	void *data = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
	if (!data)
		LOG_ERROR("GL Error:%d", glGetError());
	return data;
}
void SoftRDynamicRHI::unlock_vertex_buffer(WIPVertexBuffer *buffer) const {
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}
WIPIndexBuffer *SoftRDynamicRHI::RHICreateIndexBuffer(unsigned int size,
	void *data, BufferType tp) {
	WIPIndexBuffer *buffer = new GLIndexBuffer();
	void *s = buffer->get_rhi_resource();
	GLuint &ibo = *((GLuint *)(s));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	GLenum gltp = GL_STATIC_DRAW;
	switch (tp) {
	case BufferType::E_DYNAMIC_DRAW:
		gltp = GL_DYNAMIC_DRAW;
		break;
	case BufferType::E_STREAM_DRAW:
		gltp = GL_STREAM_DRAW;
		break;
	case BufferType::E_STATIC_DRAW:
		gltp = GL_STATIC_DRAW;
		break;
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, gltp);
	return buffer;
}
void *SoftRDynamicRHI::lock_index_buffer(WIPIndexBuffer *buffer) const {
	void *s = buffer->get_rhi_resource();
	GLuint &ibo = *((GLuint *)(s));
	glBindBuffer(GL_COPY_WRITE_BUFFER, ibo);
	void *data = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
	return data;
}
void SoftRDynamicRHI::unlock_index_buffer(WIPIndexBuffer *buffer) const {
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}
WIPVertexFormat *SoftRDynamicRHI::RHICreateVertexFormat() {
	return new GLVertexFormat();
}
void SoftRDynamicRHI::set_uniform_texture(const char *uniform_name, int tex_loc,
	const WIPBaseTexture *texture) {
	GLint locc = glGetUniformLocation(_bound_shader, uniform_name);
	glUniform1i(locc, tex_loc);
	glActiveTexture(GL_TEXTURE0 + tex_loc);
	void *s = texture->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	glBindTexture(GL_TEXTURE_2D, id1);
}

void SoftRDynamicRHI::set_uniform_texture(const char* uniform_name, int tex_loc, const WIPRenderTexture2D* texture)
{
	GLint locc = glGetUniformLocation(_bound_shader, uniform_name);
	glUniform1i(locc, tex_loc);
	glActiveTexture(GL_TEXTURE0 + tex_loc);
	GLRenderTexture2D::GLRenderTexture2DRHI* s =
		(GLRenderTexture2D::GLRenderTexture2DRHI*) texture->get_rhi_resource();
	GLuint id1 = s->_texture;
	glBindTexture(GL_TEXTURE_2D, id1);
}

void SoftRDynamicRHI::set_shader(const WIPBoundShader *shader) {
	if (!shader) {
		glUseProgram(0);
		return;
	}

	void *s = shader->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	if (_bound_shader == id1)
		return;
	glUseProgram(id1);
	_bound_shader = id1;
}

unsigned int vf_map[VertexType::E_TOTAL] =
{
	GL_FLOAT,
	GL_UNSIGNED_INT
};

unsigned int vf_map_size[VertexType::E_TOTAL] =
{
	sizeof(float),
	sizeof(unsigned int)
};

void SoftRDynamicRHI::set_vertex_format(const WIPVertexFormat *vf)
{
	if (_cur_vf == vf)
		return;
	int tn = vf->total_count;
	int off = 0;
	for (size_t i = 0; i < vf->elements.size(); ++i) {
		glVertexAttribPointer(i, vf->elements[i].count,
			vf_map[vf->elements[i].type], GL_FALSE,
			tn * vf_map_size[vf->elements[i].type],
			(void *)(off * vf_map_size[vf->elements[i].type]));
		glEnableVertexAttribArray(i);
		off += vf->elements[i].count;
	}
	_cur_vf = vf;
}
void SoftRDynamicRHI::set_vertex_buffer(const WIPVertexBuffer *vb)
{
	if (_cur_vb == vb)
		return;
	if (!vb)
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	void *s = vb->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	glBindBuffer(GL_ARRAY_BUFFER, id1);
	_cur_vb = vb;
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO1);
}
void SoftRDynamicRHI::set_index_buffer(const WIPIndexBuffer *ib)
{
	if (ib == _cur_ib)
		return;
	if (!ib)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	void *s = ib->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	// glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id1);
	_cur_ib = ib;
}

void SoftRDynamicRHI::draw_triangles(int index_count, int offset_add) const
{
	// glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, (void*)offset);
	glDrawElementsBaseVertex(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0,
		offset_add);
}


bool SoftRDynamicRHI::begin_debug_context()
{

	set_index_buffer(_debug_ib);
	set_vertex_buffer(_debug_vb);
	set_vertex_format(_debug_vf);


	if (!_debug_shader)
		return false;
	void *s = _debug_shader->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	glUseProgram(id1);
	return true;

}

void SoftRDynamicRHI::change_debug_color(const RBColorf& color)
{
	void *s = _debug_shader->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	GLint locc = glGetUniformLocation(id1, "in_color");
	glUniform4f(locc, color.r, color.g, color.b, color.a);
}

void SoftRDynamicRHI::debug_draw_aabb2d(const RBVector2& minp, const RBVector2& maxp, const WIPCamera* cam)
{

	RBVector2 vert[4];
	vert[0] = minp;
	vert[1] = RBVector2(maxp.x, minp.y);
	vert[2] = maxp;
	vert[3] = RBVector2(minp.x, maxp.y);


	debug_draw_box(vert, cam);

}

void SoftRDynamicRHI::debug_submit()
{
	if (_debug_update_count == 0)
		return;
	void* p = lock_vertex_buffer(_debug_vb);
	memcpy(p, _debug_vertex_buffer, _debug_update_count * sizeof(float));
	unlock_vertex_buffer(_debug_vb);
	glLineWidth(1);

	glDrawArrays(GL_LINES, 0, _debug_update_count / 3);

	_debug_update_count = 0;
}

void SoftRDynamicRHI::end_debug_context()
{
	glUseProgram(_bound_shader);
}

void SoftRDynamicRHI::enable_depth_test() const
{
	glEnable(GL_DEPTH_TEST);
}

void SoftRDynamicRHI::disable_depth_test() const
{
	glDisable(GL_DEPTH_TEST);
}

void SoftRDynamicRHI::set_depth_write(bool val) const
{
	glDepthMask(val ? GL_TRUE : GL_FALSE);
}

void SoftRDynamicRHI::enable_blend() const
{
	glEnable(GL_BLEND);
}

void SoftRDynamicRHI::disable_blend() const
{
	glDisable(GL_BLEND);
}

void SoftRDynamicRHI::set_blend_function() const
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


//lb rb rt lt
void SoftRDynamicRHI::debug_draw_box(const RBVector2* v, const WIPCamera* cam)
{

	float zoom = cam->_zoom;
	float w1 = 1.f / (cam->world_w*0.5f*zoom);
	float w2 = 1.f / (cam->world_h*0.5f*zoom);

	RBVector2 vert[4];

	RBVector2 cam_pos(cam->world_x, cam->world_y);
	vert[0] = v[0] - cam_pos;
	vert[1] = v[1] - cam_pos;
	vert[2] = v[2] - cam_pos;
	vert[3] = v[3] - cam_pos;

	vert[0].x *= w1;
	vert[0].y *= w2;
	vert[1].x *= w1;
	vert[1].y *= w2;
	vert[2].x *= w1;
	vert[2].y *= w2;
	vert[3].x *= w1;
	vert[3].y *= w2;

	//lb rb
	//lb lt
	//rt lt
	//rt rb
	float vert1[] = {
		vert[0].x, vert[0].y, 0.0f, vert[1].x, vert[1].y, 0.0f,
		vert[0].x, vert[0].y, 0.0f, vert[3].x, vert[3].y, 0.0f,
		vert[2].x, vert[2].y, 0.0f, vert[3].x, vert[3].y, 0.0f,
		vert[2].x, vert[2].y, 0.0f, vert[1].x, vert[1].y, 0.0f
	};

	//submit vertices if overflowed!
	if ((_debug_update_count + 24) * sizeof(float) > _debug_buffer_size)
	{
		LOG_NOTE("Debug buffer overflowed!");
		debug_submit();
		return;
	}
	memcpy(_debug_vertex_buffer + _debug_update_count * sizeof(float), vert1, 24 * sizeof(float));
	_debug_update_count += 24;
}


