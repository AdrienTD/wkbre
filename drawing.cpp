// wkbre - WK (Battles) recreated game engine
// Copyright (C) 2015-2016 Adrien Geets
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "global.h"

int drawblue;

typedef struct {float x, y, z, rhw; DWORD color; float u, v;} MYVERTEX;
#define FVF_MYVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

MYVERTEX verts[] = 
{
    {   0.0f,   0.0f, 0.5f, 1.0f, 0, 0.0f, 0.0f},
    { 639.0f,   0.0f, 0.5f, 1.0f, 0, 1.0f, 0.0f},
    {   0.0f, 479.0f, 0.5f, 1.0f, 0, 0.0f, 1.0f},
    { 639.0f, 479.0f, 0.5f, 1.0f, 0, 1.0f, 1.0f},
};

Matrix idmx = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

/*void SetRect(int x, int y, int w, int h)
{
	verts[0].x = x; verts[0].y = y;
	verts[1].x = x + w; verts[1].y = y;
	verts[2].x = x; verts[2].y = y + h;
	verts[3].x = x + w; verts[3].y = y + h;
}*/

void DrawRect(int x, int y, int w, int h, int c, float u, float v, float o, float p)
{
	verts[0].x = x;		verts[0].y = y;		verts[0].u = u;		verts[0].v = v;
	verts[1].x = x + w;	verts[1].y = y;		verts[1].u = u+o;	verts[1].v = v;
	verts[2].x = x;		verts[2].y = y + h;	verts[2].u = u;		verts[2].v = v+p;
	verts[3].x = x + w;	verts[3].y = y + h;	verts[3].u = u+o;	verts[3].v = v+p;
	for(int i = 0; i < 4; i++) {verts[i].color = c; verts[i].x -= 0.5f; verts[i].y -= 0.5f;}
	ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(MYVERTEX));
}

void DrawGradientRect(int x, int y, int w, int h, int c0, int c1, int c2, int c3)
{
	verts[0].x = x;		verts[0].y = y;		verts[0].u = 0;	verts[0].v = 0;
	verts[1].x = x + w;	verts[1].y = y;		verts[1].u = 1;	verts[1].v = 0;
	verts[2].x = x;		verts[2].y = y + h;	verts[2].u = 0;	verts[2].v = 1;
	verts[3].x = x + w;	verts[3].y = y + h;	verts[3].u = 1;	verts[3].v = 1;
	for(int i = 0; i < 4; i++) {verts[i].x -= 0.5f; verts[i].y -= 0.5f;}
	verts[0].color = c0;	verts[1].color = c1;
	verts[2].color = c2;	verts[3].color = c3;
	ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(MYVERTEX));
}

void InitRectDrawing()
{
	ddev->SetFVF(FVF_MYVERTEX);
	ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
	ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
	ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//ddev->SetRenderState(D3DRS_ZFUNC, D3DCMP_NOTEQUAL);
	ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
}

void BeginDrawing()
{
	ddev->Clear(NULL, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, mapfogcolor, 1.0f, 0);
	ddev->BeginScene();
	ddev->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&idmx);
	ddev->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&idmx);
	//ddev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&idmx);
}

void EndDrawing()
{
	ddev->EndScene();
	drawframes++;
	HRESULT r = ddev->Present(NULL, NULL, NULL, NULL);
	if(r == D3DERR_DEVICELOST)
		{lostdev = 1; Sleep(1000/60);}
	if(r == D3DERR_DEVICENOTRESET)
		{printf("Device not reset.\n"); ResetDevice();}
}

void DoDrawing()
{
	ddev->Clear(NULL, NULL, D3DCLEAR_TARGET, drawblue++, 0.0f, 0);
	ddev->Present(NULL, NULL, NULL, NULL);
	drawframes++;
}

/*IDirect3DVertexShader9 *LoadVertexShader(char *filename)
{
	char *fcnt; int fsize; ID3DXBuffer *serr, *xbuf; IDirect3DVertexShader9 *shader;
	LoadFile(filename, &fcnt, &fsize);
	int i = DLL_D3DXAssembleShader(fcnt, fsize, NULL, NULL, 0, &xbuf, &serr);
	free(fcnt);
	if(FAILED(i)) if(serr) {MessageBox(hWindow, (char*)serr->GetBufferPointer(), 0, 16); ferr("Failed to assemble a vertex shader.");}
	i = ddev->CreateVertexShader((DWORD*)xbuf->GetBufferPointer(), &shader);
	if(FAILED(i)) ferr("CreateVertexShader failed.");
	xbuf->Release();
	if(serr) serr->Release();
	return shader;
}

IDirect3DPixelShader9 *LoadPixelShader(char *filename)
{
	char *fcnt; int fsize; ID3DXBuffer *serr, *xbuf; IDirect3DPixelShader9 *shader;
	LoadFile(filename, &fcnt, &fsize);
	int i = DLL_D3DXAssembleShader(fcnt, fsize, NULL, NULL, 0, &xbuf, &serr);
	free(fcnt);
	if(FAILED(i)) if(serr) {MessageBox(hWindow, (char*)serr->GetBufferPointer(), 0, 16); ferr("Failed to assemble a pixel shader.");}
	i = ddev->CreatePixelShader((DWORD*)xbuf->GetBufferPointer(), &shader);
	if(FAILED(i)) ferr("CreatePixelShader failed.");
	xbuf->Release();
	if(serr) serr->Release();
	return shader;
}*/

void SetTransformMatrix(Matrix *m)
{
	//ddev->SetTransform(D3DTS_VIEW, (D3DMATRIX*)m);
	ddev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)m);
}

void SetTexture(uint x, texture t)
{
	ddev->SetTexture(x, t.dd);
}

void NoTexture(uint x)
{
	ddev->SetTexture(x, 0);
}