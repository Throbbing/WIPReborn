#if 0


#include "ofApp.h"
#include "FileSystem.h"
#include "IniHelper.h"
#include "Logger.h"
#include "Render.h"
#include "ResourceManager.h"
#include "ScriptManager.h"
#include "WIPTime.h"
#include "Sprite.h"
#include "AnimationManager.h"
#include "AnimationClip.h"
#include "QuadTree.h"
#include "Scene.h"
#include <iostream>

using std::cout;
using std::endl;

int pack_sprite(WIPSprite **sprite, void *mem, int n,int offset_n)
{
	float *p = (float*)mem;
	float s[sizeof(float) * 44];
	int k = -1+offset_n;
	while (n--) 
	{
		k++;
		float w1 = ofGetWindowWidth()/2;
		float w2 = ofGetWindowHeight()/2;
		//03
		//12
		// todo:padding 1 float
		
		if(!sprite[k])
			continue;
		/*
		if(sprite[k]->handled)
			continue;
			*/
		sprite[k]->handled = true;
		RBVector2 vert[4];
		sprite[k]->get_world_position(vert);
		s[0] = vert[0].x/w1;
		s[1] = vert[0].y/w2;
		s[2] = sprite[k]->_transform->z_order;
		s[3] = sprite[k]->_render->material.vert_color[0].r;
		s[4] = sprite[k]->_render->material.vert_color[0].g;
		s[5] = sprite[k]->_render->material.vert_color[0].b;
		s[6] = 0;
		s[7] = 0;
		s[8] = 1;
		// from framebox
		s[9] = sprite[k]->_animation->_framebox_ref.lb.x;//0
		s[10] = sprite[k]->_animation->_framebox_ref.lb.y;//0

		s[11] = vert[1].x/w1;
		s[12] = vert[1].y/w2;
		s[13] = sprite[k]->_transform->z_order;
		s[14] = sprite[k]->_render->material.vert_color[0].r;
		s[15] = sprite[k]->_render->material.vert_color[0].g;
		s[16] = sprite[k]->_render->material.vert_color[0].b;
		s[17] = 0;
		s[18] = 0;
		s[19] = 1;
		// from framebox
		s[20] = sprite[k]->_animation->_framebox_ref.lt.x;//0
		s[21] = sprite[k]->_animation->_framebox_ref.lt.y;//1

		s[22] = vert[2].x/w1;
		s[23] = vert[2].y/w2;
		s[24] = sprite[k]->_transform->z_order;
		s[25] = sprite[k]->_render->material.vert_color[0].r;
		s[26] = sprite[k]->_render->material.vert_color[0].g;
		s[27] = sprite[k]->_render->material.vert_color[0].b;
		s[28] = 0;
		s[29] = 0;
		s[30] = 1;
		// from framebox
		s[31] = sprite[k]->_animation->_framebox_ref.rt.x;//1
		s[32] = sprite[k]->_animation->_framebox_ref.rt.y;//1

		s[33] = vert[3].x/w1;
		s[34] = vert[3].y/w2;
		s[35] = sprite[k]->_transform->z_order;
		s[36] = sprite[k]->_render->material.vert_color[0].r;
		s[37] = sprite[k]->_render->material.vert_color[0].g;
		s[38] = sprite[k]->_render->material.vert_color[0].b;
		s[39] = 0;
		s[40] = 0;
		s[41] = 1;
		// from framebox
		s[42] = sprite[k]->_animation->_framebox_ref.rb.x;//1
		s[43] = sprite[k]->_animation->_framebox_ref.rb.y;//1
		memcpy(p,s,sizeof(float) * 44);
		p += 44;
		//LOG_WARN("%d",p-mem);
		if((int)(p+44-(float*)mem)>=MEMSIZE/sizeof(float))
		{
			//LOG_WARN("Copy overflow!Break!");
			return k+1;
		}
	}
	//draw done!
	return -1;
}

/*
03
12
*/
void pack_index(void *mem, int n)
{
	unsigned int* p = (unsigned int*)mem;
	unsigned int s[6];
	int k=-1;
	int off = 0;
	while(n--)
	{
		++k;
		s[0] = 0 + off;
		s[1] = 1 + off;
		s[2] = 3 + off;
		s[3] = 1 + off;
		s[4] = 2 + off;
		s[5] = 3 + off;
		off+=4;
		memcpy(p,s,6*sizeof(unsigned int));
		p+=6;
	}
}

/*
make singleton base class
->auto delete instance using static function.
*/
void ofApp::setup() 
{
	debug = false;
	scene = new WIPScene();
	scene->init(ofGetWindowWidth(),ofGetWindowHeight(),6);


	quad_tree = new WIPQuadTree(ofGetWindowWidth(),ofGetWindowHeight(),6);
	quad_tree->build_empty();


	fbo.allocate(ofGetWindowWidth(),ofGetWindowHeight());
	fbo.begin();
	quad_tree->debug_draw();
	fbo.end();

	draw_state = 0;

	WIPFileSystem& fs = *g_filesystem;
	std::string cur_path = fs.get_current_dir();

	fs.set_current_dir("./data/");

	g_logger->startup("./log/");
	g_logger->new_log();
	LOG_NOTE("cur path:%s", fs.get_current_dir().c_str());
	if (!WIPIniHelper::reset_ini_file("WIPCFG.ini")) {
		LOG_NOTE("WIPCFG.ini does not exists\n");
	}

	std::string path = "../../../../a.t"; // bin/WIPCFG.ini";
	std::string path_name, filename, exten;
	WIPFileSystem::split_path(path, path_name, filename, exten, true);
	LOG_INFO("\npath:%s\npath name:%s\nfile name:%s\nextension:%s\n",
		path.c_str(), path_name.c_str(), filename.c_str(), exten.c_str());

	LOG_INFO("%s", WIPFileSystem::get_extension(path, true).c_str());

	LOG_INFO("%s", WIPFileSystem::get_extension(path, false).c_str());

	LOG_INFO("%s", WIPFileSystem::get_filename(path).c_str());

	LOG_INFO("%s", WIPFileSystem::get_path(path).c_str());
	LOG_INFO("%s", WIPFileSystem::get_filename_and_extension(path, true).c_str());
	LOG_INFO("%s", WIPFileSystem::get_parent_path(path).c_str());
	LOG_INFO("%s", WIPFileSystem::replace_extension(path, ".ttt").c_str());

	LOG_INFO("%s", WIPFileSystem::add_trailing_slash(path).c_str());

	LOG_INFO("%s",WIPFileSystem::remove_trailing_slash(WIPFileSystem::add_trailing_slash(path)).c_str());

	LOG_INFO("%s", (WIPFileSystem::is_absolute_path(path) ? "true" : "false"));

	/*
	if(fs.set_current_dir(WIPFileSystem::get_path(path)))
	{
	cout<<"set success!\n"<<fs.get_current_dir()<<endl;
	}
	path = fs.get_current_dir();
	if(fs.file_exists(path))
	{
	cout<<"file exists\n";
	}
	if(!fs.file_exists(path+"s"))
	{
	cout<<"odd file does not exists\n";
	}
	if(fs.dir_exists(WIPFileSystem::get_path(path)))
	{
	cout<<"path exists\n";
	}
	if(!fs.dir_exists(WIPFileSystem::get_path(path)+"sss"))
	{
	cout<<"odd path does not exists\n";
	}
	*/
	std::string file_path;
	path = "WIPCFG.ini";
	if(WIPIniHelper::reset_ini_file(path.data()))
	{
		WIPIniHelper::get_string("Common","project",file_path);
	}
	else
	{
		printf("Read ini failed!\n");
		getchar();
	}
	WIPIniHelper::close();

	g_script_manager->startup();
	g_script_manager->load_file(file_path.c_str());

	times = new TimeSource();
	RBClock::init(times);
	clock = RBClock::Instance();

	timer = new RBTimerBase(*clock);
	clock->set_filtering(10, 0.1666666666666f);
	lastTime = timer->get_time();
	clock->update();

	_frame = 1.f/10.f;

	//auto clip_text_handler = g_res_manager->load_resource("./clips/jiangshi_bleft.clip",WIPResourceType::TEXT);
	//const char *clip_text = (((string *)clip_text_handler->ptr)->c_str());
	clip = WIPAnimationClip::create_with_atlas("man_walk","./clips/jiangshi_bleft.clip");



	auto res_handle1 = g_res_manager->load_resource("./out3.png", WIPResourceType::TEXTURE);
	int ww = ((TextureData *)(res_handle1->extra))->width;
	int hh = ((TextureData *)(res_handle1->extra))->height;
	auto res_handle2 = g_res_manager->load_resource("./jiangshi_bleft.png", WIPResourceType::TEXTURE);
	int ww1 = ((TextureData *)(res_handle2->extra))->width;
	int hh1 = ((TextureData *)(res_handle2->extra))->height;
	float rot = ww1/(float)hh1;

	/*
	^ 1
	|
	|
	|
	|----------->1
	-1
	*/



	g_rhi->init();

	tex2d = g_rhi->RHICreateTexture2D(ww, hh, res_handle1->ptr);
	tex2d1 = g_rhi->RHICreateTexture2D(ww1, hh1, res_handle2->ptr);


	auto wvs = g_rhi->get_vertex_shader("simple_vs");
	auto wps = g_rhi->get_pixel_shader("alpha_mask");
	auto wps1 = g_rhi->get_pixel_shader("opaque");
	auto wps2 = g_rhi->get_pixel_shader("translucent");

	bound_shader = g_rhi->RHICreateBoundShader(wvs, wps);
	bound_shader1 = g_rhi->RHICreateBoundShader(wvs, wps1);
	bound_shader2 = g_rhi->RHICreateBoundShader(wvs, wps2);


	for(int i=0;i<MAXN;++i)
	{
		sprites[i] = WIPSprite::create(0.2*ofGetWindowWidth()*0.5f/rot,0.2*ofGetWindowHeight()*0.5f);
		float w1 = ofGetWindowWidth()/2;
		float w2 = ofGetWindowHeight()/2;
		f32 a = RBMath::get_rand_range_f(-w1,w1);
		f32 b = RBMath::get_rand_range_f(-w2,w2); 
		sprites[i]->translate(a,b);
		float escale = RBMath::get_rand_range_f(0.2f,0.8f);
		sprites[i]->scale_to(escale,escale);
		sprites[i]->rotate_to(RBMath::get_rand_range_f(0.f,360.f) );
		int k =  ::rand();
		if(k>=RAND_MAX/2)
		{
			sprites[i]->set_texture(tex2d);
		}
		else
		{
			sprites[i]->set_texture(tex2d1);
		}
		k = RBMath::get_rand_range_i(0,30);
		if(k/10==1)
		{
			sprites[i]->_render->material.shader = bound_shader2;
		}
		else if(k/10==2)
		{
			sprites[i]->_render->material.shader = bound_shader2;
		}
		else
		{
			sprites[i]->_render->material.shader = bound_shader2;
		}

		clip->bloop = true;
		sprites[i]->_animation->add_clip(clip,clip->name);
		sprites[i]->_animation->play();

		scene->add_sprite(sprites[i]);
		
		/*
		if(i==2||i==5)
		scene->remove_sprite(sprites[i]);
		*/
		//quad_tree->insert(*sprites[i],i);
	}



	vf = g_rhi->RHICreateVertexFormat();
	vf->add_float_vertex_attribute(3);
	vf->add_float_vertex_attribute(3);
	vf->add_float_vertex_attribute(3);
	vf->add_float_vertex_attribute(2);

	index_buffer = g_rhi->RHICreateIndexBuffer(6*sizeof(unsigned int),0,BufferType::E_DYNAMIC_DRAW);
	vertex_buffer = g_rhi->RHICreateVertexBuffer(MEMSIZE,0,BufferType::E_DYNAMIC_DRAW);



	void* p = g_rhi->lock_index_buffer(index_buffer);
	pack_index(p,1);
	g_rhi->unlock_index_buffer(index_buffer);
	/*
	get_objects.clear();
	get_ids.clear();
	scene->quad_tree->get_all_nodes(get_ids);
	get_objects.resize(scene->objects.size());
	for(int i=0;i<get_ids.size();++i)
	{
		get_objects[get_ids[i]] = scene->objects[get_ids[i]];
	}
	p = g_rhi->lock_vertex_buffer(vertex_buffer);
	pack_sprite(get_objects.data(), p, scene->objects.size());
	g_rhi->unlock_vertex_buffer(vertex_buffer);
	*/

	g_res_manager->free(res_handle1, res_handle1->size);
}

ofApp::~ofApp() 
{
	g_script_manager->shutdown();
	g_logger->shutdown();
}

//--------------------------------------------------------------
void ofApp::update() 
{
	curTime = times->get_time();

	if (curTime - lastTime >= _frame) 
	{
		g_script_manager->call("main_logic");
		lastTime = curTime;
		clock->update();

		if(ofGetMousePressed(0))
		{
			get_ids.clear();
			RBVector2 mpos_w(ofGetMouseX(),(ofGetWindowHeight() - ofGetMouseY()));
			mpos_w.x -= ofGetWindowWidth()/2;
			mpos_w.y -= ofGetWindowHeight()/2;
			scene->quad_tree->get_near_node(mpos_w,get_ids);
			for(int i=0;i<get_ids.size();++i)
			{
				scene->objects[get_ids[i]]->_render->material.vert_color[0] = RBColorf::red;
				scene->objects[get_ids[i]]->_render->material.vert_color[1] = RBColorf::red;
				scene->objects[get_ids[i]]->_render->material.vert_color[2] = RBColorf::red;
				scene->objects[get_ids[i]]->_render->material.vert_color[3] = RBColorf::red;
			}
		}
		if(ofGetMousePressed(2))
		{
			get_ids.clear();
			RBVector2 mpos_w(ofGetMouseX(),(ofGetWindowHeight() - ofGetMouseY()));
			mpos_w.x -= ofGetWindowWidth()/2;
			mpos_w.y -= ofGetWindowHeight()/2;
			scene->quad_tree->get_near_node(mpos_w,get_ids);
			for(int i=0;i<get_ids.size();++i)
			{
				scene->objects[get_ids[i]]->_render->material.vert_color[0] = RBColorf::white;
				scene->objects[get_ids[i]]->_render->material.vert_color[1] = RBColorf::white;
				scene->objects[get_ids[i]]->_render->material.vert_color[2] = RBColorf::white;
				scene->objects[get_ids[i]]->_render->material.vert_color[3] = RBColorf::white;
			}
		}
		if(ofGetMousePressed(1))
		{
			get_ids.clear();
			float x = ofGetWindowWidth()/2;
			float y = ofGetWindowHeight()/2;
			RBVector2 a(-x/2,-y/2);
			RBVector2 b(x/2,y/2);
			RBAABB2D aabb(a,b);
			scene->quad_tree->get_intersected_node(aabb,get_ids);
			for(int i=0;i<get_ids.size();++i)
			{
				scene->objects[get_ids[i]]->_render->material.vert_color[0] = RBColorf::blue;
				scene->objects[get_ids[i]]->_render->material.vert_color[1] = RBColorf::blue;
				scene->objects[get_ids[i]]->_render->material.vert_color[2] = RBColorf::blue;
				scene->objects[get_ids[i]]->_render->material.vert_color[3] = RBColorf::blue;
			}
		}		

		g_animation_manager->update(clock->get_frame_time());

		/*
		void* p = g_rhi->lock_index_buffer(index_buffer);
		pack_index(p,1);
		g_rhi->unlock_index_buffer(index_buffer);
		*/

		

	} 
	else 
	{
#ifdef _WIN32
		Sleep(0);
#else
		usleep(0);
#endif
	}

}

void ofApp::draw()
{
//#define DRAWGRID

	g_rhi->set_shader(0);
	quad_tree->debug_draw();


	get_objects.clear();
	get_ids.clear();
	scene->quad_tree->get_all_nodes(get_ids);
	get_objects.resize(scene->objects.size());
	for(int i=0;i<get_ids.size();++i)
	{
		get_objects[get_ids[i]] = scene->objects[get_ids[i]];
	}
	g_rhi->set_index_buffer(index_buffer);
	g_rhi->set_vertex_buffer(vertex_buffer);
	g_rhi->set_vertex_format(vf); 

	int res = 0;
	while(res!=-1)
	{
		void* p = g_rhi->lock_vertex_buffer(vertex_buffer);
		res = pack_sprite(get_objects.data(), p, scene->objects.size()-res,res);
		g_rhi->unlock_vertex_buffer(vertex_buffer);

		for(int i=0;i<MAXN;++i)
		{
			g_rhi->set_shader(sprites[i]->_render->material.shader);
			g_rhi->set_uniform_texture("in_texture", 0, sprites[i]->_render->material.texture);
			g_rhi->draw_triangles(6,4*i);
		}
	}



	//glBindVertexArray(VAO);
	//we don not use vao！
}

void ofApp::keyPressed(int key) {}

void ofApp::keyReleased(int key) {
	if (key == 'q') {
		_frame = 1.f/10.f;
	} else if (key == 'w') {
		_frame = 1.f/60.f;
	} else {
		_frame = 1.f/800.f;
	}
	if(key=='d')
	{
		debug = !debug;
	}

}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
#endif