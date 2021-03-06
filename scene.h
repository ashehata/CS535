#pragma once

#include "gui.h"
#include "framebuffer.h"
#include "tm.h"
#include "gfb.h"
#include "CGInterface.h"
#include "CubeMap.h"

class Scene {
public:

	CGInterface *cgi;
	ShaderOneInterface *soi;
	EnvironmentMappingInterface *envoi;
	DiffuseShaderInterface *doi;

	GUI *gui;
	FrameBuffer *fb, *fb3, *smfb, *hwfb, *gpufb;
	FrameBuffer *billboard;

	V3 billboardtlc, billboardtrc, billboardblc;
	GLuint billboardTextureId;

	GFB *gfb;
	V3 L;
	float specc;
	TM *tms;
	CubeMap *cm;
	int tmsN;
	FrameBuffer *texts;
	PPC *ppc, *ppc3, *smppc;
	Scene();
	void DBG();
	void NewButton();
	void Render(FrameBuffer *fb, PPC *ppc);
	void RenderAll();
	void ShadowMapSetup();
	void RenderHW();
	void RenderGPU();
	void RenderBillboard(FrameBuffer *currfb, PPC *currppc, int tmi);
	void CreateBillboard();

	bool HWInitialized = false;
	void InitializeHW();

	void EnableFilledMode();
	void EnableWireframeMode();
	void ToggleReflectionShader();

	bool environmentMapping = true;

	bool reflections = false;


	float morphFraction;

	int hwfbCounter = 0;
	int gpufbCounter = 0;

};

extern Scene *scene;