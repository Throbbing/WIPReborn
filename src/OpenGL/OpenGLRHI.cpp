#include "OpenGLRHI.h"
#include "Camera.h"

GLIndexBuffer::GLIndexBuffer() : ibo(0) { glGenBuffers(1, &ibo); }

void *GLIndexBuffer::get_rhi_resource() const { return (void*)&ibo; }

GLVertexBuffer::GLVertexBuffer() : vbo(0) { glGenBuffers(1, &vbo); }

void *GLVertexBuffer::get_rhi_resource() const { return (void*)&vbo; }

GLVertexFormat::GLVertexFormat() : WIPVertexFormat() {}

void GLVertexFormat::add_float_vertex_attribute(int count) {
	elements.push_back(WIPVertexElement(count));
	total_count += count;
}

void GLVertexFormat::add_int_vertex_attribute(int count)
{
	elements.push_back(WIPVertexElement(count, VertexType::E_INT));
	total_count += count;
}

void *GLVertexFormat::get_rhi_resource() const { return nullptr; }

GLTexture2D::GLTexture2D(u32 inw, u32 inh, u32 in_mips, u32 in_samples,
	void *data,int flag)
	: WIPTexture2D(inw, inh, in_mips, in_samples, data) {
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//ad-hoc
	if (flag == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, flag==2?GL_RED:GL_RGBA, w, h, 0, flag==2?GL_RED:GL_RGBA, GL_UNSIGNED_BYTE,
		data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	_gl_internalformat = (flag == 2 ? GL_RED : GL_RGBA);
	_gl_format = (flag == 2 ? GL_RED : GL_RGBA);
	_gl_type = GL_UNSIGNED_BYTE;
}

void *GLTexture2D::get_rhi_resource() const { return (void*)&_texture; }

GLRenderTexture2D::GLRenderTexture2D(u32 inw, u32 inh, u32 in_mips,
	u32 in_samples, int flag,const RBColorf &ccolor)
	: WIPRenderTexture2D(inw, inh, in_mips, in_samples, ccolor) {
	_gl_internalformat = GL_RGBA;
	_gl_format = GL_RGBA;
	_gl_type = GL_UNSIGNED_BYTE;
	_rhi_res._frame_buffer = 0;
	_rhi_res._texture = 0;
	generate_texture();
}

GLRenderTexture2D::~GLRenderTexture2D() 
{
	glDeleteTextures(1, &_rhi_res._texture);
	glDeleteFramebuffers(1, &_rhi_res._frame_buffer);
}

void GLRenderTexture2D::resize(u32 nw, u32 nh)
{
	glDeleteTextures(1, &_rhi_res._texture);
	glDeleteFramebuffers(1, &_rhi_res._frame_buffer);
	w = nw;
	h = nh;
	generate_texture();
}
void *GLRenderTexture2D::get_rhi_resource() const { return (void *)&_rhi_res; }

void GLRenderTexture2D::generate_texture(void *data) 
{
	glGenFramebuffers(1, &_rhi_res._frame_buffer);
	glGenTextures(1, &_rhi_res._texture);
	glBindTexture(GL_TEXTURE_2D, _rhi_res._texture);
	glTexImage2D(GL_TEXTURE_2D, 0, _gl_internalformat, w, h, 0, _gl_format,
		_gl_type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, _rhi_res._frame_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		_rhi_res._texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLVertexShader::GLVertexShader() : _vs(0) {}
GLVertexShader::~GLVertexShader() {
	glDeleteShader(_vs);
	_vs = 0;
}
bool GLVertexShader::load_and_compile(const char *text) {
	WIPShaderBase::load_and_compile(text);
	int compiled = GL_FALSE;
	_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vs, 1, &text, NULL);
	glCompileShader(_vs);
	glGetShaderiv(_vs, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;

		glGetShaderiv(_vs, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			char *infoLog = (char *)malloc(sizeof(char) * infoLen);

			glGetShaderInfoLog(_vs, infoLen, NULL, infoLog);
			printf("Error compiling shader:\n%s\n", infoLog);

			free(infoLog);
		}

		glDeleteShader(_vs);
		_vs = 0;
		printf("vs_source failed!\n");
		return false;
	}
	return true;
}

void GLVertexShader::set_uniform_texture(const char *uniform_name,
	const WIPBaseTexture *texture) {}
void *GLVertexShader::get_rhi_resource() const { return (void*)&_vs; }

GLPixelShader::GLPixelShader() : _ps(0) {}
GLPixelShader::~GLPixelShader() {
	glDeleteShader(_ps);
	_ps = 0;
}
bool GLPixelShader::load_and_compile(const char *text) {
	WIPShaderBase::load_and_compile(text);
	int compiled = GL_FALSE;
	_ps = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_ps, 1, &text, NULL);
	glCompileShader(_ps);
	glGetShaderiv(_ps, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;

		glGetShaderiv(_ps, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			char *infoLog = (char *)malloc(sizeof(char) * infoLen);

			glGetShaderInfoLog(_ps, infoLen, NULL, infoLog);
			printf("Error compiling shader:\n%s\n", infoLog);

			free(infoLog);
		}

		glDeleteShader(_ps);
		_ps = 0;
		printf("vs_source failed!\n");
		return false;
	}
	return true;
}
void GLPixelShader::set_uniform_texture(const char *uniform_name,
	const WIPBaseTexture *texture) {}
void *GLPixelShader::get_rhi_resource() const { return (void*)&_ps; }

GLBoundShader::GLBoundShader(WIPVertexShader *shader_v,
	WIPPixelShader *shader_p)
	: _program(0) {
	this->shader_p = shader_p;
	this->shader_v = shader_v;
	_program = glCreateProgram();
}
GLBoundShader::~GLBoundShader() { glDeleteProgram(_program); }

void GLBoundShader::set_vertex_uniform_texture(const char *uniform_name,
	const WIPBaseTexture *texture) {
	shader_v->set_uniform_texture(uniform_name, texture);
}
void GLBoundShader::set_pixel_uniform_texture(const char *uniform_name,
	const WIPBaseTexture *texture) {
	shader_p->set_uniform_texture(uniform_name, texture);
}

void GLBoundShader::bind_attribute(unsigned int location, const string &name) {
	glBindAttribLocation(_program, location, name.c_str());
}
bool GLBoundShader::compile() {
	void *s1 = shader_v->get_rhi_resource();
	void *s2 = shader_p->get_rhi_resource();
	unsigned int rhi1 = *((unsigned int *)(s1));
	unsigned int rhi2 = *((unsigned int *)(s2));
	glAttachShader(_program, rhi1);
	glAttachShader(_program, rhi2);
	int linked = GL_FALSE;
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint infoLen = 0;

		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			char *infoLog = (char *)malloc(sizeof(char) * infoLen);

			glGetProgramInfoLog(_program, infoLen, NULL, infoLog);
			printf("Error linking program:\n%s\n", infoLog);

			free(infoLog);
		}

		glDeleteProgram(_program);
		_program = 0;
		return false;
	}
	return true;
}

void *GLBoundShader::get_rhi_resource() const { return (void *)&_program; }

//GLDynamicRHI::GLDynamicRHI() : _gl_version(-1) {}

int GLDynamicRHI::get_gl_version(const GLubyte *ver) { return _gl_version; }
void GLDynamicRHI::load_shaders() {
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

void GLDynamicRHI::init() {
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

void GLDynamicRHI::post_init()
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



void GLDynamicRHI::shutdown()
{
	//record and destroy all resources
	delete[] _debug_vertex_buffer;
}



WIPViewPort* GLDynamicRHI::RHICreateViewPort(int x, int y, int w, int h)
{
	return new WIPViewPort(x, y, w, h);
}

WIPViewPort* GLDynamicRHI::change_viewport(WIPViewPort* viewport)
{
	if (!viewport)
		return nullptr;
	WIPViewPort* old = _active_view_port;
	glViewport(viewport->x,viewport->y,viewport->w,viewport->h);
	_active_view_port = viewport;
	return old;
}

void GLDynamicRHI::set_back_buffer(const WIPRenderTexture2D* render_texture) const
{
	GLRenderTexture2D::GLRenderTexture2DRHI* rhires = (GLRenderTexture2D::GLRenderTexture2DRHI*)render_texture->get_rhi_resource();
	glBindFramebuffer(GL_FRAMEBUFFER, rhires->_frame_buffer);

}

void GLDynamicRHI::set_main_back_buffer() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLDynamicRHI::clear_back_buffer(const RBColorf& c) const
{
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GLDynamicRHI::set_uniform4f(const char* uniform_name, const RBColorf& color)
{
	GLint locc = glGetUniformLocation(_bound_shader, uniform_name);
	glUniform4f(locc, color.r, color.g, color.b, color.a);
}

WIPTexture2D *GLDynamicRHI::RHICreateTexture2D(uint32 SizeX, uint32 SizeY,
	void *data, uint8 Format,
	uint32 NumMips,
	uint32 NumSamples,
	uint32 Flags) {
	return new GLTexture2D(SizeX, SizeY, NumMips, NumSamples, data,Flags);
}


WIPRenderTexture2D* GLDynamicRHI::RHICreateRenderTexture2D(uint32 SizeX, uint32 SizeY, const RBColorf& c, uint8 Format /*= 0*/, uint32 NumMips /*= 0*/, uint32 NumSamples /*= 0*/, uint32 Flags /*= 0*/)
{
	return new GLRenderTexture2D(SizeX, SizeY, NumMips, NumSamples, Flags,c);
}

void GLDynamicRHI::update_texture(WIPTexture2D* texture,void* data) const
{
	void *s = texture->get_rhi_resource();
	GLuint &id = *((GLuint *)s);
	GLTexture2D* gl_tex = (GLTexture2D*)texture;
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->get_width(),texture->get_height()
		, gl_tex->_gl_format,gl_tex->_gl_type, data);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void GLDynamicRHI::update_texture(WIPTexture2D* texture, int chanel, void* data) const
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

void GLDynamicRHI::update_subrect_texture(WIPTexture2D* texture, int x, int y, int w, int h, void* data) const
{
	void *s = texture->get_rhi_resource();
	GLuint &id = *((GLuint *)(s));
	GLTexture2D* gl_tex_ = (GLTexture2D*)texture;
	glBindTexture(GL_TEXTURE_2D, id);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h
		,gl_tex_->_gl_format, gl_tex_->_gl_type, data);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
	glBindTexture(GL_TEXTURE_2D, 0);
}


WIPVertexShader *GLDynamicRHI::RHICreateVertexShader(const char *text) const {
	auto ret = new GLVertexShader();
	if (!ret->load_and_compile(text)) {
		delete ret;
		ret = nullptr;
	}
	return ret;
}
WIPPixelShader *GLDynamicRHI::RHICreatePixelShader(const char *text) const {
	auto ret = new GLPixelShader();
	if (!ret->load_and_compile(text)) {
		delete ret;
		ret = nullptr;
	}
	return ret;
}
WIPBoundShader *GLDynamicRHI::RHICreateBoundShader(WIPVertexShader *vs,
	WIPPixelShader *ps) const {
	auto bound_shader = new GLBoundShader(vs, ps);
	bound_shader->bind_attribute(0, "position");
	bound_shader->bind_attribute(1, "color");
	bound_shader->bind_attribute(2, "normal");
	bound_shader->bind_attribute(3, "texcoord");
	bound_shader->compile();
	return bound_shader;
}
WIPVertexBuffer *GLDynamicRHI::RHICreateVertexBuffer(unsigned int size,
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
void *GLDynamicRHI::lock_vertex_buffer(WIPVertexBuffer *buffer) const {
	void *s = buffer->get_rhi_resource();
	GLuint &vbo = *((GLuint *)(s));
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo);
	void *data = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
	if (!data)
		LOG_ERROR("GL Error:%d", glGetError());
	return data;
}
void GLDynamicRHI::unlock_vertex_buffer(WIPVertexBuffer *buffer) const {
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}
WIPIndexBuffer *GLDynamicRHI::RHICreateIndexBuffer(unsigned int size,
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
void *GLDynamicRHI::lock_index_buffer(WIPIndexBuffer *buffer) const {
	void *s = buffer->get_rhi_resource();
	GLuint &ibo = *((GLuint *)(s));
	glBindBuffer(GL_COPY_WRITE_BUFFER, ibo);
	void *data = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
	return data;
}
void GLDynamicRHI::unlock_index_buffer(WIPIndexBuffer *buffer) const {
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}
WIPVertexFormat *GLDynamicRHI::RHICreateVertexFormat() {
	return new GLVertexFormat();
}
void GLDynamicRHI::set_uniform_texture(const char *uniform_name, int tex_loc,
	const WIPBaseTexture *texture) {
	GLint locc = glGetUniformLocation(_bound_shader, uniform_name);
	glUniform1i(locc, tex_loc);
	glActiveTexture(GL_TEXTURE0 + tex_loc);
	void *s = texture->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	glBindTexture(GL_TEXTURE_2D, id1);
}

void GLDynamicRHI::set_uniform_texture(const char* uniform_name, int tex_loc, const WIPRenderTexture2D* texture)
{
	GLint locc = glGetUniformLocation(_bound_shader, uniform_name);
	glUniform1i(locc, tex_loc);
	glActiveTexture(GL_TEXTURE0 + tex_loc);
	GLRenderTexture2D::GLRenderTexture2DRHI* s = 
		(GLRenderTexture2D::GLRenderTexture2DRHI*) texture->get_rhi_resource();
	GLuint id1 = s->_texture;
	glBindTexture(GL_TEXTURE_2D, id1);
}

void GLDynamicRHI::set_shader(const WIPBoundShader *shader) {
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

void GLDynamicRHI::set_vertex_format(const WIPVertexFormat *vf) 
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
void GLDynamicRHI::set_vertex_buffer(const WIPVertexBuffer *vb) 
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
void GLDynamicRHI::set_index_buffer(const WIPIndexBuffer *ib) 
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

void GLDynamicRHI::draw_triangles(int index_count, int offset_add) const
{
	// glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, (void*)offset);
	glDrawElementsBaseVertex(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0,
		offset_add);
}


bool GLDynamicRHI::begin_debug_context()
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

void GLDynamicRHI::change_debug_color(const RBColorf& color)
{
	void *s = _debug_shader->get_rhi_resource();
	GLuint id1 = *((GLuint *)(s));
	GLint locc = glGetUniformLocation(id1, "in_color");
	glUniform4f(locc,color.r,color.g,color.b,color.a);
}

void GLDynamicRHI::debug_draw_aabb2d(const RBVector2& minp, const RBVector2& maxp, const WIPCamera* cam)
{

	RBVector2 vert[4];
	vert[0] = minp;
	vert[1] = RBVector2(maxp.x,minp.y);
	vert[2] = maxp;
	vert[3] = RBVector2(minp.x, maxp.y);


	debug_draw_box(vert, cam);
	
}

void GLDynamicRHI::debug_submit()
{
	if (_debug_update_count == 0)
		return;
	void* p = lock_vertex_buffer(_debug_vb);
	memcpy(p, _debug_vertex_buffer, _debug_update_count * sizeof(float));
	unlock_vertex_buffer(_debug_vb);
	glLineWidth(1);

	glDrawArrays(GL_LINES, 0, _debug_update_count/3);

	_debug_update_count = 0;
}

void GLDynamicRHI::end_debug_context()
{
	glUseProgram(_bound_shader);
}

void GLDynamicRHI::enable_depth_test() const
{
	glEnable(GL_DEPTH_TEST);
}

void GLDynamicRHI::disable_depth_test() const
{
	glDisable(GL_DEPTH_TEST);
}

void GLDynamicRHI::set_depth_write(bool val) const
{
	glDepthMask(val?GL_TRUE:GL_FALSE);
}

void GLDynamicRHI::enable_blend() const
{
	glEnable(GL_BLEND);
}

void GLDynamicRHI::disable_blend() const
{
	glDisable(GL_BLEND);
}

void GLDynamicRHI::set_blend_function() const
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


//lb rb rt lt
void GLDynamicRHI::debug_draw_box(const RBVector2* v, const WIPCamera* cam)
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

	vert[0].x*=w1;
	vert[0].y*=w2;
	vert[1].x*=w1;
	vert[1].y*=w2;
	vert[2].x*=w1;
	vert[2].y*=w2;
	vert[3].x*=w1;
	vert[3].y*=w2;

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
	if ((_debug_update_count + 24)*sizeof(float) > _debug_buffer_size)
	{
		LOG_NOTE("Debug buffer overflowed!");
		debug_submit();
		return;
	}
	memcpy(_debug_vertex_buffer + _debug_update_count*sizeof(float), vert1, 24 * sizeof(float));
	_debug_update_count += 24;
}










#include "../Platform/GLFWApp.h"

bool GLFWApp::init_gl(const char *title, int width, int height)
{

	if (!glfwInit())
	{
		LOG_ERROR("glfwInit() failed!");
		return false;
	}
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG_ERROR("Failed to initialize OpenGL context");
		return false;
	}
	return true;
}