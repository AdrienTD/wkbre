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

#ifndef WKBRE_RELEASE

#include "global.h"
#include "imgui/imgui.h"

void OnTestButtonClick(void *param)
{
	printf("You clicked on the button!\n");
	//appexit = 1;
}

//#define dbg(i) putchar((i) + '0')

float ytranslation = 0.0f, scaling = 1.0f;

void SetModelMatrices()
{
	Matrix matWorld, rotMatrix, transMatrix, scaleMatrix;
	CreateRotationYMatrix(&rotMatrix, (timeGetTime()%2000)*(2.0f*M_PI)/2000.0f);
	CreateTranslationMatrix(&transMatrix, 0.0f, ytranslation, 0.0f);
	CreateScaleMatrix(&scaleMatrix, scaling, scaling, scaling);
	MultiplyMatrices(&matWorld, &rotMatrix, &transMatrix);
	//MultiplyMatrices(&matWorld, &matWorld, &scaleMatrix);
	matWorld *= scaleMatrix;
	Matrix finalmatrix = matWorld;

	Vector3 vEyePt( 0.0f, 18.0f,-20.0f );
	Vector3 vLookatPt( 0.0f, 0.0f, 0.0f );
	Vector3 vUpVec( 0.0f, 1.0f, 0.0f );
	Matrix matView;
	CreateLookAtLHViewMatrix( &matView, &vEyePt, &vLookatPt, &vUpVec );
	finalmatrix *= matView;

	Matrix matProj;
	CreatePerspectiveMatrix( &matProj, M_PI / 4, 1.0f, 1.0f, 100.0f );
	finalmatrix *= matProj;

	SetTransformMatrix(&finalmatrix);
}

void Test1()
{
	LoadBCP("data.bcp");
	char *t; int s;
	//LoadFile("Fonts\\Proportional\\20\\Proportional - 20.font", &t, &s);
	//LoadFile("Animations_Are_Guaranteed_To_Merge.txt", &t, &s);
	//LoadFile("Campaigns\\Kingmaker.cam", &t, &s);
	//LoadFile("test\\abc.txt", &t, &s);
	//LoadFile("Languages\\..\\Animations_Are_Guaranteed_To_Merge.txt", &t, &s);
	LoadFile("Warrior Kings Game Set\\Mechanics\\Recon.cpp", &t, &s);
	fwrite(t, s, 1, stdout);
	free(t);
}

void Test2()
{
	LoadBCP("data.bcp");
	ReadLanguageFile();
	printf("%s\n", GetLocText("AI_BIOG_BLACKLERE_THE_PEASANT"));
}

void Test3()
{
	LoadBCP("data.bcp");
	ReadLanguageFile();
	InitWindow();
	//InitRectDrawing();
	//IDirect3DTexture9 *mytex = GetTexture("Interface\\LOADING SCREEN 1.tga");
	//IDirect3DPixelShader9 *psh = LoadPixelShader("test.psh");
	InitScene(); LoadMap("testmap.bcm");
	while(!appexit)
	{
		BeginDrawing();
		//ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
		//ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		//ddev->SetPixelShader(psh);
		//ddev->SetTexture(0, mytex);
		//DrawRect(0, 0, 640, 480, -1);
		DrawScene();
		EndDrawing();
		HandleWindow();

		if(keypressed[VK_UP]) camerapos += Vector3(0.0f, 0.0f, walkstep);
		if(keypressed[VK_DOWN]) camerapos -= Vector3(0.0f, 0.0f, walkstep);
		if(keypressed[VK_LEFT]) camerapos -= Vector3(walkstep, 0.0f, 0.0f);
		if(keypressed[VK_RIGHT]) camerapos += Vector3(walkstep, 0.0f, 0.0f);
		if(keypressed['E']) camerapos += Vector3(0.0f, walkstep, 0.0f);
		if(keypressed['D']) camerapos -= Vector3(0.0f, walkstep, 0.0f);
	}
}

void Test4()
{
	LoadBCP("data.bcp");
	InitWindow();
	Mesh tm("Warrior Kings Game Set\\Characters\\Abaddon\\ABADDON_MOVE.MESH3");
	Mesh tn("Warrior Kings Game Set\\Characters\\Michael\\MICHAEL1.MESH3");
	//InitMeshDrawing();
	while(!appexit)
	{
		BeginDrawing();
		BeginMeshDrawing();
		SetModelMatrices();
		tm.draw(1);
		tn.draw(2);
		EndDrawing();
		HandleWindow();
	}
}

void Test5()
{
	LoadBCP("data.bcp");
	ReadLanguageFile();
	InitWindow();
	LoadGameSet("Warrior Kings Game Set\\small extensions.cpp");
	printf("Game set loaded!\n");

	printf("Declared items:\n");
	for(int i = 0; i < strItems.len; i++)
		printf(" - %s\n", strItems[i]);

	printf("Defined values:\n");
	for(int i = 0; i < defvalue.len; i++)
		printf(" - %s = %f\n", strDefValue[i], defvalue[i]);

	printf("Appearance tags:\n");
	for(int i = 0; i < strAppearTag.len; i++)
		printf(" - %s\n", strAppearTag[i]);

	printf("ObjDefs:\n");
	for(int i = 0; i < strObjDef.len; i++)
		printf(" - %s (%s)\n", strObjDef[i], CLASS_str[typeObjDef[i]]);

	printf("Looking at first object:\n");
	printf(" - Tooltip: %s\n", objdef[0].tooltip);
	printf(" - First start item: %f\n", objdef[0].startItems[0]);
	printf(" - Shadow: %i\n", objdef[0].shadowtype);

	printf("2nd's subtypes:\n");
	for(int i = 0; i < pstObjDef[1]->len; i++)
		printf(" - %s\n", (pstObjDef[1])->getdp(i));


	printf("1st's subtypes (%i):\n", pstObjDef[0]->len);
	for(int i = 0; i < pstObjDef[0]->len; i++)
		printf(" - %s\n", (pstObjDef[0])->getdp(i));

	printf("These ObjDefs have \"Default\" subtype and \"Default\" appearance:\n");
	for(int i = 0; i < strObjDef.len; i++)
		printf("- %s: %s\n", strObjDef[i], objdef[i].subtypes[0].appear[0].def?"Yes":"No"); // :S

	getch();

	//InitMeshDrawing();
	int a = FindObjDef(CLASS_BUILDING, "Manor");
	while(!appexit)
	{
		BeginDrawing();
		BeginMeshDrawing();
		SetModelMatrices();
		objdef[a].subtypes[0].appear[0].def->draw(); // :S
		EndDrawing();
		HandleWindow();
	}
}

char *t6fn[] = {"version.txt", "test.txt", "This Is New Data.txt",
	"Warrior Kings Game Set\\Buildings\\Church\\Church.cpp", "test\\abc.txt",
	"Languages\\..\\Animations_Are_Guaranteed_To_Merge.txt"};
void Test6()
{
	LoadBCP("data.bcp");
	for(int i = 0; i < 6; i++)
		printf("%s: %i\n", t6fn[i], FileExists(t6fn[i]));
}

void Test8()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	printf("Game set loaded!\n");
	printf("Num. objdefs.: %i\n", strObjDef.len);
	for(int i = 0; i < strObjDef.len; i++)
		{printf(" - %s, ", strObjDef[i]);
		//fflush(stdout);
		printf("%s, %i\n", CLASS_str[objdef[i].type], objdef[i].renderable);}
	printf("Num. items: %i\n", strItems.len);
	for(int i = 0; i < strItems.len; i++)
		printf(" - %s\n", strItems.get(i));
	getch();
}

void Test9()
{
	//GrowStringList t;
	//t.add("Red"); t.add("Green"); t.add("Blue"); t.add("Yellow");
	//for(int i = 0; i < t.len; i++)
	//	printf("Color %i: %s\n", i, t.get(i));

	LoadBCP("data.bcp");
	GrowStringList *t = ListFiles("Warrior Kings Game Set\\Sounds\\narration"); //""); //"Interface\\Icons\\Special Icons\\");
	for(int i = 0; i < t->len; i++)
		printf("%04i: %s\n", i, t->getdp(i));

	int x = ListDlgBox(t);
	if(x != -1)
		printf("Selected: %i (%s)\n", x, t->getdp(x));

	delete t;
}

void Test10()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	CObjectDefinition *d = AskObjDef("Read starting item values of object definition:");
	if(!d) return;
	for(int i = 0; i < strItems.len; i++)
		if(d->startItems[i] != 0.0f)
			printf("%s = %f\n", strItems.getdp(i), d->startItems[i]);
}

void Test11()
{
	DynList<int> a, b;

	a.add(7); a.add(13); a.add(9); a.add(1);
	b.add(12); b.add(5); b.add(2); b.add(3);

	a.move(a.getEntry(2), &b);

	a.move(a.first, &b);
	a.move(a.first, &b);
	a.move(a.last, &b);

	b.move(b.first, &a);

	printf("A:");
	for(DynListEntry<int> *e = a.first; e; e = e->next)
		printf("\t%i", e->value);
	printf("\nB:");
	for(DynListEntry<int> *e = b.first; e; e = e->next)
		printf("\t%i", e->value);
	printf("\n");
}

void Test12()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	for(int i = 0; i < strDiplomaticStatus.len; i++)
	{
		printf("%i: %s", i, strDiplomaticStatus.getdp(i));
		printf((i == defaultDiploStat)?" (default)\n":"\n");
	}
	getch();
}

void Test13()
{
	char s[256];
	if(StrDlgBox(s))
		MessageBox(0, s, appName, 64);
}

void Test14()
{
	InitWindow();
	LoadBCP("data.bcp"); InitFont();
	ReadLanguageFile(); char *msg = GetLocText("AI_BIOG_PAUL_THE_SIMPLE");
	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();
		//DrawRect(2, 2, 64, 64, 0xFF);
		//for(int i = 0; i < 40; i++)
		//	DrawFont(i*12, i*12, "Hello! \x21\x22\x7d\x7e\xa1\xa2\xa3");
		//DrawFont(0, 0, msg);
		deffont->DrawInRect(scrw, 0, 0, msg, 0xFF00FFFF);
		EndDrawing();
		HandleWindow();
	}
}

char *mbs[2] = {"B1", "B2"};
MenuEntry mbe[] = {
{"x1", 1},
{"x2", 2},
{"x3", 3},
{0,0},
{"y1", 11},
{"y2", 12},
{0,0},
};

void Test15()
{
	InitWindow(); LoadBCP("data.bcp"); InitFont();

/*
	GUIPage gpage;
	GUISimpleButton gbutton(2, 2, 64, 64, OnTestButtonClick);
	GUITextButton gtxtbutton(2, 80, 100, 16, OnTestButtonClick, "Hello!");
	gpage.add(&gbutton); gpage.add(&gtxtbutton);
	//actualpage = &gpage;
	InitMenuBar();
	menubar.parent = &gpage;
	actualpage = &menubar;
*/

	actualpage = new GEContainer;
	GEMenuBar *b = new GEMenuBar(mbs, mbe, 2, actualpage);
	actualpage->add(b);
	b->setRect(0, 0, 640, 20);

	GEWindow *win = new GEWindow;
	actualpage->add(win);
	win->setCaption("Test15");
	win->setRect(32, 32, 150, 150);
	win->onResize();
	win->bgColor = 0x80008000;

	GETextButton *t = new GETextButton;
	win->add(t);
	t->setRect(16, 36, 64, 64);
	t->setCaption("LOL!");

	GEScrollBar *sb = new GEScrollBar;
	win->add(sb);
	sb->setRect(90, 36, 16, 80);

	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();
		actualpage->draw(0, 0);
		EndDrawing();
		HandleWindow();
	}

}

void Test16()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");

	// Investigate the "invisible explosive cart" issue.

	int ecdef = FindObjDef(CLASS_CHARACTER, "Explosive Cart"); mustbefound(ecdef);
	printf("#%i\n", ecdef);
	//__asm int 3
}

void Test17()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadSaveGame("Save_Games\\rescript1.sav");

	SequenceEnv se; char inbuf[256];

	printf("Equations:\n");
	for(int i = 0; i < strEquation.len; i++)
		printf(" %i: %s\n", i+1, strEquation.getdp(i));

	while(1)
	{
		printf("\nEquation number: ");
		int x = atoi(fgets(inbuf, 255, stdin))-1;
		if(x == -1) break;
		if((uint)x >= strEquation.len)
			printf("Eq. number does not exist.\n");
		else if(!equation[x])
			printf("Equation is null.\n");
		else
			printf("Result: %f\n", equation[x]->get(&se));
	}
};

void Test18()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadSaveGame("Save_Games\\rescript1.sav");

	SequenceEnv se; char inbuf[256];
	goref gr; gr = FindObjID(3102);
	se.self = gr;

	printf("Object finder definitions:\n");
	for(int i = 0; i < strFinderDef.len; i++)
		printf(" %i: %s\n", i+1, strFinderDef.getdp(i));

	while(1)
	{
		printf("\nFinder def. number: ");
		int x = atoi(fgets(inbuf, 255, stdin))-1;
		if(x == -1) break;
		if((uint)x >= strFinderDef.len)
			printf("OFD number does not exist.\n");
		else if(!finderdef[x])
			printf("OFD is null.\n");
		else
		{
			printf("Result:");
			CFinder *f = finderdef[x]; GameObject *o;
			f->begin(&se);
			while(o = f->getnext()) printf(" %u", o->id);
			printf("\n");
		}
	}
};

void Test19()
{
	LoadBCP("data.bcp"); ReadLanguageFile(); InitWindow();
	LoadGameSet("Warrior Kings Game Set\\multi-player extensions.cpp");
	printf("--- GAME_EVENT ---\n");
	for(int i = 0; i < strGameEvent.len; i++)
		printf("%s\n", strGameEvent.getdp(i));
	printf("--- PACKAGE_RECEIPT_TRIGGER ---\n");
	for(int i = 0; i < strPRTrigger.len; i++)
		printf("%s\n", strPRTrigger.getdp(i));
	printf("--- Both ---\n");
	for(int i = 0; i < strGameEvent.len; i++)
		if(strPRTrigger.find(strGameEvent.getdp(i)) != -1)
			printf("%s\n", strGameEvent.getdp(i));
};

void Test20()
{
	playMode = 1;
	Test7();
}

void Test21()
{
	InitWindow();
	while(!appexit)
	{
		BeginDrawing();
		EndDrawing();
		HandleWindow();
		Sleep(1000/30);
	}
}

void Test22()
{
	int curgrp = 0; char tb[512];

	printf("Use LTTT? ");
	int uselt = atoi(fgets(tb, 511, stdin));

	LoadBCP("data.bcp");
	InitWindow();
	InitFont();

	if(uselt) LoadLTTT("Maps\\Map_Textures\\128_4Peaks.lttt");
	LoadMapTextures();

	//MessageBox(hWindow, "Terrain textures loaded.", appName, 64);
	printf("%u groups:\n", maptexgroup.len);
	for(int i = 0; i < maptexgroup.len; i++)
	{
		MapTextureGroup *g = maptexgroup.getpnt(i);
		printf(" * %s\n", g->name);
		printf("   %u textures:\n", g->tex->len);
		for(int j = 0; j < g->tex->len; j++)
			printf("    - %u\n", g->tex->getpnt(j)->id);
	}
	//texture t = GetTexture("Warrior Kings Game Set\\Textures\\Tavern.pcx");
	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();

		//SetTexture(0, maptexgroup.getpnt(0)->tex->getpnt(0)->t);
		//SetTexture(0, t);
		//DrawRect(0, 0, 256, 256, -1);

		MapTextureGroup *g = maptexgroup.getpnt(curgrp);
		sprintf(tb, "%s (%u/%u)", g->name, curgrp, maptexgroup.len);
		DrawFont(0, 0, tb);
		for(int i = 0; i < g->tex->len; i++)
		{
			MapTexture *t = g->tex->getpnt(i);
			SetTexture(0, t->t);
			DrawRect(i * 65, 32, 64, 64, -1, t->x / 256.f, t->y / 256.f, t->w / 256.f, t->h / 256.f);
			sprintf(tb, "%u", t->id);
			DrawFont(i * 65, 96, tb);
		}

		if(keypressed[VK_LEFT])
		{
			keypressed[VK_LEFT] = 0;
			if(curgrp > 0) curgrp--;
		}
		if(keypressed[VK_RIGHT])
		{
			keypressed[VK_RIGHT] = 0;
			if(curgrp < maptexgroup.len-1) curgrp++;
		}

		EndDrawing();
		HandleWindow();
	}
}

int AskMap(char *o, char *hs = 0)
{
	GrowStringList *sl = ListDirectories("Maps");
	int x = ListDlgBox(sl, hs ? hs : "Select a map.", 0);
	if(x == -1) return 0;
	strcpy(o, "Maps\\");
	strcat(o, sl->getdp(x));
	strcat(o, "\\");
	strcat(o, sl->getdp(x));
	int e = strlen(o);
	strcpy(o+e, ".bcm");
	if(FileExists(o)) return 1;
	strcpy(o+e, ".snr");
	if(FileExists(o)) return 1;
	MessageBox(hWindow, "The selected map directory does not contain a BCM nor a SNR file.", appName, 48);
	return 0;
}

void Test23()
{
	int px = 0, pz = 0, ps = 64;

	LoadBCP("data.bcp");
	ReadLanguageFile();
	InitWindow();
	InitFont();
	//LoadSaveGame("Save_Games\\Advanced Economy.lvl");
	//LoadMap("Maps\\oritest.bcm");
	char mn[512];
	if(!AskMap(mn, "Select a map you want to view.")) return;
	LoadMap(mn);

	DynList<MapTile*> *ttlist = new DynList<MapTile*>[maptexfilenames.len];
	for(int z = 0; z < mapheight; z++)
	for(int x = 0; x < mapwidth; x++)
	{
		MapTile *t = &maptiles[z*mapwidth+x];
		ttlist[t->mt->tfid].add(t);
	}

	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();

		//RBatch *batch = renderer->CreateBatch(1024, 1536);
		RBatch *batch = renderer->CreateBatch(2*8192, 3*8192);
		renderer->BeginBatchDrawing();
		Matrix mx; CreateZeroMatrix(&mx);
		mx._11 = 2.0f / scrw; mx._22 = -2.0f / scrh;
		mx._41 = -1 - (0.5f / (scrw)); mx._42 = 1 + (0.5f / (scrh));
		//mx._41 = -1; mx._42 = 1;
		mx._44 = 1;
		renderer->SetTransformMatrix(&mx);

		for(int i = 0; i < maptexfilenames.len; i++)
		{
			int tns = 1;
			for(DynListEntry<MapTile*> *e = ttlist[i].first; e; e = e->next)
			{
				MapTile *c = e->value;
				int dx = (c->x - px) * ps, dy = (c->z - pz) * ps;
				if(((uint)dx >= scrw) || ((uint)dy >= scrh)) continue;
				MapTexture *t = c->mt;
				if(tns) {SetTexture(0, t->t); tns = 0;}

				batchVertex *bv; ushort *bi; uint fi;
				batch->next(4, 6, &bv, &bi, &fi);
				int a = c->rot, f = ((c->zflip&1)?1:0) ^ ((c->xflip&1)?3:0);

				bv[0].x = dx;			bv[0].y = dy;
				bv[1].x = dx + ps;		bv[1].y = dy;
				bv[2].x = dx + ps;		bv[2].y = dy + ps;
				bv[3].x = dx;			bv[3].y = dy + ps;

				bv[((0^f)+a)&3].u = t->x / 256.f;		bv[((0^f)+a)&3].v = t->y / 256.f;
				bv[((1^f)+a)&3].u = (t->x+t->w) / 256.f;	bv[((1^f)+a)&3].v = t->y / 256.f;
				bv[((2^f)+a)&3].u = (t->x+t->w) / 256.f;	bv[((2^f)+a)&3].v = (t->y+t->h) / 256.f;
				bv[((3^f)+a)&3].u = t->x / 256.f;		bv[((3^f)+a)&3].v = (t->y+t->h) / 256.f;

				for(int j = 0; j < 4; j++)
					{bv[j].z = 0; bv[j].color = -1;
					}//bv[j].x -= 0.5f; bv[j].y -= 0.5f;}
					 // L-> this will drop FPS because it rereads the locked buffer!
				bi[0] = fi+0; bi[1] = fi+1; bi[2] = fi+3;
				bi[3] = fi+1; bi[4] = fi+3; bi[5] = fi+2;
			}
			batch->flush();
		}
		delete batch;

		if(keypressed[VK_LEFT])
			{keypressed[VK_LEFT] = 0; px--;}
		if(keypressed[VK_RIGHT])
			{keypressed[VK_RIGHT] = 0; px++;}
		if(keypressed[VK_UP])
			{keypressed[VK_UP] = 0; pz--;}
		if(keypressed[VK_DOWN])
			{keypressed[VK_DOWN] = 0; pz++;}
		if(keypressed[VK_MULTIPLY])
			{keypressed[VK_MULTIPLY] = 0; ps <<= 1;}
		if(keypressed[VK_DIVIDE])
			{keypressed[VK_DIVIDE] = 0; ps >>= 1;}
		if(ps <= 0) ps = 1;

		EndDrawing();
		HandleWindow();
	}
}

void DrawAnim(Anim *anim, RBatch *batch, int uvl, int tm)
{
	for(int i = 0; i < anim->mesh->ngrp; i++)
	{
		SetTexture(0, anim->mesh->lstmattex[i]);
		if(anim->mesh->lstmatflags[i]&1) renderer->EnableAlphaTest();
		else renderer->DisableAlphaTest();
		anim->drawInBatch(batch, 0, uvl, -1, tm);
		batch->flush();
	}
}

void Test24()
{
	LoadBCP("data.bcp");
	InitWindow();
	Anim tm("Warrior Kings Game Set\\Characters\\Abaddon\\GRAB1.ANIM3");
	//Anim tm("Warrior Kings Game Set\\Characters\\Michael\\SPECIALMOVE.ANIM3");
	RBatch *batch = renderer->CreateBatch(16384, 16384);

	Matrix m1, m2;
	CreateScaleMatrix(&m1, 1.5, 1.5, 1.5);
	CreateTranslationMatrix(&m2, 0, -4, 0);
	mWorld = m1 * m2;

	while(!appexit)
	{
		BeginDrawing();
		BeginMeshDrawing();
		renderer->BeginBatchDrawing();
		SetModelMatrices();
		//CreateIdentityMatrix(&mWorld);


		//tm.mesh->draw(1);
		//tn.draw(2);

		//tm.drawInBatch(batch, 0, 1, -1, 0);
		//batch->flush();

		DrawAnim(&tm, batch, 1, timeGetTime());

		EndDrawing();
		HandleWindow();
	}
}

void Test25()
{
	LoadBCP("data.bcp"); InitWindow();

	float x1 = 0, x2 = 0;
	texture t1, t2, t3;
	t1 = GetTexture("Interface\\FrontEnd\\Background_Scroll_1.tga");
	t2 = GetTexture("Interface\\FrontEnd\\Background_Scroll_2.tga");
	t3 = GetTexture("Interface\\FrontEnd\\Background_Static_2.tga");

	while(!appexit)
	{
		BeginDrawing();
		InitRectDrawing();
		SetTexture(0, t1); DrawRect(0, 0, scrw, scrh, -1, x1, 0, 1, 1);
		SetTexture(0, t2); DrawRect(0, 0, scrw, scrh, -1, x2, 0, 1, 1);
		SetTexture(0, t3); DrawRect(0, 0, scrw, scrh, -1, 0, 0, 1, 1);
		x1 += 0.0003f; x2 -= 0.0003f;
		if(x1 >= 1) x1 -= 1; if(x2 < 0) x2 += 1;
		EndDrawing();
		HandleWindow();
	}
}

void Test26()
{
	LoadBCP("data.bcp"); InitWindow();
	ImGuiImpl_Init();

	float angle, vec[3];
	const char *lbs[] = {"Red", "Green", "Blue"};
	int lbc = 0;

	while(!appexit)
	{
		ImGuiImpl_NewFrame();
		ImGui::Text("Hello, world!");
		ImGui::InputFloat3("Vector3", vec);
		ImGui::SliderAngle("Angle", &angle);
		ImGui::ListBox("List box", &lbc, lbs, 3, -1);
		BeginDrawing();
		renderer->InitImGuiDrawing();
		ImGui::Render();
		EndDrawing();
		HandleWindow();
	}
}

MapTextureGroup *seltexgrp[2] = {0,0};
MapTexture *seltex[2] = {0,0}, *autotex = 0;
int selrot[2] = {0,0}, atrot;
bool selxflip[2] = {0,0}, selzflip[2] = {0,0}, atxflip, atzflip;
GrowList<MapTextureEdge*> atlist;
MapTextureEdge *autote = 0;

int TransformEdge(int edge, int rot, boolean xflip, boolean zflip)
{
	int d = (edge + rot) & 3;
	if(xflip) if(!(d&1)) d ^= 2;
	if(zflip) if(d&1) d ^= 2;
	return d;
}

void DoAutotile()
{
	atlist.clear(); autote = 0; autotex = 0;
	if(!(seltex[0] && seltex[1])) return;
	int da, db;
	GrowList<MapTextureEdge> *gle0 = &seltex[0]->atdir[da = TransformEdge(MAPTEXDIR_SOUTH, -selrot[0], selxflip[0], selzflip[0])];
	GrowList<MapTextureEdge> *gle1 = &seltex[1]->atdir[db = TransformEdge(MAPTEXDIR_NORTH, -selrot[1], selxflip[1], selzflip[1])];
	printf("N:%i\nS:%i\n", da, db);
	for(int i = 0; i < gle0->len; i++)
	{
		MapTextureEdge *e0 = gle0->getpnt(i);
		for(int j = 0; j < gle1->len; j++)
		{
			MapTextureEdge *e1 = gle1->getpnt(j);
			if(!memcmp(e0, e1, sizeof(MapTextureEdge)))
			{
				atlist.add(e0);
			/*
				autotex = e0->tex;
				atrot = e0->rot;
				atxflip = e0->xflip;
				atzflip = e0->zflip;
				return;
			*/
			}
		}
	}
}

void IGAutotileTest()
{
if(ImGui::CollapsingHeader("Autotile explorer"))
{
	for(int i = 0; i < maptexgroup.len; i++)
	{
		MapTextureGroup *g = maptexgroup.getpnt(i);
		if(ImGui::TreeNode(g->name))
		{
			for(int j = 0; j < g->tex->len; j++)
			{
				MapTexture *t = g->tex->getpnt(j);
				if(ImGui::TreeNode((void*)j, "%i", t->id))
				{
					for(int k = 0; k < 4; k++)
					{
						if(ImGui::TreeNode((void*)k, "%s (%i elements)", MAPTEXDIR_str[k], t->atdir[k].len))
						{
							for(int l = 0; l < t->atdir[k].len; l++)
							{
								MapTextureEdge *e = t->atdir[k].getpnt(l);
								ImGui::Text("%s %i, Rot:%i XFlip:%i ZFlip:%i", e->tex->grp->name, e->tex->id, e->rot, e->xflip, e->zflip);
							}
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}
}
if(ImGui::CollapsingHeader("Autotile test"))
{
	ImGui::Columns(2);
	for(int n = 0; n < 2; n++)
	{
		ImGui::PushItemWidth(-1);
		ImGui::PushID(n);
		ImGui::TextColored(n ? ImVec4(0,1,1,1) : ImVec4(1,0,1,1), n ? "--- South ---" : "--- North---");
		ImGui::Text("Group:");
		ImGui::ListBoxHeader("##Group");
		for(int i = 0; i < maptexgroup.len; i++)
		{
			MapTextureGroup *g = maptexgroup.getpnt(i);
			if(ImGui::Selectable(g->name, seltexgrp[n] == g))
				{seltexgrp[n] = g; seltex[n] = g->tex->getpnt(0);}
		}
		ImGui::ListBoxFooter();

		ImGui::BeginChild(n ? "Texture1" : "Texture0", ImVec2(0,96), true, ImGuiWindowFlags_HorizontalScrollbar);
		MapTextureGroup *g = seltexgrp[n];
		if(g)
		for(int i = 0; i < g->tex->len; i++)
		{
			MapTexture *t = g->tex->getpnt(i);
			boolean ss = seltex[n] == t;
			ImGui::PushID(i);
			if(ss)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1,0,0,1));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,0.3f,0.3f,1));
			}
			if(ImGui::ImageButton((ImTextureID)t->t, ImVec2(64, 64), ImVec2(t->x/256.f, t->y/256.f), ImVec2((t->x+t->w)/256.f, (t->y+t->h)/256.f)))
				seltex[n] = t;
			if(ImGui::IsItemHovered())
				ImGui::SetTooltip("ID: %i", t->id);
			if(ss) ImGui::PopStyleColor(2);
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::NewLine();
		ImGui::EndChild();

		ImGui::Text("Rotation:"); ImGui::SameLine();
		ImGui::RadioButton("0°##rot", &selrot[n], 0); ImGui::SameLine();
		ImGui::RadioButton("90°##rot", &selrot[n], 1); ImGui::SameLine();
		ImGui::RadioButton("180°##rot", &selrot[n], 2); ImGui::SameLine();
		ImGui::RadioButton("270°##rot", &selrot[n], 3);
		ImGui::Text("Flip:"); ImGui::SameLine();
		ImGui::Checkbox("X", &selxflip[n]); ImGui::SameLine();
		ImGui::Checkbox("Z", &selzflip[n]);

		ImGui::PopID();
		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::TextColored(ImVec4(0,1,0,1), "--- Autotile ---");
	if(ImGui::Button("Find"))
		DoAutotile();

	ImGui::BeginChild("AutotileResult", ImVec2(0,96), true, ImGuiWindowFlags_HorizontalScrollbar);
	for(int i = 0; i < atlist.len; i++)
	{
		MapTextureEdge *e = atlist[i];
		MapTexture *t = e->tex;
		boolean ss = e == autote;
		ImGui::PushID(i);
		if(ss)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1,0,0,1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,0.3f,0.3f,1));
		}
		if(ImGui::ImageButton((ImTextureID)t->t, ImVec2(64, 64), ImVec2(t->x/256.f, t->y/256.f), ImVec2((t->x+t->w)/256.f, (t->y+t->h)/256.f)))
		{
			autote = e;
			autotex = e->tex;
			atrot = e->rot;
			atxflip = e->xflip;
			atzflip = e->zflip;
		}
		if(ImGui::IsItemHovered())
			ImGui::SetTooltip("%s %i\nRot:%i XFlip:%i ZFlip:%i", t->grp->name, t->id, e->rot, e->xflip, e->zflip);
		if(ss) ImGui::PopStyleColor(2);
		ImGui::PopID();
		ImGui::SameLine();
	}
	ImGui::NewLine();
	ImGui::EndChild();

	for(int i = 0; i < 3; i++)
	{
		MapTexture *t = (i==0) ? seltex[0] : ((i==1) ? autotex : seltex[1]);
		if(t)
		{
			ImGui::Image((ImTextureID)t->t, ImVec2(64, 64), ImVec2(t->x/256.f, t->y/256.f), ImVec2((t->x+t->w)/256.f, (t->y+t->h)/256.f));
			ImGui::SameLine();
			ImGui::BeginGroup();
			if(i==0)	ImGui::TextColored(ImVec4(1,0,1,1), "North");
			else if(i==1)	ImGui::TextColored(ImVec4(0,1,0,1), "Autotile");
			else if(i==2)	ImGui::TextColored(ImVec4(0,1,1,1), "South");
			ImGui::Text("%s %i", t->grp->name, t->id);
			if(i==1) ImGui::Text("Rot:%i XFlip:%i ZFlip:%i", atrot, atxflip, atzflip);
			else ImGui::Text("Rot:%i XFlip:%i ZFlip:%i", selrot[i/2], selxflip[i/2], selzflip[i/2]);
			ImGui::EndGroup();
		}
	}
}
}

void Test27()
{
	scrw = 1024; scrh = 768;
	LoadBCP("data.bcp"); InitWindow();
	ImGuiImpl_Init();
	LoadMapTextures();
	//mapfogcolor = 0xFF204080;

	while(!appexit)
	{
		ImGuiImpl_NewFrame();
		IGAutotileTest();

		BeginDrawing();
		renderer->InitImGuiDrawing();
		ImGui::Render();
		EndDrawing();
		HandleWindow();
	}
}

#define NUMTESTS 27
void (*tt[NUMTESTS])() = {Test1, Test2, Test3, Test4, Test5, Test6, Test7,
 Test8, Test9, Test10, Test11, Test12, Test13, Test14, Test15, Test16, Test17,
 Test18, Test19, Test20, Test21, Test22, Test23, Test24, Test25, Test26, Test27};

void RunTest()
{
	printf("Enter test number (from 1 to %i): ", NUMTESTS);
	char s[64];
	uint n = 0;
	n = atoi(fgets(s, 63, stdin));
	if((n > NUMTESTS) || !n) n = 7;
	tt[n-1]();
}

#endif