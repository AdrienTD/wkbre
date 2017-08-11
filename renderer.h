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

struct batchVertex
{
	union
	{
		struct {float x, y, z;};
		float p[3];
	};
	int color;
	float u, v;
};

struct RBatch
{
	uint maxverts, maxindis;
	uint curverts, curindis;

	virtual ~RBatch() {}
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void flush() = 0;
	virtual void next(uint nverts, uint nindis, batchVertex **vpnt, ushort **ipnt, uint *fi) = 0;
};

struct RVertexBuffer
{
	int size;
	virtual ~RVertexBuffer() {}
	virtual batchVertex *lock() = 0;
	virtual void unlock() = 0;
};

struct RIndexBuffer
{
	int size;
	virtual ~RIndexBuffer() {}
	virtual ushort *lock() = 0;
	virtual void unlock() = 0;
};

struct IRenderer
{
	// Initialisation
	virtual void Init() = 0;
	virtual void Reset() = 0;

	// Frame begin/end
	virtual void BeginDrawing() = 0;
	virtual void EndDrawing() = 0;

	// Textures management
	virtual texture CreateTexture(Bitmap *bm, int mipmaps) = 0;
	virtual void FreeTexture(texture t) = 0;

	// State changes
	virtual void SetTransformMatrix(Matrix *m) = 0;
	virtual void SetTexture(uint x, texture t) = 0;
	virtual void NoTexture(uint x) = 0;
	virtual void SetFog() = 0;
	virtual void DisableFog() = 0;
	virtual void EnableAlphaTest() = 0;
	virtual void DisableAlphaTest() = 0;
	virtual void EnableColorBlend() = 0;
	virtual void DisableColorBlend() = 0;
	virtual void SetBlendColor(int c) = 0;
	virtual void EnableScissor() = 0;
	virtual void DisableScissor() = 0;
	virtual void SetScissorRect(int x, int y, int w, int h) = 0;

	// 2D Rectangles drawing
	virtual void InitRectDrawing() = 0;
	virtual void DrawRect(int x, int y, int w, int h, int c, float u, float v, float o, float p) = 0;
	virtual void DrawGradientRect(int x, int y, int w, int h, int c0, int c1, int c2, int c3) = 0;
	virtual void DrawFrame(int x, int y, int w, int h, int c) = 0;

	// 3D Landscape/Heightmap drawing
	virtual void CreateMapPart(MapPart *p, int x, int y, int w, int h) = 0;
	virtual void FreeMapPart(MapPart *p) = 0;
	virtual void BeginMapDrawing() = 0;
	virtual void DrawPart(MapPart *p) = 0;
	virtual void BeginLakeDrawing() = 0;

	// 3D Mesh drawing
	virtual void CreateMesh(Mesh *m) = 0;
	virtual void BeginMeshDrawing() = 0;
	virtual void DrawMesh(Mesh *m, int iwtcolor) = 0;

	// Batch drawing
	virtual RBatch *CreateBatch(int mv, int mi) = 0;
	virtual void BeginBatchDrawing() = 0;

	// Buffer drawing
	virtual RVertexBuffer *CreateVertexBuffer(int nv) = 0;
	virtual RIndexBuffer *CreateIndexBuffer(int ni) = 0;
	virtual void SetVertexBuffer(RVertexBuffer *_rv) = 0;
	virtual void SetIndexBuffer(RIndexBuffer *_ri) = 0;
	virtual void DrawBuffer(int first, int count) = 0;

	// ImGui
	virtual void InitImGuiDrawing() = 0;
};

IRenderer *CreateD3D9Renderer();
IRenderer *CreateOGL1Renderer();
