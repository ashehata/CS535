#pragma once
#include "framebuffer.h"
#include <string>
#include "ppc.h"
class CubeMap
{
public:
	CubeMap();
	CubeMap(string cubemapPath);
	CubeMap(string px, string py, string pz, string nx, string ny, string nz);
	~CubeMap();

	unsigned int Lookup(V3 dir);

	//Standard Order - +x, -x, +y, -y, +z, -z
	FrameBuffer *fbs[6];
    
    PPC *ppc;

	void InitializeHW();

	GLuint texId;

};

