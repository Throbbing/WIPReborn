#include "SimGPU.h"

SrSimGPU* g_gpu = nullptr;// new SrSimGPU();

atomic_int SrSimGPU::handle_count = 0;

ThreadMem *SrSimGPU::mem_list = nullptr;
void SrSimGPU::mem_list_init()
{
	mem_list = new ThreadMem();
	SrSimGPU::mem_list->frame.init((1 << 20) * 500,"class SrSimGPU");
	SrSimGPU::mem_list->frame.getframe(SrSimGPU::mem_list->mf, false);
}

void SrSimGPU::mem_list_deinit()
{
	SrSimGPU::mem_list->frame.shutdown();
	delete mem_list;
}