#include "global.h"
#include "imgui/imgui.h"

uint iglasttime;
char *imguiFontFile = 0;
float imguiFontSize = 12;

#define BGRA_TO_RGBA(x) ( (((x)&0xFF)<<16) | (((x)&0xFF0000)>>16) | ((x)&0xFF00FF00) )

void ImGuiImpl_RenderDrawLists(ImDrawData *dr)
{
	ImGuiIO &io = ImGui::GetIO();
	if(winMinimized) return;

	renderer->BeginBatchDrawing();
	renderer->EnableScissor();

	for(int i = 0; i < dr->CmdListsCount; i++)
	{
		ImDrawList *cl = dr->CmdLists[i];

		RVertexBuffer *vb = renderer->CreateVertexBuffer(cl->VtxBuffer.size());
		RIndexBuffer *ib = renderer->CreateIndexBuffer(cl->IdxBuffer.size());
		batchVertex *vm = vb->lock();
		for(int j = 0; j < cl->VtxBuffer.size(); j++)
		{
			ImDrawVert *a = &cl->VtxBuffer[j];
			vm[j].x = a->pos.x;
			vm[j].y = a->pos.y;
			vm[j].z = 0;
			vm[j].color = BGRA_TO_RGBA(a->col);
			vm[j].u = a->uv.x;
			vm[j].v = a->uv.y;
		}
		vb->unlock();
		ushort *im = ib->lock();
		for(int j = 0; j < cl->IdxBuffer.size(); j++)
			im[j] = cl->IdxBuffer[j];
		ib->unlock();
		renderer->SetVertexBuffer(vb);
		renderer->SetIndexBuffer(ib);

		int e = 0;
		for(int j = 0; j < cl->CmdBuffer.size(); j++)
		{
			ImDrawCmd *cmd = &cl->CmdBuffer[j];
			//if(cmd->TextureId)
				renderer->SetTexture(0, (texture)cmd->TextureId);
			//else
			//	renderer->NoTexture(0);
			renderer->SetScissorRect(cmd->ClipRect.x, cmd->ClipRect.y,
						cmd->ClipRect.z - cmd->ClipRect.x,
						cmd->ClipRect.w - cmd->ClipRect.y);
			renderer->DrawBuffer(e, cmd->ElemCount);
			e += cmd->ElemCount;
		}

		delete vb; delete ib;
	}

	renderer->DisableScissor();
}

void ImGuiImpl_CreateFontsTexture()
{
	ImGuiIO &io = ImGui::GetIO();
	uchar *pix; int w, h, bpp;
	if(imguiFontFile)
		io.Fonts->AddFontFromFileTTF(imguiFontFile, imguiFontSize);
	io.Fonts->GetTexDataAsRGBA32(&pix, &w, &h, &bpp);
	Bitmap bm;
	bm.w = w; bm.h = h; bm.form = BMFORMAT_B8G8R8A8; bm.pix = pix; bm.pal = 0;
	texture t = CreateTexture(&bm, 1);
	io.Fonts->TexID = (void*)t;
}

void ImGuiImpl_Init()
{
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();

	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.RenderDrawListsFn = ImGuiImpl_RenderDrawLists;
	io.ImeWindowHandle = hWindow;
	ImGuiImpl_CreateFontsTexture();

	iglasttime = GetTickCount();
}

void ImGuiImpl_NewFrame()
{
	ImGuiIO &io = ImGui::GetIO();
	io.DisplaySize = ImVec2(scrw, scrh);

	uint newtime = GetTickCount();
	io.DeltaTime = (newtime - iglasttime) / 1000.f;
	iglasttime = newtime;

	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	ImGui::NewFrame();
}
