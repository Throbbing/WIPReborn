#include "StageOM.h"
#include <math.h>
#include "..\\RBMath\\Inc\\Color32.h"

void SrStageOM::proccess(std::vector<SrFragment*>& _triangles_fragments, SrSSBuffer<RBColorf>& color, SrSSBuffer<float>& depth)
{
	for (auto f:_triangles_fragments)
	{
		for (auto px : f->frag)
		{
			//if (px.position.z < 0.9)
				//break;
			if (px.position.z < depth.get_data(RBMath::round_f(px.position.x) + depth.w*0.5, -RBMath::round_f(px.position.y) + depth.h *0.5))
			{
				depth.set_data(RBMath::round_f(px.position.x) + depth.w*0.5, -RBMath::round_f(px.position.y ) + depth.h *0.5, px.position.z);

				RBColor32 c(px.normal.x * 255, px.normal.y * 255, px.normal.z * 255, px.position.w * 255);
				color.set_data(RBMath::round_f(px.position.x) + color.w*0.5, -RBMath::round_f(px.position.y ) + color.h*0.5, c);
			}
			
		}
	}
}

#include "Texture2D.h"

void SrStageOM::proccess(VertexP3N3T2& px, SrTexture2D* color, SrSSBuffer<RBColor32>* bk, SrSSBuffer<float>& depth)
{
	if (px.position.z < depth.get_data(px.position.x,  px.position.y))
	{
		depth.set_data(px.position.x , px.position.y, px.position.z);
		if (bk)
		{
			RBColor32 c(px.normal.x*255, px.normal.y*255, px.normal.z*255, px.position.w*255);
			bk->set_data(px.position.x, px.position.y, c);
		}
		else if(color)
		{
			RBColorf c(px.normal.x , px.normal.y , px.normal.z , px.position.w );
			color->set_color(px.position.x, px.position.y, c);
		}
	}
	
	//强制禁用深度测试
	//RBColor32 c(px.normal.x * 255, px.normal.y * 255, px.normal.z * 255, px.position.w * 255);
	//color.set_data(RBMath::round_f(px.position.x) + color.w*0.5, -RBMath::round_f(px.position.y) + color.h*0.5, c);
	
}

SrStageOM::SrStageOM()
{
}

SrStageOM::~SrStageOM()
{
}