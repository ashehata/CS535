#include "CubeMap.h"
#include <math.h>
#include "glext.h"

CubeMap::CubeMap()
{
}

CubeMap::CubeMap(string cubemapPath) {
	FrameBuffer* temp = new FrameBuffer(0, 0, 500, 500, -1);
	temp->LoadImage(cubemapPath);
	int cubeMapSize = temp->w / 3;

	for (int i = 0; i < 6; i++) {
			fbs[i] = new FrameBuffer(0, 0, cubeMapSize, cubeMapSize, i);
	}


	/*Setup up */
	V3 uv;
	for (int u = 0; u < cubeMapSize; u++) {
		for (int v = 0; v < cubeMapSize; v++) {
			uv = V3(u, v, 0);

			/*Front*/
			fbs[2]->Set(uv, temp->Get(u + cubeMapSize, v + cubeMapSize));

			/*Left*/
			fbs[1]->Set(uv, temp->Get(u, v + cubeMapSize));

			/*Back*/
			fbs[0]->Set(uv, temp->Get(cubeMapSize * 2 - u - 1, (cubeMapSize * 4 - 1) - v));

			/*Right*/
			fbs[3]->Set(uv, temp->Get(u + cubeMapSize * 2, v + cubeMapSize));

			/* Top */
			fbs[4]->Set(uv, temp->Get(u + cubeMapSize, v));

			/* Bottom */
			fbs[5]->Set(uv, temp->Get(u + cubeMapSize, v + cubeMapSize * 2));

		}
	}

    ppc = new PPC(cubeMapSize, cubeMapSize, 90);
}
CubeMap::CubeMap(string px, string py, string pz, string nx, string ny, string nz) {
	for (int i = 0; i < 4; i++) {
		fbs[i] = new FrameBuffer(0, 0, 0, 0, i);
	}
	fbs[0]->LoadImage(pz);
	fbs[1]->LoadImage(nx);
	fbs[2]->LoadImage(nz);
	fbs[3]->LoadImage(px);

	int cubeMapSize = fbs[0]->w / 3;
	fbs[4] = new FrameBuffer(0, 0, cubeMapSize, cubeMapSize, 4);
	fbs[5] = new FrameBuffer(0, 0, cubeMapSize, cubeMapSize, 5);

	fbs[4]->SetBGR(0);
	fbs[5]->SetBGR(0);

	ppc = new PPC(cubeMapSize, cubeMapSize, 90);

}

unsigned int CubeMap::Lookup(V3 dir) {
    
	unsigned int retval = 0;
    
    float U = 0, V = 0;
    float X = dir[0];
    float Y = dir[1];
    float Z = dir[2];
    
    int frame = -1;
    
    if (fabs(X) > fabs(Y) && fabs(X) > fabs(Z)){
        if (X < 0){
            frame = 1;
            U = ((1 * Z/fabs(X)) + 1)/2;
            V = ((-1 * Y/fabs(X)) + 1)/2;
        }
        if (X > 0){
            frame = 3;
            U = ((-1 * Z/fabs(X)) + 1)/2;
            V = ((-1 * Y/fabs(X)) + 1)/2;
        }
    }
    
    if (fabs(Y) > fabs(X) && fabs(Y) > fabs(Z)){
        if (Y < 0){
            frame = 5;
            U = ((1 * X/fabs(Y)) + 1)/2;
            V = ((-1 * Z/fabs(Y)) + 1)/2;

        }
        if (Y > 0){
            frame = 4;
            U = ((1 * X/fabs(Y)) + 1)/2;
            V = ((1 * Z/fabs(Y)) + 1)/2;
        }
    }
    
    if (fabs(Z) > fabs(X) && fabs(Z) > fabs(Y)){
        if (Z < 0){
            frame = 0;
            U = ((1 * X/fabs(Z)) + 1)/2;
            V = ((-1 * Y/fabs(Z)) + 1)/2;
        }
        if (Z > 0){
            frame = 2;
            U = ((1 * X/fabs(Z)) + 1)/2;
            V = ((-1 * Y/fabs(Z)) + 1)/2;
        }
    }
    
    
    if (frame == -1){
        return 0;
    }
    float width = fbs[frame]->w;
    float height = fbs[frame]->h;
    
    if (U < 1.0/width){
        U = 1.0/width;
    }
    else if (U > 1.0 - 1.0/width){
        U = 1.0 - 1.0/width;
    }
    
    if (V < 1.0/height){
        V = 1.0/height;
    }
    else if (V > 1.0 - 1.0/height){
        V = 1.0 - 1.0/height;
    }
    
    retval = fbs[frame]->LookUpBilinear(U, V);
    
    if (retval == 0XFFFFFFFF){
        retval = fbs[frame]->LookUpBilinear(U, V);
    }

	return retval;
    
    

}

CubeMap::~CubeMap()
{

}

void CubeMap::InitializeHW() {

	glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, texId);
	
	glTexParameterf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, 0, GL_RGBA8, fbs[0]->w, fbs[0]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbs[0]->pix);

	//tempPix = fbs[1]->ConvertToGL();
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, 0, GL_RGBA8, fbs[1]->w, fbs[1]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbs[1]->pix);
	//delete tempPix;

	//tempPix = fbs[2]->ConvertToGL();
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, 0, GL_RGBA8, fbs[2]->w, fbs[2]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbs[2]->pix);
	//delete tempPix;

	//tempPix = fbs[3]->ConvertToGL();
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT , 0, GL_RGBA8, fbs[3]->w, fbs[3]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbs[3]->pix);
	//delete tempPix;

	//tempPix = fbs[4]->ConvertToGL();
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT , 0, GL_RGBA8, fbs[4]->w, fbs[4]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbs[4]->pix);
	//delete tempPix;

	//tempPix = fbs[5]->ConvertToGL();
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, 0, GL_RGBA8, fbs[5]->w, fbs[5]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbs[5]->pix);
	//delete tempPix;

}