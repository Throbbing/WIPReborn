#include "InnerData.h"


#ifdef POOL
void *SrTriangle::nodes = nullptr;
class RBPoolAllctor* SrTriangle::pool = nullptr;
#else
RBFrameAlloctor* SrTriangle::frame = nullptr;
MemoryFrame SrTriangle::me;
#endif