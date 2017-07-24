#include "OpenGLRHI.h"

GLIndexBuffer::GLIndexBuffer() : ibo(0) { glGenBuffers(1, &ibo); }

void *GLIndexBuffer::get_rhi_resource() const { return (void *)ibo; }

GLVertexBuffer::GLVertexBuffer() : vbo(0) { glGenBuffers(1, &vbo); }

void *GLVertexBuffer::get_rhi_resource() const { return (void *)vbo; }

GLVertexFormat::GLVertexFormat() : WIPVertexFormat() {}

void GLVertexFormat::add_float_vertex_attribute(int count) {
	elements.push_back(WIPVertexElement(count));
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	_gl_internalformat = GL_RGBA;
	_gl_format = GL_RGBA;
	_gl_type = GL_UNSIGNED_BYTE;
}

void *GLTexture2D::get_rhi_resource() const { return (void *)_texture; }

GLRenderTexture2D::GLRenderTexture2D(u32 inw, u32 inh, u32 in_mips,
	u32 in_samples, const RBColorf &ccolor)
	: WIPRenderTexture2D(inw, inh, in_mips, in_samples, ccolor) {
	_gl_internalformat = GL_RGBA;
	_gl_format = GL_RGBA;
	_gl_type = GL_UNSIGNED_BYTE;
	generate_texture();
}

GLRenderTexture2D::~GLRenderTexture2D() {
	glDeleteTextures(1, &_texture);
	glDeleteFramebuffers(1, &_frame_buffer);
}

void GLRenderTexture2D::resize(u32 nw, u32 nh) {
	glDeleteTextures(1, &_texture);
	glDeleteFramebuffers(1, &_frame_buffer);
	w = nw;
	h = nh;
	generate_texture();
}
void *GLRenderTexture2D::get_rhi_resource() const { return (void *)_texture; }

void GLRenderTexture2D::generate_texture(void *data) {
	glGenFramebuffers(1, &_frame_buffer);
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexImage2D(GL_TEXTURE_2D, 0, _gl_internalformat, w, h, 0, _gl_format,
		_gl_type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		_texture, 0);
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
void *GLVertexShader::get_rhi_resource() const { return (void *)_vs; }

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
void *GLPixelShader::get_rhi_resource() const { return (void *)_ps; }

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
	unsigned int rhi1 = *((unsigned int *)(&s1));
	unsigned int rhi2 = *((unsigned int *)(&s2));
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

void *GLBoundShader::get_rhi_resource() const { return (void *)_program; }

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

	for (int i = 0; i < ress_v.size(); ++i) {
		auto shader_text_handler = g_res_manager->load_resource(
			(path + ress_v[i]).c_str(), WIPResourceType::TEXT);
		const char *shader_text = (((string *)shader_text_handler->ptr)->c_str());
		WIPVertexShader *wvs = RHICreateVertexShader(shader_text);
		if (!wvs)
			continue;
		string filename = WIPFileSystem::get_filename(ress_v[i]);
		_vertex_shaders[filename] = wvs;
	}

	for (int i = 0; i < ress_p.size(); ++i) {
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

WIPTexture2D *GLDynamicRHI::RHICreateTexture2D(uint32 SizeX, uint32 SizeY,
	void *data, uint8 Format,
	uint32 NumMips,
	uint32 NumSamples,
	uint32 Flags) {
	return new GLTexture2D(SizeX, SizeY, NumMips, NumSamples, data,Flags);
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
	GLuint &vbo = *((GLuint *)(&s));
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
	GLuint &vbo = *((GLuint *)(&s));
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
	GLuint &ibo = *((GLuint *)(&s));
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
	GLuint &ibo = *((GLuint *)(&s));
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
	GLuint id1 = *((GLuint *)(&s));
	glBindTexture(GL_TEXTURE_2D, id1);
}

void GLDynamicRHI::set_shader(const WIPBoundShader *shader) {
	if (!shader) {
		glUseProgram(0);
		return;
	}
	void *s = shader->get_rhi_resource();
	GLuint id1 = *((GLuint *)(&s));
	glUseProgram(id1);
	_bound_shader = id1;
}
void GLDynamicRHI::set_vertex_format(const WIPVertexFormat *vf) const {
	int tn = vf->total_count;
	int off = 0;
	for (int i = 0; i < vf->elements.size(); ++i) {
		glVertexAttribPointer(i, vf->elements[i].count, GL_FLOAT, GL_FALSE,
			tn * sizeof(float), (void *)(off * sizeof(float)));
		glEnableVertexAttribArray(i);
		off += vf->elements[i].count;
	}
}
void GLDynamicRHI::set_vertex_buffer(const WIPVertexBuffer *vb) const {
	if (!vb)
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	void *s = vb->get_rhi_resource();
	GLuint id1 = *((GLuint *)(&s));
	glBindBuffer(GL_ARRAY_BUFFER, id1);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO1);
}
void GLDynamicRHI::set_index_buffer(const WIPIndexBuffer *ib) const {
	if (!ib)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	void *s = ib->get_rhi_resource();
	GLuint id1 = *((GLuint *)(&s));
	// glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id1);
}
void GLDynamicRHI::draw_triangles(int vertex_count, int offset_add) const{
	// glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, (void*)offset);
	glDrawElementsBaseVertex(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, 0,
		offset_add);
}


void GLDynamicRHI::begin_debug_context()
{

	set_index_buffer(_debug_ib);
	set_vertex_buffer(_debug_vb);
	set_vertex_format(_debug_vf);


	if (!_debug_shader)
		return;
	void *s = _debug_shader->get_rhi_resource();
	GLuint id1 = *((GLuint *)(&s));
	glUseProgram(id1);

	
}

void GLDynamicRHI::change_debug_color(const RBColorf& color)
{
	void *s = _debug_shader->get_rhi_resource();
	GLuint id1 = *((GLuint *)(&s));
	GLint locc = glGetUniformLocation(id1, "in_color");
	glUniform4f(locc,color.r,color.g,color.b,color.a);
}

void GLDynamicRHI::debug_draw_aabb2d(const RBVector2& minp, const RBVector2& maxp, int w, int h) 
{
	if (!_active_view_port)
		return;

	int ow = w * 0.5;
	int oh = h * 0.5;
	float vert[] = { 
		minp.x + ow, minp.y + oh, 0.0f, maxp.x + ow, minp.y + oh, 0.0f,
		minp.x + ow, minp.y + oh, 0.0f, minp.x + ow, maxp.y + oh, 0.0f,
		maxp.x + ow, maxp.y + oh, 0.0f, minp.x + ow, maxp.y + oh, 0.0f,
		maxp.x + ow, maxp.y + oh, 0.0f, maxp.x + ow, minp.y + oh, 0.0f
	};

	//submit vertices if overflowed!
	if ((_debug_update_count + 24)*sizeof(float) > _debug_buffer_size)
	{
		LOG_NOTE("Debug buffer overflowed!");
		debug_submit();
	}
	memcpy(_debug_vertex_buffer + _debug_update_count*sizeof(float), vert, 24 * sizeof(float));
	_debug_update_count += 24;
	
}

void GLDynamicRHI::debug_submit()
{
	if (_debug_update_count == 0)
		return;
	void* p = lock_vertex_buffer(_debug_vb);
	memcpy(p, _debug_vertex_buffer, _debug_update_count * sizeof(float));
	unlock_vertex_buffer(_debug_vb);
	glLineWidth(1);

	glDrawArrays(GL_LINES, 0, _debug_update_count);

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