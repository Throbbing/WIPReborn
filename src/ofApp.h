#if 0
#pragma once

#define MAXN 1000
#define MEMSIZE 1024*1024
class ofApp
{
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	~ofApp();
	class TimeSource* times;
	class RBClock* clock;
	class RBTimerBase *timer;
	double lastTime;
	double curTime;
	float _frame;
	unsigned int VBO,VAO,IBO;
	unsigned int VBO1,VAO1,IBO1;
	class WIPBoundShader* bound_shader;
	class WIPBoundShader* bound_shader1;
	class WIPBoundShader* bound_shader2;
	class WIPTexture2D* tex2d;
	class WIPTexture2D* tex2d1;
	class WIPVertexFormat* vf;

	class WIPIndexBuffer* index_buffer;
	class WIPVertexBuffer* vertex_buffer;
	class WIPAnimationClip* clip;
	class WIPScene* scene;

	class WIPSprite* sprites[MAXN];
	int draw_state;

	std::vector<int> get_ids;
	std::vector<WIPSprite*> get_objects;

	class WIPQuadTree* quad_tree;
	bool debug;

	ofFbo fbo;
};
#endif