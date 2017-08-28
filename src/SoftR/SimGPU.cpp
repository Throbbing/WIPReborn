#include "SimGPU.h"

SrSimGPU* g_gpu = nullptr;// new SrSimGPU();

void func()
{

}
//const int thread_num = 2;
std::mutex SrSimGPU::mu;
std::condition_variable SrSimGPU::g_pass_done;