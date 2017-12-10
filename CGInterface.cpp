#include "stdafx.h"

//#define GEOM_SHADER

#include "CGInterface.h"
#include "v3.h"
#include "scene.h"

#include <iostream>

using namespace std;

CGInterface::CGInterface() {};

void CGInterface::PerSessionInit() {

  glEnable(GL_DEPTH_TEST);

  CGprofile latestVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
  CGprofile latestGeometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
  CGprofile latestPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

  if (latestGeometryProfile == CG_PROFILE_UNKNOWN) {
    cerr << "ERROR: geometry profile is not available" << endl;
#ifdef GEOM_SHADER
    exit(0);
#endif
  }

  cgGLSetOptimalOptions(latestGeometryProfile);
  CGerror Error = cgGetError();
  if (Error) {
	  cerr << "CG ERROR: " << cgGetErrorString(Error) << endl;
  }

  cout << "Info: Latest GP Profile Supported: " << cgGetProfileString(latestGeometryProfile) << endl;

  geometryCGprofile = latestGeometryProfile;

  cout << "Info: Latest VP Profile Supported: " << cgGetProfileString(latestVertexProfile) << endl;
  cout << "Info: Latest FP Profile Supported: " << cgGetProfileString(latestPixelProfile) << endl;

  vertexCGprofile = latestVertexProfile;
  pixelCGprofile = latestPixelProfile;
  cgContext = cgCreateContext();  

  GLenum glewError = glewInit();
  if (glewError) {
	  cerr << "GLEW ERROR: " << glewGetErrorString(glewError) << endl;
  }

  cout << "Info: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
}

bool ShaderOneInterface::PerSessionInit(CGInterface *cgi) {

#ifdef GEOM_SHADER
  geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->geometryCGprofile, "GeometryMain", NULL);
  if (geometryProgram == NULL)  {
    CGerror Error = cgGetError();
    cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }
#endif

  vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->vertexCGprofile, "VertexMain", NULL);
  if (vertexProgram == NULL) {
    CGerror Error = cgGetError();
    cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }

  fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->pixelCGprofile, "FragmentMain", NULL);
  if (fragmentProgram == NULL)  {
    CGerror Error = cgGetError();
    cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// build some parameters by name such that we can set them later...
  vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj" );
  vertexSphereRadius = cgGetNamedParameter(vertexProgram, "sphereRadius");
  vertexSphereCenter = cgGetNamedParameter(vertexProgram, "sphereCenter");
  vertexMorphFraction = cgGetNamedParameter(vertexProgram, "morphFraction");
  geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj" );
  fragmentEye = cgGetNamedParameter(fragmentProgram, "eye");
  fragmentLight = cgGetNamedParameter(fragmentProgram, "light");

  billboardTexture = cgGetNamedParameter(fragmentProgram, "billboardTexture");
  billboardtlc = cgGetNamedParameter(fragmentProgram, "tlc");
  billboardtrc = cgGetNamedParameter(fragmentProgram, "trc");
  billboardblc = cgGetNamedParameter(fragmentProgram, "blc");
  cgGLSetTextureParameter(environmentMap, scene->cm->texId);
  cgGLEnableTextureParameter(environmentMap);

  return true;

}

void ShaderOneInterface::PerFrameInit() {

	//set parameters
	cgGLSetStateMatrixParameter(
    vertexModelViewProj, 
		CG_GL_MODELVIEW_PROJECTION_MATRIX, 
    CG_GL_MATRIX_IDENTITY);

  cgGLSetStateMatrixParameter(
    geometryModelViewProj, 
		CG_GL_MODELVIEW_PROJECTION_MATRIX, 
    CG_GL_MATRIX_IDENTITY);


  V3 sphereCenter = scene->tms[1].GetCenterOfMass();
  cgGLSetParameter3fv(vertexSphereCenter, (float*)&sphereCenter);
  AABB aabb = scene->tms[1].ComputeAABB();
  float sphereRadius = (aabb.corners[1] - aabb.corners[0]).Length()/3.0f;
  cgGLSetParameter1f(vertexSphereRadius, sphereRadius);
  cgGLSetParameter1f(vertexMorphFraction, scene->morphFraction);

  cgGLSetParameter3fv(billboardtlc, (float*)&scene->billboardtlc);
  cgGLSetParameter3fv(billboardtrc, (float*)&scene->billboardtrc);
  cgGLSetParameter3fv(billboardblc, (float*)&scene->billboardblc);

  cgGLSetTextureParameter(billboardTexture, scene->billboardTextureId);
  cgGLEnableTextureParameter(billboardTexture);

  cgGLSetTextureParameter(environmentMap, scene->cm->texId);
  cgGLEnableTextureParameter(environmentMap);

  V3 eye = scene->ppc->C;
//  eye = eye + V3(14.0f, 14.0f, 14.0f);
//  eye = eye/150.0f;
//  cerr << "eye:" << eye << endl;
  cgGLSetParameter3fv(fragmentEye, (float*)&eye);
  V3 light = scene->L;
//  light = V3(0.0f, 100.0f, -100.0f);
  cgGLSetParameter3fv(fragmentLight, (float*)&light);

}

void ShaderOneInterface::PerFrameDisable() {
}


void ShaderOneInterface::BindPrograms() {

#ifdef GEOM_SHADER
  cgGLBindProgram(geometryProgram);
#endif
  cgGLBindProgram(vertexProgram);
  cgGLBindProgram(fragmentProgram);

}

void CGInterface::DisableProfiles() {

  cgGLDisableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
  cgGLDisableProfile(geometryCGprofile);
#endif
  cgGLDisableProfile(pixelCGprofile);

}

void CGInterface::EnableProfiles() {

  cgGLEnableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
  cgGLEnableProfile(geometryCGprofile);
#endif
  cgGLEnableProfile(pixelCGprofile);

}

bool EnvironmentMappingInterface::PerSessionInit(CGInterface *cgi) {

#ifdef GEOM_SHADER
	geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/environmentMapping.cg", cgi->geometryCGprofile, "GeometryMain", NULL);
	if (geometryProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}
#endif

	vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/environmentMapping.cg", cgi->vertexCGprofile, "VertexMain", NULL);
	if (vertexProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/environmentMapping.cg", cgi->pixelCGprofile, "FragmentMain", NULL);
	if (fragmentProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// build some parameters by name such that we can set them later...
	vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj");
	vertexSphereRadius = cgGetNamedParameter(vertexProgram, "sphereRadius");
	vertexSphereCenter = cgGetNamedParameter(vertexProgram, "sphereCenter");
	vertexMorphFraction = cgGetNamedParameter(vertexProgram, "morphFraction");
	geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj");
	fragmentEye = cgGetNamedParameter(fragmentProgram, "eye");
	environmentMap = cgGetNamedParameter(fragmentProgram, "environmentMap");

	return true;
	}

void EnvironmentMappingInterface::PerFrameInit() {
	//set parameters
	cgGLSetStateMatrixParameter(vertexModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
#ifdef GEOMETRY_SUPPORT
	cgGLSetStateMatrixParameter(geometryModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
#endif

	cgGLSetParameter3fv(fragmentEye, (float*)&(scene->ppc->C));
	cgGLSetTextureParameter(environmentMap, scene->cm->texId);
	cgGLEnableTextureParameter(environmentMap);
}

void EnvironmentMappingInterface::PerFrameDisable() {
}

void EnvironmentMappingInterface::BindPrograms() {
#ifdef GEOMETRY_SUPPORT
	cgGLBindProgram(geometryProgram);
#endif
	cgGLBindProgram(vertexProgram);
	cgGLBindProgram(fragmentProgram);
}

bool DiffuseShaderInterface::PerSessionInit(CGInterface *cgi) {

#ifdef GEOM_SHADER
	geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/shaderOne.cg", cgi->geometryCGprofile, "GeometryMain", NULL);
	if (geometryProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}
#endif

	vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/diffuseShader.cg", cgi->vertexCGprofile, "VertexMain", NULL);
	if (vertexProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
		"CG/diffuseShader.cg", cgi->pixelCGprofile, "FragmentMain", NULL);
	if (fragmentProgram == NULL) {
		CGerror Error = cgGetError();
		cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// build some parameters by name such that we can set them later...
	vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj");
	vertexSphereRadius = cgGetNamedParameter(vertexProgram, "sphereRadius");
	vertexSphereCenter = cgGetNamedParameter(vertexProgram, "sphereCenter");
	vertexMorphFraction = cgGetNamedParameter(vertexProgram, "morphFraction");
	geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj");
	fragmentEye = cgGetNamedParameter(fragmentProgram, "eye");
	fragmentLight = cgGetNamedParameter(fragmentProgram, "light");

	return true;
}

void DiffuseShaderInterface::PerFrameInit() {
	//set parameters
	cgGLSetStateMatrixParameter(
		vertexModelViewProj,
		CG_GL_MODELVIEW_PROJECTION_MATRIX,
		CG_GL_MATRIX_IDENTITY);

	cgGLSetStateMatrixParameter(
		geometryModelViewProj,
		CG_GL_MODELVIEW_PROJECTION_MATRIX,
		CG_GL_MATRIX_IDENTITY);


	V3 sphereCenter = scene->tms[1].GetCenterOfMass();
	cgGLSetParameter3fv(vertexSphereCenter, (float*)&sphereCenter);
	AABB aabb = scene->tms[1].ComputeAABB();
	float sphereRadius = (aabb.corners[1] - aabb.corners[0]).Length() / 3.0f;
	cgGLSetParameter1f(vertexSphereRadius, sphereRadius);
	cgGLSetParameter1f(vertexMorphFraction, scene->morphFraction);
	V3 eye = scene->ppc->C;
	cgGLSetParameter3fv(fragmentEye, (float*)&eye);
	V3 light = scene->L;
	cgGLSetParameter3fv(fragmentLight, (float*)&light);
}

void DiffuseShaderInterface::PerFrameDisable() {
}

void DiffuseShaderInterface::BindPrograms() {
#ifdef GEOMETRY_SUPPORT
	cgGLBindProgram(geometryProgram);
#endif
	cgGLBindProgram(vertexProgram);
	cgGLBindProgram(fragmentProgram);
}

