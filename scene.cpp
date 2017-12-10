#include "stdafx.h"

#include "scene.h"
#include "v3.h"
#include "m33.h"
#include "ppc.h"

Scene *scene;

using namespace std;

#include <iostream>

Scene::Scene() {

	scene = this;

	morphFraction = 0.0f;

	gui = new GUI();
	gui->show();

	cm = new CubeMap("textures/uffizi_cross.tiff");


	specc = 90.0f;

	int u0 = 20;
	int v0 = 20;
	int sci = 2;
	int w = sci * 320;
	int h = sci * 240;

	fb = new FrameBuffer(u0, v0, w, h, 0);
	fb->label("First person");
//	fb->show();

	hwfb = new FrameBuffer(u0 /*+ w + 20*/, v0, w, h, 0);
	hwfb->isHW = 1;
	hwfb->label("Fixed Pipeline First person");
	hwfb->show();

	gpufb = new FrameBuffer(u0 + w + 20, v0, w, h, 0);
	gpufb->isHW = 2;
	gpufb->label("GPU First person");
	gpufb->show();



	gui->uiw->position(u0, v0 + fb->h + 60);

	float hfov = 55.0f;
	ppc = new PPC(fb->w, fb->h, hfov);

	tmsN = 4;
	tms = new TM[tmsN];
	tms[0].LoadBin("geometry/teapot57K.bin");
	tms[0].enabled = 1;
	tms[0].textured = false;
	//AABB aabb = tms[1].ComputeAABB();
	//cerr << "INFO: teapot aabb: " << aabb.corners[0] << "; " << aabb.corners[1] << endl;

	AABB aabb = tms[0].ComputeAABB();
	V3 bC = tms[0].GetCenterOfMass();
	bC[1] = aabb.corners[0][1];

	tms[1].SetToBox(bC, V3(200.0f, 0.0f, 200.0f), V3(1.0f, 1.0f, 1.0f));
	tms[1].enabled = 1;
	tms[1].textured = true;

	tms[2].LoadBin("geometry/bunny.bin");
	tms[2].SetColor(V3(0,0,1));
	tms[2].TranslateVertices(V3(0, 10, 70));
	tms[2].enabled = 1;
	tms[2].id = 2;
	tms[2].textured = false;
	tms[2].Scale(V3(200, 200, 200));
	tms[2].enabled = 1;



	ppc->C = tms[0].GetCenterOfMass() + V3(-25.0f, 25.0f, 200);
	L = ppc->C;

	CreateBillboard();


	smppc = 0;
	smfb = 0;
	gfb = 0;


}

void Scene::CreateBillboard() {
	billboard = new FrameBuffer(0, 0, 1024, 1024, -1);

	PPC *cam = new PPC(1024, 1024, 45);
	cam->PositionAndOrient(V3(0, 0, 0), tms[2].GetCenterOfMass(), V3(0, 1, 0));
	RenderBillboard(billboard, cam, 2);

	AABB aabb = tms[2].ComputeAABB();

	double bbWidth = fabs(aabb.corners[0][0] - aabb.corners[1][0]);
	double bbHeight = fabs(aabb.corners[0][1] - aabb.corners[1][1]);

	V3 origin = V3((aabb.corners[0][0] + aabb.corners[1][0] / 2), (aabb.corners[0][1] + aabb.corners[1][1]) / 2, (aabb.corners[0][2] + aabb.corners[1][2]) / 2);
	tms[3].SetToRectangle(origin, V3(bbWidth, bbHeight, 0.0f), V3(0, 0, 0));
	tms[3].enabled = 0;

	billboardtlc = tms[3].verts[0];
	billboardblc = tms[3].verts[1];
	billboardtrc = tms[3].verts[3];

}

void Scene::RenderBillboard(FrameBuffer *currfb, PPC *currppc, int tmi) {
	unsigned int bgr = 0x00000000;
	currfb->Clear(bgr, 0.0f);
	tms[tmi].RenderFilled(currppc, currfb);

}

void Scene::Render(FrameBuffer *currfb, PPC *currppc) {

	unsigned int bgr = 0xFFFF0000;
	currfb->Clear(bgr, 0.0f);

	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (currfb->id == 1) {
			continue;
		}
		if (!tms[tmi].enabled)
			continue;
//		tms[tmi].RenderPoints(ppc, fb);
//		tms[tmi].RenderWireframe(currppc, currfb);
		tms[tmi].RenderFilled(currppc, currfb);

	}

	if (currfb->id == 1) {
		float visz = 40.0f;
		ppc->Visualize(visz, currppc, currfb);
		fb->VisualizeImagePoints(visz, ppc, ppc3, fb3);
		fb->Visualize3DPoints(ppc, ppc3, fb3);
	}

	currfb->Draw3DSegment(L, L + V3(0.0f, 5.0f, 0.0f), V3(1.0f, 1.0f, 0.0f), V3(1.0f, 1.0f, 0.0f), currppc);
	currfb->Draw3DSegment(L, L + V3(5.0f, 0.0f, 0.0f), V3(1.0f, 1.0f, 0.0f), V3(1.0f, 1.0f, 0.0f), currppc);
	currfb->Draw3DSegment(L, L + V3(0.0f, 0.0f, 5.0f), V3(1.0f, 1.0f, 0.0f), V3(1.0f, 1.0f, 0.0f), currppc);

	currfb->redraw();
}

void Scene::ShadowMapSetup() {


	int smw = 128;
	smppc = new PPC(smw, smw, 55.0f);
	smppc->PositionAndOrient(L, tms[1].GetCenterOfMass(), V3(0.0f, 1.0f, 0.0f));
	FrameBuffer *tmpsmfb = new FrameBuffer(100, 100, smw, smw, 2);
	Render(tmpsmfb, smppc);
	tmpsmfb->label("Shadow Map");
	tmpsmfb->show();

	smfb = tmpsmfb;

}


void Scene::DBG() {

	{
		V3 O = tms[0].GetCenterOfMass();
		int framesN = 150;
		for (int fi = 0; fi < framesN; fi++) {
			float fracf = (float)fi / (float)(framesN - 1);
			ppc->SetSlerpInterpolated(O, 1, V3(0, 1, 0));
			RenderAll();
			Fl::check();
		}

		for (int fi = 0; fi < framesN - 20; fi++) {
			float fracf = (float)fi / (float)(framesN - 1);
			ppc->SetSlerpInterpolated(O, 1, V3(-1, 1, 1));
			RenderAll();
			Fl::check();
		}

		for (int fi = 0; fi < framesN - 20; fi++) {
			float fracf = (float)fi / (float)(framesN - 1);
			ppc->SetSlerpInterpolated(O, 1, V3(1, 0, 0));
			RenderAll();
			Fl::check();
		}



	}

	{
		V3 L0 = tms[1].GetCenterOfMass() + V3(0.0f, 0.0f, 100.0f);
		V3 L1 = tms[1].GetCenterOfMass() + V3(0.0f, 100.0f, 0.0f);
		for (int fi = 0; fi < 100; fi++) {
			L = L0 + (L1 - L0)*(float)fi / 99.0f;
			cerr << L << "        \r";
			RenderAll();
			Fl::check();
		}
		return;

	}

	{
		PPC ppc1(*ppc);
		ppc1.LoadFromTextFile("view.txt");
		PPC ppc0(*ppc);
		for (int fi = 0; fi < 100; fi++) {
			ppc->SetInterpolated(&ppc0, &ppc1, (float)fi / 99.0f);
			RenderAll();
			Fl::check();
		}
		return;
	}

	{
		ppc->LoadFromTextFile("view.txt");
		Fl::check();
		ShadowMapSetup();
		Fl::check();
		RenderAll();
		Fl::check();
		int gfbw = smfb->w, gfbh = smfb->h;
		gfb = new GFB(gfbw, gfbh);
		// add sampling locations from output image
		gfb->AddSLs(ppc, fb, smppc);
		gfb->SLStats();
		// render scene from light viewpoint on generalized framebuffer
		for (int tmi = 0; tmi < tmsN; tmi++) {
			if (!tms[tmi].enabled)
				continue;
			tms[tmi].RenderGFB(smppc, gfb);
		}
		// collecting shadow information
		gfb->ApplyShadow(fb);
		fb->redraw();
		Fl::check();
		return;

	}

	{

		V3 L0 = L;
		V3 L1 = L + V3(50.0f, -20.0f, -30.0f);

		ppc->LoadFromTextFile("view.txt");
		int fsN = 30;
		for (int fi = 0; fi < fsN; fi++) {
			L = L0 + (L1 - L0)*(float)fi / (float) fsN;
			RenderAll();
			ShadowMapSetup();
			RenderAll();
			Fl::check();
		}
		L = L0;
		return;

	}

	{
		float s0 = 10000.0f;
		float s1 = 10.0f;
		for (int fi = 0; fi < 100; fi++) {
			specc = s0 + (s1 - s0)*(float)fi / 99.0f;
			RenderAll();
			Fl::check();
		}
		return;

	}

	{
		V3 L0 = tms[1].GetCenterOfMass() + V3(0.0f, 0.0f, 100.0f);
		V3 L1 = tms[1].GetCenterOfMass() + V3(0.0f, 100.0f, 0.0f);
		for (int fi = 0; fi < 100; fi++) {
			L = L0 + (L1 - L0)*(float)fi / 99.0f;
			RenderAll();
			Fl::check();
		}
		return;

	}

	{

		float len = 10.0f;
		tms[1].VisualizeNormals(ppc, fb, len);
		fb->redraw();
		return;

	}

	{

		fb->Clear(0xFFFFFFFF, 0.0f);
		fb->Draw3DPoint(ppc->C+V3(0.0f, 0.0f, -100.0f), V3(0.0f, 0.0f, 1.0f),
			11.2f, ppc);
		fb->redraw();
		return;

	}

	{

		V3 a(1.0f, 2.0f, 0.25f);
		a = a.UnitVector();
		V3 C = tms[1].GetCenterOfMass();
		for (int i = 0; i < 360; i++) {
			tms[1].RotateAboutAxis(C, a, 1.0f);
			Render(fb, ppc);
			Fl::check();
		}
		return;

	}


	{

		PPC ppc0 = *ppc;
		V3 O = tms[0].GetCenterOfMass();
		V3 newC = ppc->C + V3(50.0f, 40.0f, 30.0f);
		ppc->PositionAndOrient(newC, O, V3(0.0f, 1.0f, 0.0f));
		PPC ppc1 = *ppc;
		int framesN = 300;
		for (int fi = 0; fi < framesN; fi++) {
			float fracf = (float)fi / (float)(framesN - 1);
			ppc->SetInterpolated(&ppc0, &ppc1, fracf);
			Render(fb, ppc);
			Fl::check();
		}
		*ppc = ppc0;
		return;

	}

	{

		float hfov = 55.0f;
		PPC ppc(fb->w, fb->h, hfov);

		V3 P(0.0f, 0.0f, -100.0f);
		V3 projP;
		if (ppc.Project(P, projP)) {
			cerr << projP << endl;
			fb->DrawCircle((int) projP[0], (int) projP[1], 5.0f, 0xFF00FF00);
			fb->redraw();
		}
		else {
			cerr << "point behind head" << endl;
		}


		return;

	}

	{

		M33 m;
		m[0] = V3(1.0f, -3.0f, 7.0f);
		m[1] = V3(2.0f, 10.0f, 4.3f);
		m[2] = V3(-8.0f, -50.0f, 1.3f);

		M33 m1 = m.Inverted();
		cerr << m1*m << endl << m*m1 << endl;
		return;


	}


	{
		V3 p0(23.5f, 100.1f, 0.0f);
		V3 p1 = p0 + V3(300.0f, 0.0f, 0.0f);
		V3 p2(23.5f, 300.1f, 0.0f);
		V3 p3 = p2 + V3(300.0f, 0.0f, 0.0f);
		int stepsN = 1000;
		for (int i = 0; i < stepsN; i++) {
			float frac = (float)i / (float)stepsN;
			V3 p01 = p0 + (p1 - p0)*frac;
			V3 p32 = p3 + (p2 - p3)*frac;
			V3 p02 = p0 + (p2 - p0)*frac;
			V3 p31 = p3 + (p1 - p3)*frac;
			fb->SetBGR(0xFFFFFFFF);
			V3 c1(0.0f, 1.0f, 0.0f);
			V3 c0(1.0f, 0.0f, 0.0f);
			fb->Draw2DSegment(p01, p32, c0, c0);
			fb->Draw2DSegment(p02, p31, c1, c1);
			fb->redraw();
			Fl::check();
		}
		return;

	}


	{

		M33 m;
		m[0] = V3(1.0f, 0.0f, 0.0f);
		m[1] = V3(0.0f, 1.0f, 0.0f);
		m[2] = V3(0.0f, 0.0f, 1.0f);
		cerr << m << endl;
		V3 v(2.0f, 10.0f, -1.0f);
		cerr << m*v << endl;
		return;

	}

	{
		int u0 = 100;
		int v0 = 200;
		int u1 = 500;
		int v1 = 400;
		float r = 34.3f;
		int stepsN = 100;
		unsigned int color = 0xFF00FFFF;
		for (int stepi = 0; stepi < stepsN; stepi++) {
			int curru = u0 + (u1 - u0)*stepi / stepsN;
			int currv = v0 + (v1 - v0)*stepi / stepsN;
			fb->SetBGR(0xFFFFFFFF);
			fb->DrawCircle(curru, currv, r, color);
			fb->redraw();
			Fl::check();
		}
		return;
	}

	{
		V3 v0(4.0f, 3.0f, 0.0f);
		V3 v1(5.0f, 2.0f, 1.0f);
		cerr << v0.Length() << endl;
		return;
		cerr << v0 << endl << v1 << endl << v0-v1 << endl;
		return;
		v0[0] = 3.0f;
		cerr << "v0[0]= " << v0[0] << endl;
		return;
	}

	{
		int u0 = 20;
		int v0 = 40;
		int u1 = 400;
		int v1 = 200;
		unsigned int color = 0xFFFF0000;
		fb->Draw2DRectangle(u0, v0, u1, v1, color);
	}

	fb->redraw();
	return;

	for (int u = 0; u < fb->w; u++) {
		fb->Set(u, fb->h / 2, 0xFF000000);
	}

	fb->redraw();

}


void Scene::NewButton() {
	cerr << "INFO: pressed New Button" << endl;
	ppc->SaveToTextFile("view.txt");
}

void Scene::RenderAll() {

	//	Render(fb, ppc);
	if (hwfb) {
		hwfb->redraw();
	}
	if (gpufb) {
		gpufb->redraw();
	}
	//	Render(fb3, ppc3);

}

void Scene::InitializeHW() {

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	texts = new FrameBuffer(0, 0, 1024, 1024, -1);
	texts->SetBWCheckerboard(16);

	unsigned int * texture = new unsigned int[texts->w * texts->h];

	int i = 0;

	for (int v = 0; v < texts->h; v++) {
		for (int u = 0; u < texts->w; u++) {
			texture[i] = texts->Get(u, v);
			i++;
		}
	}

	GLuint checkerboardId;
	glGenTextures(1, &checkerboardId);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, checkerboardId);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texts->w, texts->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	
	tms[1].texId = checkerboardId;


	unsigned int * bbtexture = new unsigned int[billboard->w * billboard->h];

	i = 0;

	for (int v = 0; v < texts->h; v++) {
		for (int u = 0; u < texts->w; u++) {
			texture[i] = billboard->Get(texts->w - u-1, v);
			i++;
		}
	}

	GLuint billboardId;

	glGenTextures(1, &billboardId);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, billboardId);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, billboard->w, billboard->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	billboardTextureId = billboardId;
	tms[0].texId = billboardId;

	cm->InitializeHW();

	HWInitialized = true;
}

void Scene::RenderHW() {
	
	// clear the framebuffer
	glClearColor(0.0, 0.0f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);

	// set view
	// set intrinsics
	ppc->SetIntrinsicsHW(1.0f, 1000.0f);
	// set extrinsics
	ppc->SetExtrinsicsHW();

	if (!HWInitialized) {
		InitializeHW();
	}

	// render geometry
	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (!tms[tmi].enabled)
			continue;
		tms[tmi].RenderHW();
	}

	hwfbCounter++;
	hwfb->SaveImageHW("HW/" + to_string(hwfbCounter) + ".tiff");

}

void Scene::RenderGPU() {
	// if the first time, call per session initialization
	if (cgi == NULL) {
		cgi = new CGInterface();
		cgi->PerSessionInit();
		soi = new ShaderOneInterface();
		soi->PerSessionInit(cgi);
		envoi = new EnvironmentMappingInterface();
		envoi->PerSessionInit(cgi);
		doi = new DiffuseShaderInterface();
		doi->PerSessionInit(cgi);
	}

	if (!HWInitialized) {
		InitializeHW();
	}



	// clear the framebuffer
	glClearColor(0.0, 0.0f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);

	// set view
	// set intrinsics
	ppc->SetIntrinsicsHW(1.0f, 1000.0f);
	// set extrinsics
	ppc->SetExtrinsicsHW();
	
	tms[1].RenderHW();


	cgi->EnableProfiles();

	envoi->PerFrameInit();
	envoi->BindPrograms();
	ppc->createGLImage();
	envoi->PerFrameDisable();



	// per frame initialization

	if (reflections) {
		soi->PerFrameInit();
		soi->BindPrograms();
	}
	// render geometry
	else {
		doi->PerFrameInit();
		doi->BindPrograms();
	}
	tms[0].RenderHW();
	

	if (reflections) {
		soi->PerFrameDisable();
	}
	// render geometry
	else {
		doi->PerFrameDisable();
	}

	doi->PerFrameInit();
	doi->BindPrograms();

	tms[2].RenderHW();


	doi->PerFrameDisable();
	cgi->DisableProfiles();

	gpufbCounter++;
	gpufb->SaveImageHW("GPU/" + to_string(hwfbCounter) + ".tiff");

}

void Scene::EnableFilledMode() {
	for (int i = 0; i < tmsN; i++) {
		tms[i].filledMode = true;
	}
	RenderAll();
}

void Scene::EnableWireframeMode() {
	for (int i = 0; i < tmsN; i++) {
		tms[i].filledMode = false;
	}
	RenderAll();
}

void Scene::ToggleReflectionShader() {
	reflections = ! reflections;
	RenderAll();
}
