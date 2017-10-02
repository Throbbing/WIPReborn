#pragma once



#include "Remotery.h"
#include "Logger.h"

class RemoteryProfiler
{
public:
	//rmt_ScopedOpenGLSample(s);
	//rmt_BeginCPUSample(s, 1);
	//flags:
	//RMTSF_Aggregate
	static void startup()
	{
		rmtSettings* settings = rmt_Settings();
		if (settings)
			settings->port = 1089;

		if (RMT_ERROR_NONE != rmt_CreateGlobalInstance(&rmt)) 
		{
			LOG_ERROR("Remotery init failed!");
			return;
		}
#ifndef USE_D3D11
		rmt_BindOpenGL();
#endif
	}
	static void shutdown()
	{
		if (rmt)
		{

#ifndef USE_D3D11
			rmt_UnbindOpenGL();
#endif
			rmt_DestroyGlobalInstance(rmt);
			rmt = nullptr;
		}

		
	}
private:
	static Remotery* rmt;
};


