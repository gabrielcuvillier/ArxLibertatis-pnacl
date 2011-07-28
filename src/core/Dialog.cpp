/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// DanaeDlg.CPP
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		DANAE Dialog Box Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "core/Dialog.h"

#ifdef BUILD_EDITOR

#include <windows.h>
#include <commdlg.h>

#include <cstdio>

#include "ai/Paths.h"

#include "core/GameTime.h"
#include "core/Resource.h"
#include "core/Core.h"

#include "game/Player.h"

#include "gui/Interface.h"
#include "gui/Text.h"

#include "graphics/Math.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/Frame.h"
#include "graphics/data/Texture.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/Screenshot.h"
#include "io/IO.h"
#include "io/FilePath.h"
#include "io/Logger.h"
#include "io/Registry.h"

#include "physics/Clothes.h"
#include "physics/Physics.h"

#include "scene/GameSound.h"
#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/LoadLevel.h"

long FASTLOADS = 0;

extern long NOCHECKSUM;
extern long ZMAPMODE;
extern long TreatAllIO;
extern long HIDEMAGICDUST;
extern long LaunchDemo;
extern long USE_PLAYERCOLLISIONS;
extern long EXTERNALVIEWING;
extern long DYNAMIC_NORMALS;
extern long SHOWSHADOWS;
extern long ForceIODraw;
extern long NEED_ANCHORS;
long HIDEANCHORS = 1;
extern float TIMEFACTOR;
extern long ALLOW_MESH_TWEAKING;
extern long HIDESPEECH;
extern HWND PRECALC;
extern HANDLE LIGHTTHREAD;
extern long PROGRESS_COUNT;
extern long PROGRESS_TOTAL;
extern long PAUSED_PRECALC;

static COLORREF custcr[16];

 
INT_PTR CALLBACK SnapShotDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

long InitMemorySnaps();
void FlushMemorySnaps(long snap);

std::string SCRIPT_SEARCH_TEXT;

#define CHECK 1
#define UNCHECK 0

//*************************************************************************************
//*************************************************************************************

void SetCheck(HWND hWnd, int id, long chk)
{
	HWND thWnd;
	thWnd = GetDlgItem(hWnd, id);

	if (chk == 0) SendMessage(thWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	else if (chk == 1) SendMessage(thWnd, BM_SETCHECK, BST_CHECKED, 0);
}

//*************************************************************************************
//*************************************************************************************

bool IsChecked(HWND hWnd, int id)
{
	HWND thWnd;
	thWnd = GetDlgItem(hWnd, id);

	if (SendMessage(thWnd, BM_GETSTATE, 0, 0) == BST_CHECKED) return true;

	return false;
}

//*************************************************************************************
//*************************************************************************************

void SetClick(HWND hWnd, int id)
{
	HWND thWnd;
	thWnd = GetDlgItem(hWnd, id);
	SendMessage(thWnd, BM_CLICK, 0, 0);
}

float FORCED_REDUCTION_VALUE = 0.f;
INT_PTR CALLBACK MeshReductionProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;
	long t;

	switch (uMsg)
	{
		case WM_NOTIFY:
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_REDUCTION);
			t = SendMessage(thWnd, TBM_GETPOS, true, 0);
			FORCED_REDUCTION_VALUE = (float)t * ( 1.0f / 10000 );
			break;
		case WM_CLOSE:
			MESH_REDUCTION_WINDOW = NULL;
			EndDialog(hWnd, true);
			return false;
		case WM_INITDIALOG:
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_REDUCTION);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 10000));
			t = (long)(FORCED_REDUCTION_VALUE * 10000.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));
			return true;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					thWnd = GetDlgItem(hWnd, IDC_SLIDER_REDUCTION);
					t = SendMessage(thWnd, TBM_GETPOS, true, 0);
					FORCED_REDUCTION_VALUE = (float)t * ( 1.0f / 10000 );
					MESH_REDUCTION_WINDOW = NULL;
					EndDialog(hWnd, true);
					break;
			}

			break;
	}

	return false;
}

char ERRORSTRING[65535];
char ERRORTITLE[512];

static INT_PTR CALLBACK IDDErrorLogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;

	switch (uMsg)
	{
		case WM_CLOSE:
			EndDialog(hWnd, true);
			return false;
		case WM_INITDIALOG:
			thWnd = GetDlgItem(hWnd, IDC_ERRORLOG);
			SetWindowText(thWnd, ERRORSTRING);
			thWnd = GetDlgItem(hWnd, IDC_ERRORSTRING);
			SetWindowText(thWnd, ERRORTITLE);
			return true;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hWnd, true);
					break;
			}

			break;
	}

	return false;
}

HWND ShowErrorPopup( const char * title, const char * tex)
{
	strcpy(ERRORTITLE, title);
	strcpy(ERRORSTRING, tex);

	if (mainApp->m_pFramework->m_bIsFullscreen)
	{
		ARX_TIME_Pause();
		mainApp->Pause(true);
		DialogBox((HINSTANCE)GetWindowLongPtr(mainApp->m_hWnd, GWLP_HINSTANCE),
		          MAKEINTRESOURCE(IDD_SCRIPTERROR), mainApp->m_hWnd, IDDErrorLogProc);
		mainApp->Pause(false);
		ARX_TIME_UnPause();
		return NULL;
	}

	HWND hdl = CreateDialogParam((HINSTANCE)GetWindowLongPtr(mainApp->m_hWnd, GWLP_HINSTANCE),
	                             MAKEINTRESOURCE(IDD_SCRIPTERROR), mainApp->m_hWnd, IDDErrorLogProc, 0);
	return hdl;
}

INT_PTR CALLBACK PathwayOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;

	switch (uMsg)
	{
		case WM_CLOSE:
			CDP_PATHWAYS_Options = NULL;
			EndDialog(hWnd, true);
			return false;
		case WM_NOTIFY:
			char temp[256];
			float fval;
			long val;

			thWnd = GetDlgItem(hWnd, IDC_SLIDER1);
			val = (long)SendMessage(thWnd, TBM_GETPOS, true, 0);
			fval = (float)(val) * ( 1.0f / 10 );
			thWnd = GetDlgItem(hWnd, IDC_FCLIPTEXT);
			sprintf(temp, "%3.1f m", fval);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER2);
			val = (long)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_MAXVOLTEXT);
			sprintf(temp, "%ld%%", val);
			SetWindowText(thWnd, temp);

			if (ARX_PATHS_SelectedAP)
			{
				thWnd = GetDlgItem(hWnd, IDC_SHOWCOLOR);
				InvalidateRect(thWnd, NULL, true);

				if(HDC dc = GetDC(thWnd)) {
					RECT rect;
					GetClientRect(thWnd, &rect);
					HBRUSH brush = CreateSolidBrush(ARX_PATHS_SelectedAP->rgb.toRGB());
					SelectObject(dc, brush);
					FillRect(dc, &rect, brush);
					DeleteObject(brush);
					ValidateRect(thWnd, NULL);
					ReleaseDC(thWnd, dc);
				}
			}

			break;
		case WM_INITDIALOG:


			if (ARX_PATHS_HIERARCHYMOVE)
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
				SetWindowText(thWnd, "Hierarchy Edition");
			}
			else
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
				SetWindowText(thWnd, "WayPoint Edition");
			}

			if ((ARX_PATHS_SelectedAP == NULL) ||
			        (ARX_PATHS_SelectedNum == -1))
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
				ShowWindow(thWnd, SW_HIDE);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYBEZIER);
				ShowWindow(thWnd, SW_HIDE);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
				ShowWindow(thWnd, SW_SHOW);

			}
			else
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
				ShowWindow(thWnd, SW_SHOW);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYBEZIER);
				ShowWindow(thWnd, SW_SHOW);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
				ShowWindow(thWnd, SW_SHOW);
			}

			if ((ARX_PATHS_SelectedAP != NULL) &&
			        (ARX_PATHS_SelectedNum != -1))
			{
				thWnd = GetDlgItem(hWnd, IDC_SHOWCOLOR);
				InvalidateRect(thWnd, NULL, true);

				if(HDC dc = GetDC(thWnd)) {
					RECT rect;
					GetClientRect(thWnd, &rect);
					HBRUSH brush = CreateSolidBrush(ARX_PATHS_SelectedAP->rgb.toRGB());
					SelectObject(dc, brush);
					FillRect(dc, &rect, brush);
					DeleteObject(brush);
					ValidateRect(thWnd, NULL);
					ReleaseDC(thWnd, dc);
				}



				if (ARX_PATHS_SelectedAP->flags & PATH_LOOP)
				{
					thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
					SetWindowText(thWnd, "Loop");
				}
				else
				{
					thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
					SetWindowText(thWnd, "No Loop");
				}

				////////////////////////////////////////////////	NEW
				if (ARX_PATHS_SelectedAP->flags & PATH_AMBIANCE)
				{
					SetCheck(hWnd, IDC_AMBIANCE, CHECK);
					thWnd = GetDlgItem(hWnd, IDC_AMBIANCETEXT);
					SetWindowText(thWnd, ARX_PATHS_SelectedAP->ambiance);
				}
				else
				{
					SetCheck(hWnd, IDC_AMBIANCE, UNCHECK);
					thWnd = GetDlgItem(hWnd, IDC_AMBIANCETEXT);
					SetWindowText(thWnd, "NONE");
				}

				if (ARX_PATHS_SelectedAP->flags & PATH_RGB)
				{
					SetCheck(hWnd, IDC_FADECOLOR, CHECK);
				}
				else
				{
					SetCheck(hWnd, IDC_FADECOLOR, UNCHECK);
				}

				thWnd = GetDlgItem(hWnd, IDC_SLIDER1);
				SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(10, 400));

				if (ARX_PATHS_SelectedAP->flags & PATH_FARCLIP)
				{
					SetCheck(hWnd, IDC_CLIPPINGFAR, CHECK);
					long t = ARX_CLEAN_WARN_CAST_LONG((long)ARX_PATHS_SelectedAP->farclip * ( 1.0f / 10 ));
					SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));
				}
				else
				{
					SetCheck(hWnd, IDC_CLIPPINGFAR, UNCHECK);
					long t = (long)280;
					SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));
				}

				thWnd = GetDlgItem(hWnd, IDC_SLIDER2);	//Reverb
				SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1, 100)); // %

				if (ARX_PATHS_SelectedAP->flags & PATH_REVERB)
				{
					SetCheck(hWnd, IDC_REVERB, CHECK);
				}
				else
				{
					SetCheck(hWnd, IDC_REVERB, UNCHECK);
				}

				long t = ARX_PATHS_SelectedAP->amb_max_vol;

				if (t <= 1) t = 100;

				SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));
				thWnd = GetDlgItem(hWnd, IDC_MAXVOLTEXT);
				sprintf(temp, "%ld", t);
				SetWindowText(thWnd, temp);
				////////////////////////////////////////////////	END NEW

				if (ARX_PATHS_SelectedAP->pathways[ARX_PATHS_SelectedNum-1].flag == PATHWAY_BEZIER)
					SetCheck(hWnd, IDC_PATHWAYBEZIER, CHECK);
				else SetCheck(hWnd, IDC_PATHWAYBEZIER, UNCHECK);

				if (ARX_PATHS_SelectedAP->height != 0)
				{
					SetCheck(hWnd, IDC_ZONE, CHECK);
					thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
					char str[20];

					if (ARX_PATHS_SelectedAP->height < 0)
						SetWindowText(thWnd, "0");
					else
					{
						sprintf(str, "%ld", ARX_PATHS_SelectedAP->height);
						SetWindowText(thWnd, str);
					}
				}
				else
				{
					SetCheck(hWnd, IDC_ZONE, UNCHECK);
					thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
					SetWindowText(thWnd, "0");
				}

				thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
				char str[20];
				sprintf(str, "%.2f", ARX_PATHS_SelectedAP->pathways[ARX_PATHS_SelectedNum-1]._time);
				SetWindowText(thWnd, str);

				thWnd = GetDlgItem(hWnd, IDC_EDITNAME);
				char str2[64];
				sprintf(str2, "%s", ARX_PATHS_SelectedAP->name);
				SetWindowText(thWnd, str2);
			}
			else
			{
				SetCheck(hWnd, IDC_ZONE, UNCHECK);
				thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
				SetWindowText(thWnd, "0");
			}

			return true;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_COLORCHOOSE:

					if (ARX_PATHS_SelectedAP != NULL)
					{
						CHOOSECOLOR cc;
						cc.lStructSize = sizeof(CHOOSECOLOR);
						cc.hwndOwner = hWnd;
						cc.hInstance = 0; //Ignored
						cc.rgbResult = ARX_PATHS_SelectedAP->rgb.toRGB();
						cc.lpCustColors = custcr;
						cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
						ChooseColor(&cc);
						ARX_PATHS_SelectedAP->rgb = Color3f::fromRGB(cc.rgbResult);
						thWnd = GetDlgItem(hWnd, IDC_SHOWCOLOR);
						InvalidateRect(thWnd, NULL, true);

						if(HDC dc = GetDC(thWnd)) {
							RECT rect;
							GetClientRect(thWnd, &rect);
							HBRUSH brush = CreateSolidBrush(cc.rgbResult);
							SelectObject(dc, brush);
							FillRect(dc, &rect, brush);
							DeleteObject(brush);
							ValidateRect(thWnd, NULL);
							ReleaseDC(thWnd, dc);
						}
					}

					break;
				case IDC_PATHWAYLOOP:

					if ((ARX_PATHS_SelectedAP != NULL) &&
					        (ARX_PATHS_SelectedNum != -1))
					{
						if (ARX_PATHS_SelectedAP->flags & PATH_LOOP)
						{
							thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
							SetWindowText(thWnd, "No Loop");
							ARX_PATHS_SelectedAP->flags &= ~PATH_LOOP;
						}
						else
						{
							thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
							SetWindowText(thWnd, "Loop");
							ARX_PATHS_SelectedAP->flags |= PATH_LOOP;
						}
					}

					break;
				case IDC_PATHWAYHIERARCHY:

					if (ARX_PATHS_HIERARCHYMOVE)
					{
						ARX_PATHS_HIERARCHYMOVE = 0;
						thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
						SetWindowText(thWnd, "WayPoint Edition");
					}
					else
					{
						ARX_PATHS_HIERARCHYMOVE = ARX_PATH_HIERARCHY;
						thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
						SetWindowText(thWnd, "Hierarchy Edition");
					}

					break;
				case IDOK:
				case IDAPPLY:

					if ((ARX_PATHS_SelectedAP != NULL) &&
					        (ARX_PATHS_SelectedNum != -1))
					{
						if (IsChecked(hWnd, IDC_PATHWAYBEZIER))
							ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP, ARX_PATHS_SelectedNum, ARX_PATH_MOD_FLAGS, NULL, PATHWAY_BEZIER, 0);
						else ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP, ARX_PATHS_SelectedNum, ARX_PATH_MOD_FLAGS, NULL, PATHWAY_STANDARD, 0);

						HWND thWnd;
						thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
						char str[20];
						GetWindowText(thWnd, str, 20);
						ARX_PATHS_SelectedAP->pathways[ARX_PATHS_SelectedNum-1]._time = (float)atof(str);


						if (IsChecked(hWnd, IDC_ZONE))
						{
							thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
							char str[20];
							GetWindowText(thWnd, str, 20);
							float val = (float)atof(str);

							if (val <= 0)
							{
								ARX_PATHS_SelectedAP->height = -1;
							}
							else ARX_PATHS_SelectedAP->height = (long)val;
						}
						else
						{
							ARX_PATHS_SelectedAP->height = 0;
						}

						if (IsChecked(hWnd, IDC_AMBIANCE))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_AMBIANCE;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_AMBIANCE;
						}

						thWnd = GetDlgItem(hWnd, IDC_AMBIANCETEXT);
						GetWindowText(thWnd, ARX_PATHS_SelectedAP->ambiance, 127);

						if (IsChecked(hWnd, IDC_FADECOLOR))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_RGB;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_RGB;
						}

						if (IsChecked(hWnd, IDC_CLIPPINGFAR))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_FARCLIP;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_FARCLIP;
						}

						thWnd = GetDlgItem(hWnd, IDC_SLIDER1);
						val	=	SendMessage(thWnd, TBM_GETPOS, true, 0);
						fval =	(float)(val) * 10.f;
						ARX_PATHS_SelectedAP->farclip = fval;

						if (IsChecked(hWnd, IDC_REVERB))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_REVERB;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_REVERB;
						}

						thWnd = GetDlgItem(hWnd, IDC_SLIDER2);
						val	=	SendMessage(thWnd, TBM_GETPOS, true, 0);
						fval =	(float)(val);
						ARX_PATHS_SelectedAP->amb_max_vol = fval;
						thWnd = GetDlgItem(hWnd, IDC_EDITNAME);
						char str2[64];
						GetWindowText(thWnd, str2, 63);
						ARX_PATH * ap = ARX_PATHS_ExistName(str2);

						if ((ap != ARX_PATHS_SelectedAP) && (ap != NULL))
							LogError << ("This Name is already used by another path, New name ignored...");
						else ARX_PATHS_ChangeName(ARX_PATHS_SelectedAP, str2);

						if (LOWORD(wParam) == IDOK)
						{
							CDP_PATHWAYS_Options = NULL;
							EndDialog(hWnd, true);
						}
					}

					break;
				case IDCANCEL:
					CDP_PATHWAYS_Options = NULL;
					EndDialog(hWnd, true);
					break;
			}

			break;
	}

	return false;
}

//-----------------------------------------------------------------------------------

HWND InterObjDlg = NULL;
HWND dlgTreeViewhWnd = NULL;
WNDPROC lpfnOldWndProc;
struct TVINFO
{
	HTREEITEM hti;
	char text[260];
	INTERACTIVE_OBJ * io;
};

#define MAXTVV 5000
TVINFO * tvv[MAXTVV];
long TVVcount = 0;
Vec3f TVCONTROLEDplayerpos;
long TVCONTROLED = 0;
HTREEITEM hfix = NULL;
HTREEITEM hitem = NULL;
HTREEITEM hnpc = NULL;
HTREEITEM hroot = NULL;
HTREEITEM hcam = NULL;
HTREEITEM hmarker = NULL;
HTREEITEM hpath = NULL;

void InterTreeViewGotoPosition(HTREEITEM hitem)
{
	for (long i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				if (tvv[i]->io != NULL)
				{
					TVCONTROLEDplayerpos.x = tvv[i]->io->pos.x + (float)EEsin(radians(player.angle.b)) * 100.f;
					TVCONTROLEDplayerpos.y = tvv[i]->io->pos.y - 80.f;
					TVCONTROLEDplayerpos.z = tvv[i]->io->pos.z - (float)EEcos(radians(player.angle.b)) * 100.f;
					TVCONTROLED = 1;
				}
			}
		}
	}

	for (int i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				ARX_PATH * ap = ARX_PATH_GetAddressByName(tvv[i]->text);

				if (ap != NULL)
				{
					TVCONTROLEDplayerpos.x = ap->initpos.x + (float)EEsin(radians(player.angle.b)) * 100.f;
					TVCONTROLEDplayerpos.y = ap->initpos.y - 80.f;
					TVCONTROLEDplayerpos.z = ap->initpos.z - (float)EEcos(radians(player.angle.b)) * 100.f;
					TVCONTROLED = 1;
				}
			}
		}
	}
}
void InterTreeSelectObject(HTREEITEM hitem)
{
	for (long i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				if (!strcasecmp(tvv[i]->text, "player"))
				{
					if (inter.iobj[0])
					{
						if (inter.iobj[0]->EditorFlags & EFLAG_SELECTED)
						{
							UnSelectIO(inter.iobj[0]);
						}
						else
						{
							SelectIO(inter.iobj[0]);
						}

						return;
					}
				}

				if (tvv[i]->io != NULL)
				{
					if (tvv[i]->io->EditorFlags & EFLAG_SELECTED)
					{
						UnSelectIO(tvv[i]->io);
					}
					else
					{
						SelectIO(tvv[i]->io);
					}
				}
			}
		}
	}

	for (int i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				ARX_PATH * ap = ARX_PATH_GetAddressByName(tvv[i]->text);

				if (ap != NULL)
				{
					ARX_PATHS_SelectedAP = ap;
					ARX_PATHS_SelectedNum = 1;
				}
			}
		}
	}
}

INT_PTR CALLBACK InteractiveObjDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam) {
	
	(void)lParam;
	
	HTREEITEM hitem;

	switch (uMsg)
	{
		case WM_MOVE:

			if ((mainApp->m_pDeviceInfo->bWindowed) && (mainApp->m_hWnd != NULL))
			{
				RECT rect1, rect2;
				GetWindowRect(mainApp->m_hWnd, &rect1);
				GetWindowRect(hWnd, &rect2);
				long posx = rect2.left - rect1.left;
				long posy = rect2.top - rect1.top;
				Danae_Registry_WriteValue("WND_IO_DlgProc_POSX", posx);
				Danae_Registry_WriteValue("WND_IO_DlgProc_POSY", posy);
			}

			break;
		case WM_CLOSE:
			KillInterTreeView();
			return false;
		case WM_INITDIALOG:

			if ((mainApp->m_pDeviceInfo->bWindowed) && (mainApp->m_hWnd != NULL))
			{
				long posx, posy;
				Danae_Registry_ReadValue("WND_IO_DlgProc_POSX", &posx, 0);
				Danae_Registry_ReadValue("WND_IO_DlgProc_POSY", &posy, 0);

				if ((posx != -1) && (posy != -1)
				        && (posx < 1000) && (posx > 0)
				        && (posy < 800) && (posy > 0)
				   )
				{
					RECT rect1;
					GetWindowRect(mainApp->m_hWnd, &rect1);
					posx = rect1.left + posx;
					posy = rect1.left + posy;

					if (posx < 0) posx = 0;

					if (posy < 0) posy = 0;

					SetWindowPos(hWnd, NULL, posx, posy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			return true;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_GOTOOBJECT:
					hitem = TreeView_GetSelection(dlgTreeViewhWnd);
					InterTreeViewGotoPosition(hitem);
					break;
				case IDC_SELECTOBJECT:
					hitem = TreeView_GetSelection(dlgTreeViewhWnd);
					InterTreeSelectObject(hitem);
					break;
			}

			break;
	}

	return false;
}
void InterTreeViewDisplayInfo(HTREEITEM hitem)
{
	char texx[512];

	for (long i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				if (!strcasecmp(tvv[i]->text, "player"))
				{
					sprintf(texx, "%s", tvv[i]->text);
					SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
					return;
				}

				if (tvv[i]->io == NULL)
				{
					sprintf(texx, "%s", tvv[i]->text);
					SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
					return;
				}

				char typee[16];
				strcpy(typee, "X");

				if (tvv[i]->io->ioflags & IO_NPC) strcpy(typee, "NPC IO");

				if (tvv[i]->io->ioflags & IO_ITEM) strcpy(typee, "ITEM IO");

				if (tvv[i]->io->ioflags & IO_FIX) strcpy(typee, "FIX IO");

				if (tvv[i]->io->ioflags & IO_CAMERA) strcpy(typee, "Camera");

				if (tvv[i]->io->ioflags & IO_MARKER) strcpy(typee, "Marker");

				sprintf(texx, "Ident: %s\n%s NumIO %ld\nPos X:%ld Y:%ld Z:%ld\nShow %d Lvl %d (%d)"
				        , tvv[i]->text, typee, GetInterNum(tvv[i]->io), (long)tvv[i]->io->pos.x, (long)tvv[i]->io->pos.y, (long)tvv[i]->io->pos.z,
				        tvv[i]->io->show, tvv[i]->io->level, tvv[i]->io->truelevel
				       );
				SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
				return;
			}
		}
	}

	strcpy(texx, "No Interactive Object Selected");
	SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
}
LONG CALLBACK InterTreeViewSubClassFunc(HWND hWnd,
                                        UINT uMsg, WORD wParam, LONG lParam)
{
	switch (uMsg)
	{

		case WM_LBUTTONUP:
			HTREEITEM hitem;
			hitem = TreeView_GetSelection(dlgTreeViewhWnd);
			InterTreeViewDisplayInfo(hitem);
			break;
	}

	return CallWindowProc((WNDPROC)lpfnOldWndProc, hWnd, uMsg, wParam,
	                      lParam);
}


void RemoveIOTVItem(HWND tvhwnd, INTERACTIVE_OBJ * io, const char * name) {
	
	if (TVVcount != 0) {
		if (io != NULL)
		{
			for (long i = 0; i < TVVcount; i++)
			{
				if (tvv[i] != NULL)
				{
					if (tvv[i]->io == io)
					{
						(void)TreeView_DeleteItem(tvhwnd, tvv[i]->hti);
						free(tvv[i]);
						tvv[i] = NULL;
						TVVcount--;

						while (i < TVVcount)
						{
							tvv[i] = tvv[i+1];
							i++;
						}

						return;
					}
				}
			}
		}
		else
		{
			// path removal
			for (long i = 0; i < TVVcount; i++)
			{
				if (tvv[i] != NULL)
				{
					if (!strcasecmp(name, tvv[i]->text))
					{
						(void)TreeView_DeleteItem(tvhwnd, tvv[i]->hti);
						free(tvv[i]);

						while (i < TVVcount - 1)
						{
							tvv[i] = tvv[i+1];
							i++;
						}

						TVVcount--;
						return;
					}
				}
			}

		}
	}
}

void AddIOTVItem(HWND tvhwnd, INTERACTIVE_OBJ * io, const char * name, long type)
{
	TVINSERTSTRUCT tis;
	HTREEITEM parent = NULL;
	std::string temp;

	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	memset(tvv[TVVcount], 0, sizeof(TVINFO));

	if (type == IOTVTYPE_PLAYER)
		temp = "PLAYER";
	else if (io != NULL)
		temp = io->long_name();
	else
		temp = name;

	strcpy(tvv[TVVcount]->text, temp.c_str());
	tvv[TVVcount]->io = io;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;

	if (type == IOTVTYPE_PLAYER) tis.hParent = hroot;
	else if (type == IOTVTYPE_PATH) tis.hParent = hpath;
	else if (io)
	{
		if (io->ioflags & IO_NPC) tis.hParent = hnpc;

		if (io->ioflags & IO_FIX) tis.hParent = hfix;

		if (io->ioflags & IO_ITEM) tis.hParent = hitem;

		if (io->ioflags & IO_CAMERA) tis.hParent = hcam;

		if (io->ioflags & IO_MARKER) tis.hParent = hmarker;
	}

	tis.hInsertAfter = TVI_SORT;
	parent = TreeView_InsertItem(tvhwnd, &tis);
	tvv[TVVcount]->hti = parent;
	TVVcount++;
	InterTreeViewDisplayInfo(parent);
}

void FillInterTreeView(HWND tvhwnd)
{
	long i;
	TVINSERTSTRUCT tis;

 
	HTREEITEM hti = NULL;

	(void)TreeView_DeleteAllItems(tvhwnd);
	(void)TreeView_SetBkColor(tvhwnd, 0x00000000);
	(void)TreeView_SetTextColor(tvhwnd, 0x00FFFFFF);

	if (TVVcount != 0)
		for (i = 0; i < TVVcount; i++)
		{
			if (tvv[i] != NULL)
			{
				free(tvv[i]);
				tvv[i] = NULL;
			}
		}

	TVVcount = 0;
	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Root");
	tvv[TVVcount]->io = NULL;

	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = NULL;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hroot = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Camera");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hcam = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Marker");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hmarker = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "FIX");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hfix = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "ITEMS");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hitem = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "NPC");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hnpc = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Path");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hpath = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	InterTreeViewDisplayInfo(hroot);

	AddIOTVItem(tvhwnd, inter.iobj[0], NULL, IOTVTYPE_PLAYER);

	for (i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			AddIOTVItem(tvhwnd, inter.iobj[i], NULL, 0);

		}
	}

	(void)TreeView_Expand(tvhwnd, hroot, TVE_EXPAND);
}
 
 
 
 
void InterTreeViewItemRemove(INTERACTIVE_OBJ * io, const char * name)
{
	if (InterObjDlg) RemoveIOTVItem(dlgTreeViewhWnd, io, name);
}
void InterTreeViewItemAdd(INTERACTIVE_OBJ * io, const char * name, long type)
{
	if (InterObjDlg) AddIOTVItem(dlgTreeViewhWnd, io, name, type);
}

void LaunchInteractiveObjectsApp(HWND hwnd)
{
	if (!mainApp->m_pDeviceInfo->bWindowed)
		return;

	if (InterObjDlg) return;

	InterObjDlg = CreateDialogParam(
	                  (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	                  MAKEINTRESOURCE(IDD_INTERDLG),
	                  hwnd,
	                  InteractiveObjDlgProc, 0);

	dlgTreeViewhWnd = GetDlgItem(InterObjDlg, IDC_INTERTREEVIEW);
	lpfnOldWndProc = (WNDPROC)SetWindowLongPtr(dlgTreeViewhWnd,
	                                        GWLP_WNDPROC, (LONG_PTR)InterTreeViewSubClassFunc);
	FillInterTreeView(dlgTreeViewhWnd);
	ShowWindow(InterObjDlg, SW_SHOW);
}
void KillInterTreeView()
{
	if (InterObjDlg)
	{
		if (TVVcount != 0)
			for (long i = 0; i < TVVcount; i++)
			{
				if (tvv[i] != NULL)
				{
					free(tvv[i]);
					tvv[i] = NULL;
				}
			}

		EndDialog(InterObjDlg, true);
		InterObjDlg = NULL;
	}
}

extern long FINAL_COMMERCIAL_DEMO;

//*************************************************************************************

long THREAD_MINX = 0;
long THREAD_MINZ = 0;
long THREAD_MAXX = 0;
long THREAD_MAXZ = 0;

long PAUSED_PRECALC = 0;
HWND PRECALC = NULL;
long LIGHT_THREAD_STATUS = 0; // 0=not created EXITED_LIGHT_THREAD=0;
// 1=working
// 2=finished exited
// 3=immediate exit !
LPTHREAD_START_ROUTINE EERIE_LIGHT_LightProc(char * ts) {
	
	(void)ts;
	
	LIGHT_THREAD_STATUS = 1;
	EERIEPrecalcLights(THREAD_MINX, THREAD_MINZ, THREAD_MAXX, THREAD_MAXZ);
	LIGHT_THREAD_STATUS = 2;

	ExitThread(1);
	return 0;
}

//*************************************************************************************
// "Clean" Kill for light thread
//*************************************************************************************
void KillLightThread()
{
	if (LIGHT_THREAD_STATUS == 2)
	{
		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}

	if ((LIGHT_THREAD_STATUS == 1) || (LIGHT_THREAD_STATUS == 3))
	{
		LIGHT_THREAD_STATUS = 3;

		while (LIGHT_THREAD_STATUS != 2)
			Sleep(10);

		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}
}

//*************************************************************************************
//*************************************************************************************

void LaunchLightThread(long minx, long minz, long maxx, long maxz)
{
	char args;
	DWORD id;

	if (LIGHT_THREAD_STATUS == 2)
	{
		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}

	if ((LIGHT_THREAD_STATUS == 1) || (LIGHT_THREAD_STATUS == 3))
	{
		LIGHT_THREAD_STATUS = 3;

		while (LIGHT_THREAD_STATUS != 2)
			Sleep(10);

		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}

	if (PRECALC == NULL)
	{
		if (mainApp->m_pFramework->m_bIsFullscreen)
		{

			ARX_TIME_Pause();
			DialogBox((HINSTANCE)GetWindowLongPtr(mainApp->m_hWnd, GWLP_HINSTANCE),
			          MAKEINTRESOURCE(IDD_PRECALC), mainApp->m_hWnd, PrecalcProc);
			ARX_TIME_UnPause();
		}

		else
			PRECALC = (CreateDialogParam((HINSTANCE)GetWindowLongPtr(mainApp->m_hWnd, GWLP_HINSTANCE),
			                             MAKEINTRESOURCE(IDD_PRECALC), mainApp->m_hWnd, PrecalcProc, 0));
	}

	THREAD_MINX = minx;
	THREAD_MINZ = minz;
	THREAD_MAXX = maxx;
	THREAD_MAXZ = maxz;

	LIGHTTHREAD = (HANDLE)CreateThread(
	                  NULL,												//pointer to security attributes
	                  0,												// initial thread stack size
	                  (LPTHREAD_START_ROUTINE) EERIE_LIGHT_LightProc,	// pointer to thread function
	                  (LPVOID)&args,									// argument for new thread
	                  0,                     // creation flags
	                  (LPDWORD)&id										// pointer to receive thread ID
	              );

}

void RecalcLightZone(float x, float z, long siz) {
	
	long i, j, x0, x1, z0, z1;
	
	i = x * ACTIVEBKG->Xmul;
	j = z * ACTIVEBKG->Zmul;
	
	x0 = i - siz;
	x1 = i + siz;
	z0 = j - siz;
	z1 = j + siz;
	
	if (x0 < 2) x0 = 2;
	else if (x0 >= ACTIVEBKG->Xsize - 2) x0 = ACTIVEBKG->Xsize - 3;
	
	if (x1 < 2) x1 = 0;
	else if (x1 >= ACTIVEBKG->Xsize - 2) x1 = ACTIVEBKG->Xsize - 3;

	if (z0 < 2) z0 = 0;
	else if (z0 >= ACTIVEBKG->Zsize - 2) z0 = ACTIVEBKG->Zsize - 3;
	
	if (z1 < 2) z1 = 0;
	else if (z1 >= ACTIVEBKG->Zsize - 2) z1 = ACTIVEBKG->Zsize - 3;
	
	LaunchLightThread(x0, z0, x1, z1);
}

long SYNTAXCHECKING = 0;

INT_PTR CALLBACK PrecalcProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;

	switch (uMsg)
	{
		case WM_MOVE:

			if (mainApp->m_pDeviceInfo->bWindowed)
			{
				RECT rect1, rect2;
				GetWindowRect(mainApp->m_hWnd, &rect1);
				GetWindowRect(hWnd, &rect2);
				long posx = rect2.left - rect1.left;
				long posy = rect2.top - rect1.top;
				Danae_Registry_WriteValue("WND_LightPrecalc_POSX", posx);
				Danae_Registry_WriteValue("WND_LightPrecalc_POSY", posy);
			}

			break;
		case WM_TIMER:
			thWnd = GetDlgItem(hWnd, IDC_PROGRESS);
			float t;
			t = (float)PROGRESS_COUNT / (float)PROGRESS_TOTAL * 1000.f;
			SendMessage(thWnd, PBM_SETRANGE , 0, MAKELPARAM(0, 1000));
			SendMessage(thWnd, PBM_SETPOS , (long)t, 0);

			thWnd = GetDlgItem(hWnd, IDC_STATICC);

			if (PAUSED_PRECALC) SetWindowText(thWnd, "Paused");
			else if (LIGHTTHREAD != NULL)
			{
				char tex[32];
				t *= ( 1.0f / 10 );
				sprintf(tex, "Working... ( %ld%% )", (long)t);
				SetWindowText(thWnd, tex);
			}
			else
			{
				SetWindowText(thWnd, "Idle...");
				PROGRESS_COUNT = PROGRESS_TOTAL;
				SendMessage(thWnd, PBM_SETPOS , 1000, 0);

				if (mainApp->m_pFramework->m_bIsFullscreen)
				{
					PRECALC = NULL;
					KillTimer(hWnd, 1);
					EndDialog(hWnd, true);
					return false;
				}
			}

			break;
		case WM_COMMAND:

			if (ID_PAUSE == LOWORD(wParam))
			{
				if (PAUSED_PRECALC)
				{
					thWnd = GetDlgItem(hWnd, ID_PAUSE);
					SetWindowText(thWnd, "Pause");
					PAUSED_PRECALC = 0;
				}
				else
				{
					thWnd = GetDlgItem(hWnd, ID_PAUSE);
					SetWindowText(thWnd, "Resume");
					PAUSED_PRECALC = 1;
				}
			}

			if (ID_STOP == LOWORD(wParam))
			{
				if (LIGHTTHREAD != NULL)
				{
					TerminateThread(LIGHTTHREAD, 1);
					LIGHTTHREAD = NULL;
				}
			}

			if (ID_RECALCULATE == LOWORD(wParam))
			{
				LaunchLightThread(0, 0, 999999, 9999999);
			}

			if (ID_AROUND == LOWORD(wParam))
			{
				RecalcLightZone(player.pos.x, player.pos.z, 2);
			}

			break;
		case WM_CLOSE:
			PRECALC = NULL;
			KillTimer(hWnd, 1);
			EndDialog(hWnd, true);
			return false;
			break;
		case WM_INITDIALOG:

			if (mainApp->m_pDeviceInfo->bWindowed)
			{
				long posx, posy;
				Danae_Registry_ReadValue("WND_LightPrecalc_POSX", &posx, 0);
				Danae_Registry_ReadValue("WND_LightPrecalc_POSY", &posy, 0);

				if ((posx != -1) && (posy != -1)
				        && (posx < 1000) && (posx > 0)
				        && (posy < 1000) && (posy > 0)
				   )
				{
					RECT rect1;
					GetWindowRect(mainApp->m_hWnd, &rect1);
					posx = rect1.left + posx;
					posy = rect1.left + posy;

					if (posx < 0) posx = 0;

					if (posy < 0) posy = 0;

					SetWindowPos(hWnd, NULL, posx, posy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			PRECALC = hWnd;
			SetTimer(hWnd, 1, 100, NULL);
			thWnd = GetDlgItem(hWnd, IDC_PROGRESS);
			SendMessage(thWnd, PBM_SETRANGE , 0, MAKELPARAM(0, 1000));
			SendMessage(thWnd, PBM_SETPOS , 0, 0);
			SendMessage(thWnd, PBM_SETBKCOLOR , 0, 0);
			SendMessage(thWnd, PBM_SETBARCOLOR , 0, 0xFF0000FF);
			thWnd = GetDlgItem(hWnd, IDC_STATICC);
			SetWindowText(thWnd, "Idle");
			return true;
			break;
	}

	return false;
}

//*************************************************************************************
//*************************************************************************************

static INT_PTR CALLBACK GaiaTextEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM)
{
	HWND thWnd;

	if (WM_COMMAND == uMsg)
	{
		if (IDOK == LOWORD(wParam))
		{
			thWnd = GetDlgItem(hWnd, IDC_TEXTEDIT);
			GetWindowText(thWnd, GTE_TEXT, GTE_SIZE - 1);
			EndDialog(hWnd, true);
		}

		if (IDCANCEL == LOWORD(wParam))
			EndDialog(hWnd, true);
	}

	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hWnd, GTE_TITLE);
		thWnd = GetDlgItem(hWnd, IDC_TEXTEDIT);
		SetWindowText(thWnd, GTE_TEXT);
		return true;
	}

	return false;
}

void ExitProc();

INT_PTR CALLBACK StartProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	long val;
	HWND thWnd;

	switch (uMsg)
	{
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_LAUNCHDEMO:
					LaunchDemo = 1;

				case IDOK:

					if (IsChecked(hWnd, IDC_FASTLOADING)) FASTLOADS = 1;
					else FASTLOADS = 0;

					if (IsChecked(hWnd, IDC_NODIRCREATION)) NODIRCREATION = 1;
					else NODIRCREATION = 0;

					if (IsChecked(hWnd, IDC_SOUND))
					{
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), false);
						SetCheck(hWnd, IDC_REVERB, UNCHECK);
					}
					else
					{
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), true);
					}

					Project.bits = 32;

					if (IsChecked(hWnd, IDC_SYNTAXCHECKING)) SYNTAXCHECKING = 1;
					else SYNTAXCHECKING = 0;

					if (IsChecked(hWnd, IDC_NOCHECKSUM)) NOCHECKSUM = 1;
					else NOCHECKSUM = 0;

					if (IsChecked(hWnd, IDC_LOADDEMO))	Project.demo = LEVELDEMO;

					if (IsChecked(hWnd, IDC_LOADDEMO2))	Project.demo = LEVELDEMO2;

					if (IsChecked(hWnd, IDC_LOADDEMO3))	Project.demo = LEVELDEMO3;

					if (IsChecked(hWnd, IDC_LOADDEMO4))	Project.demo = LEVELDEMO4;

					if (IsChecked(hWnd, IDC_TLEVEL0))	Project.demo = LEVEL0;

					if (IsChecked(hWnd, IDC_TLEVEL1))	Project.demo = LEVEL1;

					if (IsChecked(hWnd, IDC_TLEVEL2))	Project.demo = LEVEL2;

					if (IsChecked(hWnd, IDC_TLEVEL3))	Project.demo = LEVEL3;

					if (IsChecked(hWnd, IDC_TLEVEL4))	Project.demo = LEVEL4;

					if (IsChecked(hWnd, IDC_TLEVEL5))	Project.demo = LEVEL5;

					if (IsChecked(hWnd, IDC_TLEVEL6))	Project.demo = LEVEL6;

					if (IsChecked(hWnd, IDC_TLEVEL7))	Project.demo = LEVEL7;

					if (IsChecked(hWnd, IDC_TLEVEL8))	Project.demo = LEVEL8;

					if (IsChecked(hWnd, IDC_TLEVEL9))	Project.demo = LEVEL9;

					if (IsChecked(hWnd, IDC_TLEVEL10))	Project.demo = LEVEL10;

					if (IsChecked(hWnd, IDC_TLEVEL11))	Project.demo = LEVEL11;

					if (IsChecked(hWnd, IDC_TLEVEL12))	Project.demo = LEVEL12;

					if (IsChecked(hWnd, IDC_TLEVEL13))	Project.demo = LEVEL13;

					if (IsChecked(hWnd, IDC_TLEVEL14))	Project.demo = LEVEL14;

					if (IsChecked(hWnd, IDC_TLEVEL15))	Project.demo = LEVEL15;

					if (IsChecked(hWnd, IDC_TLEVEL16))	Project.demo = LEVEL16;

					if (IsChecked(hWnd, IDC_TLEVEL17))	Project.demo = LEVEL17;

					if (IsChecked(hWnd, IDC_TLEVEL18))	Project.demo = LEVEL18;

					if (IsChecked(hWnd, IDC_TLEVEL19))	Project.demo = LEVEL19;

					if (IsChecked(hWnd, IDC_TLEVEL20))	Project.demo = LEVEL20;

					if (IsChecked(hWnd, IDC_TLEVEL21))	Project.demo = LEVEL21;

					if (IsChecked(hWnd, IDC_TLEVEL22))	Project.demo = LEVEL22;

					if (IsChecked(hWnd, IDC_TLEVEL23))	Project.demo = LEVEL23;

					if (IsChecked(hWnd, IDC_TLEVEL24))	Project.demo = LEVEL24;

					if (IsChecked(hWnd, IDC_NEED_ANCHOR)) NEED_ANCHORS = 1;
					else NEED_ANCHORS = 0;

					if (IsChecked(hWnd, IDC_COMPATIBILITY)) Project.compatibility = 1;
					else Project.compatibility = 0;

					//if (IsChecked(hWnd, IDC_OTHERSERVER))
					//{
					//	thWnd = GetDlgItem(hWnd, IDC_OTHERSERVER);
					//	GetWindowText(thWnd, Project_workingdir, 256);
					//}
					//else strcpy(Project_workingdir, "\\\\ARKANESERVER\\Public\\Arx\\");

					//chdir("GRAPH\\LEVELS\\");

					thWnd = GetDlgItem(hWnd, IDC_TEXTUREPRECISION);
					val = SendMessage(thWnd, TBM_GETPOS, true, 0);

					switch (val)
					{
						case 1:
							Project.TextureSize = 32;
							break;
						case 2:
							Project.TextureSize = 64;
							break;
						case 3:
							Project.TextureSize = 96;
							break;
						case 4:
							Project.TextureSize = 128;
							break;
						case 5:
							Project.TextureSize = 192;
							break;
						case 6:
							Project.TextureSize = 256;
							break;
						case 7:
							Project.TextureSize = 2;
							break;
						default:
							Project.TextureSize = 0;
					}

					if (IsChecked(hWnd, IDC_TEX16)) Project.TextureBits = 16;
					else Project.TextureBits = 32;

					//Danae_Registry_Write("LastWorkingDir", Project_workingdir);
					EndDialog(hWnd, true);
					break;
				case IDQUIT:
					EndDialog(hWnd, true);
					ExitProc();
					break;
				case IDC_CHOOSEDIR:
					thWnd = GetDlgItem(hWnd, IDC_OTHERSERVER);
					SendMessage(thWnd, BM_CLICK, 0, 0);
					//HERMESFolderSelector(Project_workingdir, "Choose Working Folder");

					//SetWindowText(thWnd, Project_workingdir);
					//Danae_Registry_Write("LastWorkingDir", Project_workingdir);
					break;
			}

			break;
		case WM_NOTIFY:
			long val;
			char temp[64];
			thWnd = GetDlgItem(hWnd, IDC_TEXTUREPRECISION);
			val = SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATICC);

			switch (val)
			{
				case 1:
					sprintf(temp, "Texture Size: 32x32");
					break;
				case 2:
					sprintf(temp, "Texture Size: 64x64");
					break;
				case 3:
					sprintf(temp, "Texture Size: 96x96");
					break;
				case 4:
					sprintf(temp, "Texture Size: 128x128");
					break;
				case 5:
					sprintf(temp, "Texture Size: 192x192");
					break;
				case 6:
					sprintf(temp, "Texture Size: 256x256");
					break;
				case 7:
					sprintf(temp, "Texture Size: ( 1.0f / 2 )");
					break;
				default:
					sprintf(temp, "Texture Size: ANY");
			}

			SetWindowText(thWnd, temp);
			break;
		case WM_INITDIALOG:
			HWND thWnd;
			char tex[128];

			thWnd = GetDlgItem(hWnd, IDC_NODIRCREATION);

			if (NODIRCREATION) SendMessage(thWnd, BM_CLICK, 0, 0);

			thWnd = GetDlgItem(hWnd, IDC_VERSION);
			sprintf(tex, "Ver.%2.3f", DANAE_VERSION);
			SetWindowText(thWnd, tex);

			SetClick(hWnd, IDC_OTHERSERVER);

			SetClick(hWnd, IDC_LOADDEMO);

			if (NOCHECKSUM) SetClick(hWnd, IDC_NOCHECKSUM);

			if (SYNTAXCHECKING)	SetClick(hWnd, IDC_SYNTAXCHECKING);

			SetClick(hWnd, IDC_TEX16);
			thWnd = GetDlgItem(hWnd, IDC_TEXTUREPRECISION);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1, 8));
			long t = 8;
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			if (NEED_ANCHORS) SetClick(hWnd, IDC_NEED_ANCHOR);

			return true;
			break;
	}

	return false;
}
INT_PTR CALLBACK ScriptSearchProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM)
{
    if (WM_COMMAND == uMsg)
    {
        switch (LOWORD(wParam))
        {
            case IDOK:
                HWND thWnd;
                thWnd = GetDlgItem(hWnd, IDC_SEARCHEDIT);
                char temp[256];
                GetWindowText(thWnd, temp, 255);
                SCRIPT_SEARCH_TEXT = temp;
                EndDialog(hWnd, true);
                break;
            case IDCANCEL:
                EndDialog(hWnd, true);
                break;
        }
    }

    if (uMsg == WM_INITDIALOG)
    {
        SCRIPT_SEARCH_TEXT[0] = 0;
        HWND thWnd;
        thWnd = GetDlgItem(hWnd, IDC_SEARCHEDIT);
        SetFocus(thWnd); 
        return true;
    }

    return false;
}

//*************************************************************************************
// AboutProc()
//  message proc function for the about box
//*************************************************************************************
INT_PTR CALLBACK AboutProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM)
{
	if (WM_COMMAND == uMsg)
		if (IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam))
			EndDialog(hWnd, true);

	if (uMsg == WM_INITDIALOG)
	{
		HWND thWnd;
		char tex[128];

		thWnd = GetDlgItem(hWnd, IDC_ABOUT_VERSION);
		sprintf(tex, "Ver.%2.3f", DANAE_VERSION);
		SetWindowText(thWnd, tex);
		return true;
	}

	return false;
}
extern long USEINTERNORM;
long oml;
extern long TRUEFIGHT;

INT_PTR CALLBACK OptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;
	static long wuz;

	switch (uMsg)
	{
		case WM_INITDIALOG :
		{
			ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);

			oml = ModeLight;

			if (SYNTAXCHECKING)				SetClick(hWnd, IDC_SYNTAXCHECK);

			if (ZMAPMODE)					SetClick(hWnd, IDC_ZMAPMODE);

			if (Project.vsync)				SetClick(hWnd, IDC_VSYNC);

			if (SHOWSHADOWS)				SetClick(hWnd, IDC_SHOWSHADOWS);

			if (INVERTMOUSE)				SetClick(hWnd, IDC_INVERTMOUSE);

			if (Project.bits == 16)			SetClick(hWnd, IDC_FULLRENDER16BITS);

			if (Project.bits == 32)			SetClick(hWnd, IDC_FULLRENDER32BITS);

			if (DEBUGNPCMOVE)				SetClick(hWnd, IDC_DEBUGNPCMOVE);

			if (DYNAMIC_NORMALS)			SetClick(hWnd, IDC_DYNAMICNORMALS);

			if (ViewMode & VIEWMODE_WIRE)	SetClick(hWnd, IDC_WIREFRAME);

			if (TRUEFIGHT)					SetClick(hWnd, IDC_TRUEFIGHT);

			if (ModeLight & MODE_STATICLIGHT)
			{
				SetClick(hWnd, IDC_SHOWLIGHTSNSHADOWS);
				wuz = 1;
			}
			else wuz = 0;


			if (ModeLight & MODE_NORMALS)	SetClick(hWnd, IDC_ILLUMNORMAL);

			if (ModeLight & MODE_RAYLAUNCH)	SetClick(hWnd, IDC_ILLUMRAYLAUNCH);

			if (ModeLight & MODE_SMOOTH)	SetClick(hWnd, IDC_ILLUMSMOOTH);

			if (ModeLight & MODE_DYNAMICLIGHT)	SetClick(hWnd, IDC_TORCHHALO);

			if (USE_COLLISIONS)				SetClick(hWnd, IDC_COLLISIONS);

			if (USE_PLAYERCOLLISIONS)		SetClick(hWnd, IDC_PLAYERCOLLISIONS);

			if (SHOW_TORCH)					SetClick(hWnd, IDC_TORCHHALO2);

			if (ViewMode & VIEWMODE_NORMALS)	SetClick(hWnd, IDC_SHOWNORMALS);

			if (ModeLight & MODE_DEPTHCUEING)	SetClick(hWnd, IDC_SHOWDEPTH);

			if (ViewMode & VIEWMODE_INFOTEXT)	SetClick(hWnd, IDC_INFOTEXT);

			if (ViewMode & VIEWMODE_FLAT)		SetClick(hWnd, IDC_NOTEXTURES);

			if (USEINTERNORM)				SetClick(hWnd, IDC_INTERNORM);

			if (EXTERNALVIEWING)			SetClick(hWnd, IDC_THIRDPERSON);

			if (ALLOW_MESH_TWEAKING) SetClick(hWnd, IDC_MESHTWEAK);

			thWnd = GetDlgItem(hWnd, IDC_SLIDERDEPTH);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1000, 8000));
			long t = (long)subj.cdepth;
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_TIMESLIDER);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 200));
			t = (long)(TIMEFACTOR * 100.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			if (!ARX_SOUND_IsEnabled())
			{
				SetCheck(hWnd, IDC_DISABLESOUND, CHECK);
				EnableWindow(GetDlgItem(hWnd, IDC_REVERB), false);
			}

			return true;
		}

		case WM_DESTROY :
			ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
			break;

		case WM_COMMAND :
		{
			switch (LOWORD(wParam))
			{
				case IDC_TIMEFACTOR :
				{
					TIMEFACTOR = 1.f;
					long t = (long)(TIMEFACTOR * 100.f);
					thWnd = GetDlgItem(hWnd, IDC_TIMESLIDER);
					SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));
					break;
				}

				case IDC_BKGCOLOR :
				{
					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hWnd;
					cc.hInstance = 0; //Ignored
					cc.rgbResult = subj.bkgcolor.toRGB(0);
					cc.lpCustColors = custcr;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					ChooseColor(&cc);
					subj.bkgcolor = Color::fromRGB(cc.rgbResult, 0);
					break;
				}

				case IDC_DISABLESOUND :

					if (IsChecked(hWnd, IDC_DISABLESOUND))
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), false);
					else
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), true);

					break;

				case IDC_DEPTHDEFAULT :
					SendMessage(GetDlgItem(hWnd, IDC_SLIDERDEPTH), TBM_SETPOS, true, (LPARAM)(2800));
					break;

				case IDOK :
				{
					if (IsChecked(hWnd, IDC_DISABLESOUND)) ARX_SOUND_Release();
					else
					{
						if (!ARX_SOUND_IsEnabled()) ARX_SOUND_Init();
					}

					if (IsChecked(hWnd, IDC_TRUEFIGHT)) TRUEFIGHT = 1;
					else TRUEFIGHT = 0;

					if (IsChecked(hWnd, IDC_SYNTAXCHECK)) SYNTAXCHECKING = 1;
					else SYNTAXCHECKING = 0;

					if (IsChecked(hWnd, IDC_VSYNC)) Project.vsync = 1;
					else Project.vsync = 0;

					if (IsChecked(hWnd, IDC_ZMAPMODE)) ZMAPMODE = 1;
					else ZMAPMODE = 0;

					if (IsChecked(hWnd, IDC_SHOWSHADOWS)) SHOWSHADOWS = 1;
					else SHOWSHADOWS = 0;

					if (IsChecked(hWnd, IDC_FULLRENDER16BITS)) Project.bits = 16;
					else Project.bits = 32;

					mainApp->m_pFramework->bitdepth = Project.bits;

					if (IsChecked(hWnd, IDC_DEBUGNPCMOVE)) DEBUGNPCMOVE = 1;
					else DEBUGNPCMOVE = 0;

					if (IsChecked(hWnd, IDC_DYNAMICNORMALS)) DYNAMIC_NORMALS = 1;
					else DYNAMIC_NORMALS = 0;

					if (IsChecked(hWnd, IDC_WIREFRAME)) ViewMode |= VIEWMODE_WIRE;
					else ViewMode &= ~VIEWMODE_WIRE;

					if (IsChecked(hWnd, IDC_ILLUMNORMAL))
					{
						if (!(oml & MODE_NORMALS)) wuz = 0;

						ModeLight |= MODE_NORMALS;
					}
					else
					{
						if ((oml & MODE_NORMALS)) wuz = 0;

						ModeLight &= ~MODE_NORMALS;
					}

					if (IsChecked(hWnd, IDC_ILLUMRAYLAUNCH))
					{
						if (!(oml & MODE_RAYLAUNCH)) wuz = 0;

						ModeLight |= MODE_RAYLAUNCH;
					}
					else
					{
						if ((oml & MODE_RAYLAUNCH)) wuz = 0;

						ModeLight &= ~MODE_RAYLAUNCH;
					}

					if (IsChecked(hWnd, IDC_ILLUMSMOOTH))
					{
						if (!(oml & MODE_SMOOTH)) wuz = 0;

						ModeLight |= MODE_SMOOTH;
					}
					else
					{
						if ((oml & MODE_SMOOTH)) wuz = 0;

						ModeLight &= ~MODE_SMOOTH;
					}



					if (IsChecked(hWnd, IDC_SHOWLIGHTSNSHADOWS) && !wuz)
					{
						ModeLight |= MODE_STATICLIGHT;
						EERIERemovePrecalcLights();
						LaunchLightThread(0, 0, 999999, 999999);
					}
					else if (!IsChecked(hWnd, IDC_SHOWLIGHTSNSHADOWS) && wuz)
					{
						ModeLight &= ~MODE_STATICLIGHT;
						EERIERemovePrecalcLights();
					}

					if (IsChecked(hWnd, IDC_TORCHHALO)) ModeLight |= MODE_DYNAMICLIGHT;
					else ModeLight &= ~MODE_DYNAMICLIGHT;

					if (IsChecked(hWnd, IDC_TORCHHALO2))
					{
						SHOW_TORCH = 1;
						ARX_SOUND_Stop(SND_TORCH_LOOP);
						ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
					}
					else
					{
						ARX_SOUND_Stop(SND_TORCH_LOOP);
						SHOW_TORCH = 0;
					}

					if (IsChecked(hWnd, IDC_COLLISIONS)) USE_COLLISIONS = 1;
					else USE_COLLISIONS = 0;

					if (IsChecked(hWnd, IDC_INVERTMOUSE)) INVERTMOUSE = 1;
					else INVERTMOUSE = 0;

					if (IsChecked(hWnd, IDC_INTERNORM)) USEINTERNORM = 1;
					else USEINTERNORM = 0;

					if (IsChecked(hWnd, IDC_THIRDPERSON)) EXTERNALVIEWING = 1;
					else EXTERNALVIEWING = 0;

					if (IsChecked(hWnd, IDC_PLAYERCOLLISIONS))
					{
						USE_PLAYERCOLLISIONS = 1;
						EERIEPOLY * ep = BCCheckInPoly(player.pos.x, player.pos.y, player.pos.z);

						if (ep != NULL) player.pos.y = ep->max.y + PLAYER_BASE_HEIGHT;
					}
					else USE_PLAYERCOLLISIONS = 0;

					if (IsChecked(hWnd, IDC_SHOWNORMALS)) ViewMode |= VIEWMODE_NORMALS;
					else ViewMode &= ~VIEWMODE_NORMALS;

					if (IsChecked(hWnd, IDC_SHOWDEPTH)) ModeLight |= MODE_DEPTHCUEING;
					else ModeLight &= ~MODE_DEPTHCUEING;

					if (IsChecked(hWnd, IDC_INFOTEXT)) ViewMode |= VIEWMODE_INFOTEXT;
					else ViewMode &= ~VIEWMODE_INFOTEXT;

					if (IsChecked(hWnd, IDC_NOTEXTURES)) ViewMode |= VIEWMODE_FLAT;
					else ViewMode &= ~VIEWMODE_FLAT;

					if (IsChecked(hWnd, IDC_MESHTWEAK))  ALLOW_MESH_TWEAKING = 1;
					else ALLOW_MESH_TWEAKING = 0;

					thWnd = GetDlgItem(hWnd, IDC_SLIDERDEPTH);
					long t = SendMessage(thWnd, TBM_GETPOS, true, 0);
					EERIE_CAMERA * oldcam = ACTIVECAM;
					SetActiveCamera(&subj);
					SetCameraDepth((float)t);
					SetActiveCamera(oldcam);

					thWnd = GetDlgItem(hWnd, IDC_TIMESLIDER);
					TIMEFACTOR = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 100 );

					EndDialog(hWnd, true);
					break;
				}

				case IDCANCEL :
					EndDialog(hWnd, true);
					break;
			}

			break;
		}
	}

	return false;
}

long WATERFX = 0;
long REFLECTFX = 0;

INT_PTR CALLBACK OptionsProc_2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	HRESULT hr;

	if (WM_COMMAND == uMsg)
	{

		if (IDC_MODE  == LOWORD(wParam))
		{
			if (!mainApp->m_pDeviceInfo->bWindowed)
			{
				if (SUCCEEDED(D3DEnum_UserChangeDevice(&mainApp->m_pDeviceInfo)))
				{
					ARX_Text_Close();

					if (FAILED(hr = mainApp->Change3DEnvironment()))
					{
						LogError << ("Error Changing Environment");
						return 0;
					}

					ARX_Text_Init();
				}
				else LogError << ("Error Changing Device");
			}
		}

		if (IDC_INTERCOLOR  == LOWORD(wParam))
		{
			CHOOSECOLOR cc;
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = mainApp->m_hWnd;
			cc.hInstance = 0; //Ignored
			cc.rgbResult = (((long)(float)(Project.interfacergb.r * 255.f) & 255))
			               | (((long)(float)(Project.interfacergb.g * 255.f) & 255) << 8)
			               | (((long)(float)(Project.interfacergb.b * 255.f) & 255)) << 16;
			cc.lpCustColors = custcr;
			cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc))
			{
				Project.interfacergb.b = (float)((cc.rgbResult >> 16 & 255)) * ( 1.0f / 255 );
				Project.interfacergb.g = (float)((cc.rgbResult >> 8 & 255)) * ( 1.0f / 255 );
				Project.interfacergb.r = (float)((cc.rgbResult & 255)) * ( 1.0f / 255 );
			}
		}

		if (IDC_INTERCOLOR2  == LOWORD(wParam))
		{
			CHOOSECOLOR cc;
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = mainApp->m_hWnd;
			cc.hInstance = 0; //Ignored
			cc.rgbResult = (((long)(float)(Project.torch.r * 255.f) & 255))
			               | (((long)(float)(Project.torch.g * 255.f) & 255) << 8)
			               | (((long)(float)(Project.torch.b * 255.f) & 255)) << 16;
			cc.lpCustColors = custcr;
			cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc))
			{
				Project.torch.b = (float)((cc.rgbResult >> 16 & 255)) * ( 1.0f / 255 );
				Project.torch.g = (float)((cc.rgbResult >> 8 & 255)) * ( 1.0f / 255 );
				Project.torch.r = (float)((cc.rgbResult & 255)) * ( 1.0f / 255 );
			}
		}

		if (IDOK == LOWORD(wParam))
		{
			if (IsChecked(hWnd, IDC_HIDESPEECH))		HIDESPEECH = 1;
			else HIDESPEECH = 0;

			if (IsChecked(hWnd, IDC_WATERFX))		WATERFX = 1;
			else WATERFX = 0;

			if (IsChecked(hWnd, IDC_REFLECTFX))		REFLECTFX = 1;
			else REFLECTFX = 0;

			if (IsChecked(hWnd, IDC_FORCEIO))		ForceIODraw = 1;
			else ForceIODraw = 0;

			if (IsChecked(hWnd, IDC_TREATALLIO))		TreatAllIO = 1;
			else TreatAllIO = 0;


			if (IsChecked(hWnd, IDC_HIDEMAGICDUST))	HIDEMAGICDUST = 1;
			else HIDEMAGICDUST = 0;

			if (IsChecked(hWnd, IDC_HIDEANCHORS))	HIDEANCHORS = 1;
			else HIDEANCHORS = 0;

			if (IsChecked(hWnd, IDC_HIDEBACKGROUND))	Project.hide |= HIDE_BACKGROUND;
			else Project.hide &= ~HIDE_BACKGROUND;

			if (IsChecked(hWnd, IDC_HIDENPC))		Project.hide |= HIDE_NPC;
			else Project.hide &= ~HIDE_NPC;

			if (IsChecked(hWnd, IDC_HIDEFIXINTER))	Project.hide |= HIDE_FIXINTER;
			else Project.hide &= ~HIDE_FIXINTER;

			if (IsChecked(hWnd, IDC_HIDEITEMS))		Project.hide |= HIDE_ITEMS;
			else Project.hide &= ~HIDE_ITEMS;

			if (IsChecked(hWnd, IDC_HIDEPARTICLES))	Project.hide |= HIDE_PARTICLES;
			else Project.hide &= ~HIDE_PARTICLES;

			if (IsChecked(hWnd, IDC_HIDECAMERAS))	Project.hide |= HIDE_CAMERAS;
			else Project.hide &= ~HIDE_CAMERAS;

			if (IsChecked(hWnd, IDC_HIDEINTERFACE))	Project.hide |= HIDE_INTERFACE;
			else Project.hide &= ~HIDE_INTERFACE;

			if (IsChecked(hWnd, IDC_HIDENODES))		Project.hide |= HIDE_NODES;
			else Project.hide &= ~HIDE_NODES;


			EndDialog(hWnd, true);
		}

		if (IDCANCEL == LOWORD(wParam))
		{
			EndDialog(hWnd, true);
		}
	}

	if (WM_INITDIALOG == uMsg)
	{
		if (HIDESPEECH)						SetClick(hWnd, IDC_HIDESPEECH);

		if (Project.hide & HIDE_BACKGROUND) SetClick(hWnd, IDC_HIDEBACKGROUND);

		if (Project.hide & HIDE_NPC)		SetClick(hWnd, IDC_HIDENPC);

		if (Project.hide & HIDE_FIXINTER)	SetClick(hWnd, IDC_HIDEFIXINTER);

		if (Project.hide & HIDE_ITEMS)		SetClick(hWnd, IDC_HIDEITEMS);

		if (Project.hide & HIDE_PARTICLES)	SetClick(hWnd, IDC_HIDEPARTICLES);

		if (Project.hide & HIDE_CAMERAS)	SetClick(hWnd, IDC_HIDECAMERAS);

		if (Project.hide & HIDE_INTERFACE)	SetClick(hWnd, IDC_HIDEINTERFACE);

		if (Project.hide & HIDE_NODES)		SetClick(hWnd, IDC_HIDENODES);

		if (HIDEANCHORS)					SetClick(hWnd, IDC_HIDEANCHORS);

		if (HIDEMAGICDUST)					SetClick(hWnd, IDC_HIDEMAGICDUST);

		if (WATERFX)						SetClick(hWnd, IDC_WATERFX);

		if (REFLECTFX)						SetClick(hWnd, IDC_REFLECTFX);

		if (TreatAllIO)						SetClick(hWnd, IDC_TREATALLIO);


		return true;
	}

	return false;
	// TODO Nuky - unreachable code
	return WM_INITDIALOG == uMsg ? true : false;
}

INT_PTR CALLBACK ChangeLevelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	if (WM_COMMAND == uMsg)
	{
		if (IDC_GOTOPOLY == LOWORD(wParam))
		{
			ARX_PLAYER_GotoAnyPoly();
			CHANGE_LEVEL_PROC_RESULT = -1;
			EndDialog(hWnd, true);
		}
		else if (IDOK == LOWORD(wParam))
		{
			if (IsChecked(hWnd, IDC_C_LEVEL0))	CHANGE_LEVEL_PROC_RESULT = 0;

			if (IsChecked(hWnd, IDC_C_LEVEL1))	CHANGE_LEVEL_PROC_RESULT = 1;

			if (IsChecked(hWnd, IDC_C_LEVEL2))	CHANGE_LEVEL_PROC_RESULT = 2;

			if (IsChecked(hWnd, IDC_C_LEVEL3))	CHANGE_LEVEL_PROC_RESULT = 3;

			if (IsChecked(hWnd, IDC_C_LEVEL4))	CHANGE_LEVEL_PROC_RESULT = 4;

			if (IsChecked(hWnd, IDC_C_LEVEL5))	CHANGE_LEVEL_PROC_RESULT = 5;

			if (IsChecked(hWnd, IDC_C_LEVEL6))	CHANGE_LEVEL_PROC_RESULT = 6;

			if (IsChecked(hWnd, IDC_C_LEVEL7))	CHANGE_LEVEL_PROC_RESULT = 7;

			if (IsChecked(hWnd, IDC_C_LEVEL8))	CHANGE_LEVEL_PROC_RESULT = 8;

			if (IsChecked(hWnd, IDC_C_LEVEL9))	CHANGE_LEVEL_PROC_RESULT = 9;

			if (IsChecked(hWnd, IDC_C_LEVEL10))	CHANGE_LEVEL_PROC_RESULT = 10;

			if (IsChecked(hWnd, IDC_C_LEVEL11))	CHANGE_LEVEL_PROC_RESULT = 11;

			if (IsChecked(hWnd, IDC_C_LEVEL12))	CHANGE_LEVEL_PROC_RESULT = 12;

			if (IsChecked(hWnd, IDC_C_LEVEL13))	CHANGE_LEVEL_PROC_RESULT = 13;

			if (IsChecked(hWnd, IDC_C_LEVEL14))	CHANGE_LEVEL_PROC_RESULT = 14;

			if (IsChecked(hWnd, IDC_C_LEVEL15))	CHANGE_LEVEL_PROC_RESULT = 15;

			if (IsChecked(hWnd, IDC_C_LEVEL16))	CHANGE_LEVEL_PROC_RESULT = 16;

			if (IsChecked(hWnd, IDC_C_LEVEL17))	CHANGE_LEVEL_PROC_RESULT = 17;

			if (IsChecked(hWnd, IDC_C_LEVEL18))	CHANGE_LEVEL_PROC_RESULT = 18;

			if (IsChecked(hWnd, IDC_C_LEVEL19))	CHANGE_LEVEL_PROC_RESULT = 19;

			if (IsChecked(hWnd, IDC_C_LEVEL20))	CHANGE_LEVEL_PROC_RESULT = 20;

			if (IsChecked(hWnd, IDC_C_LEVEL21))	CHANGE_LEVEL_PROC_RESULT = 21;

			if (IsChecked(hWnd, IDC_C_LEVEL22))	CHANGE_LEVEL_PROC_RESULT = 22;

			if (IsChecked(hWnd, IDC_C_LEVEL23))	CHANGE_LEVEL_PROC_RESULT = 23;

			EndDialog(hWnd, true);
		}
		else if (IDCANCEL == LOWORD(wParam))
		{

			EndDialog(hWnd, true);
		}
	}

	if (WM_INITDIALOG == uMsg)
	{
		CHANGE_LEVEL_PROC_RESULT = -1;
		SetClick(hWnd, IDC_C_LEVEL0);
		return true;
	}

	return false;
	// TODO Nuky - unreachable code
	return WM_INITDIALOG == uMsg ? true : false;
}



EERIE_LIGHT lightparam;
EERIE_LIGHT lightcopy;
long CONSTANTUPDATELIGHT = 0;

void LightApply(HWND hWnd)
{
	HWND thWnd;

	if (CDP_EditLight != NULL)
	{
		lightparam.extras &= EXTRAS_FIREPLACE;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER11);
		lightparam.fallstart = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

		thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
		lightparam.fallend = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

		thWnd = GetDlgItem(hWnd, IDC_SLIDER13);
		lightparam.intensity = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) / 100.f;

		if (IsChecked(hWnd, IDC_SEMIDYNAMIC))	lightparam.extras |= EXTRAS_SEMIDYNAMIC;
		else lightparam.extras &= ~EXTRAS_SEMIDYNAMIC;

		if (IsChecked(hWnd, IDC_EXTINGUISH))		lightparam.extras |= EXTRAS_EXTINGUISHABLE;
		else lightparam.extras &= ~EXTRAS_EXTINGUISHABLE;

		if (IsChecked(hWnd, IDC_EXTINGUISH2))	lightparam.extras |= EXTRAS_STARTEXTINGUISHED;
		else lightparam.extras &= ~EXTRAS_STARTEXTINGUISHED;

		if (IsChecked(hWnd, IDC_NO_IGNIT))	lightparam.extras |= EXTRAS_NO_IGNIT;
		else lightparam.extras &= ~EXTRAS_NO_IGNIT;

		if (IsChecked(hWnd, IDC_SPAWNFIRE))		lightparam.extras |= EXTRAS_SPAWNFIRE;
		else lightparam.extras &= ~EXTRAS_SPAWNFIRE;

		if (IsChecked(hWnd, IDC_SPAWNSMOKE))		lightparam.extras |= EXTRAS_SPAWNSMOKE;
		else lightparam.extras &= ~EXTRAS_SPAWNSMOKE;

		if (IsChecked(hWnd, IDC_CAST_SHADOWS))	lightparam.extras |= EXTRAS_NOCASTED;
		else lightparam.extras &= ~EXTRAS_NOCASTED;

		if (IsChecked(hWnd, IDC_FIXFLARESIZE))	lightparam.extras |= EXTRAS_FIXFLARESIZE;
		else lightparam.extras &= ~EXTRAS_FIXFLARESIZE;

		if (IsChecked(hWnd, IDC_COLORLEGACY))	lightparam.extras |= EXTRAS_COLORLEGACY;
		else lightparam.extras &= ~EXTRAS_COLORLEGACY;

		if (IsChecked(hWnd, IDC_FLARE))	lightparam.extras |= EXTRAS_FLARE;
		else lightparam.extras &= ~EXTRAS_FLARE;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER14);
		lightparam.ex_flicker.r = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 100 );

		thWnd = GetDlgItem(hWnd, IDC_SLIDER15);
		lightparam.ex_flicker.g = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 100 );

		thWnd = GetDlgItem(hWnd, IDC_SLIDER16);
		lightparam.ex_flicker.b = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 100 );

		thWnd = GetDlgItem(hWnd, IDC_SLIDER17);
		lightparam.ex_radius = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

		thWnd = GetDlgItem(hWnd, IDC_SLIDER18);
		lightparam.ex_frequency = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 100 );

		thWnd = GetDlgItem(hWnd, IDC_SLIDER19);
		lightparam.ex_size = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 10 );

		thWnd = GetDlgItem(hWnd, IDC_SLIDER20);
		lightparam.ex_speed = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 100 );

		thWnd = GetDlgItem(hWnd, IDC_SLIDER21);
		lightparam.ex_flaresize = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

		lightparam.pos.x = CDP_EditLight->pos.x;
		lightparam.pos.y = CDP_EditLight->pos.y;
		lightparam.pos.z = CDP_EditLight->pos.z;
		lightparam.tl = CDP_EditLight->tl;

		memcpy(CDP_EditLight, &lightparam, sizeof(EERIE_LIGHT));

		if (CDP_EditLight->tl != -1) DynLight[CDP_EditLight->tl].exist = 0;

		if (!(lightparam.extras & EXTRAS_SEMIDYNAMIC))
		{
			RecalcLight(CDP_EditLight);
			RecalcLightZone(CDP_EditLight->pos.x, CDP_EditLight->pos.z, (long)(CDP_EditLight->fallend * ACTIVEBKG->Xmul) + 1);
		}
	}
	}

long INITT = 0;

static INT_PTR CALLBACK LightOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;
	long l;

	switch (uMsg)
	{
		case WM_MOVE:

			if (mainApp->m_pDeviceInfo->bWindowed)
			{
				RECT rect1, rect2;
				GetWindowRect(mainApp->m_hWnd, &rect1);
				GetWindowRect(hWnd, &rect2);
				long posx = rect2.left - rect1.left;
				long posy = rect2.top - rect1.top;
				Danae_Registry_WriteValue("WND_LightOptions_POSX", posx);
				Danae_Registry_WriteValue("WND_LightOptions_POSY", posy);
			}

			break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CONSTANTUPDATE:
					PrecalcIOLighting(NULL, 0, 1);

					if (CONSTANTUPDATELIGHT)
					{
						CONSTANTUPDATELIGHT = 0;
						thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
						SetWindowText(thWnd, "No Real-Time Update");
					}
					else
					{
						CONSTANTUPDATELIGHT = 1;
						thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
						SetWindowText(thWnd, "Real-Time Update");
						SendMessage(hWnd, WM_COMMAND, IDAPPLY, 0);
					}

					break;
				case IDC_SEMIDYNAMIC:
				case IDC_EXTINGUISH:
				case IDC_EXTINGUISH2:
				case IDC_SPAWNFIRE:
				case IDC_SPAWNSMOKE:
				case IDC_COLORLEGACY:
				case IDC_CAST_SHADOWS:
				case IDC_FIXFLARESIZE:
				case IDC_SLIDER11:
				case IDC_SLIDER12:
				case IDC_SLIDER13:
				case IDC_SLIDER14:
				case IDC_SLIDER15:
				case IDC_SLIDER16:
				case IDC_SLIDER17:
				case IDC_SLIDER18:
				case IDC_SLIDER19:
				case IDC_SLIDER20:
				case IDC_SLIDER21:
				{
					PrecalcIOLighting(NULL, 0, 1);

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);
				}
				break;
				case IDC_LIGHTCOLOR:
					PrecalcIOLighting(NULL, 0, 1);
					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = mainApp->m_hWnd;
					cc.hInstance = 0; //Ignored
					cc.rgbResult = (((long)(float)(lightparam.rgb.r * 255.f) & 255))
					               | (((long)(float)(lightparam.rgb.g * 255.f) & 255) << 8)
					               | (((long)(float)(lightparam.rgb.b * 255.f) & 255)) << 16;
					cc.lpCustColors = custcr;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

					if (ChooseColor(&cc))
					{
						lightparam.rgb.b = (float)((cc.rgbResult >> 16 & 255)) * ( 1.0f / 255 );
						lightparam.rgb.g = (float)((cc.rgbResult >> 8 & 255)) * ( 1.0f / 255 );
						lightparam.rgb.r = (float)((cc.rgbResult & 255)) * ( 1.0f / 255 );
					}

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);

					break;
				case IDAPPLY:
				{
					PrecalcIOLighting(NULL, 0, 1);
					LightApply(hWnd);
				}
				break;
				case IDCOPY:
					PrecalcIOLighting(NULL, 0, 1);
					memcpy(&lightcopy, &lightparam, sizeof(EERIE_LIGHT));
					break;
				case IDPASTE:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.fallstart = lightcopy.fallstart;
					lightparam.fallend = lightcopy.fallend;
					lightparam.intensity = lightcopy.intensity;
					lightparam.rgb.r = lightcopy.rgb.r;
					lightparam.rgb.g = lightcopy.rgb.g;
					lightparam.rgb.b = lightcopy.rgb.b;
					lightparam.type = lightcopy.type;
					RecalcLight(&lightparam);
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);
					break;
				case IDRESET:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.fallstart = 500.f;
					lightparam.fallend = 600.f;
					lightparam.intensity = 1.2f;
					lightparam.rgb.r = 1.f;
					lightparam.rgb.g = 0.f;
					lightparam.rgb.b = 0.f;
					RecalcLight(&lightparam);
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);
					break;
				case IDC_SET_FIRE:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.rgb.r = 0.71f;
					lightparam.rgb.g = 0.43f;
					lightparam.rgb.b = 0.29f;
					lightparam.intensity = 4.f;
					lightparam.fallstart = 250.f;
					lightparam.fallend = 450.f;
					lightparam.ex_flicker.r = 0.25f;
					lightparam.ex_flicker.g = 0.25f;
					lightparam.ex_flicker.b = 0.25f;
					lightparam.ex_radius = 1;
					lightparam.ex_frequency = 0.7f;
					lightparam.ex_size = 0.8f;
					lightparam.ex_speed = 0.7f;
					lightparam.ex_flaresize = 40.f;
					lightparam.extras =	EXTRAS_SEMIDYNAMIC | EXTRAS_EXTINGUISHABLE |
					                    EXTRAS_SPAWNFIRE   | EXTRAS_SPAWNSMOKE | EXTRAS_FLARE;
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);

					break;
				case IDC_SET_FIRE2:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.rgb.r = 0.71f;
					lightparam.rgb.g = 0.43f;
					lightparam.rgb.b = 0.29f;
					lightparam.intensity = 4.f;
					lightparam.fallstart = 350.f;
					lightparam.fallend = 520.f;
					lightparam.ex_flicker.r = 0.25f;
					lightparam.ex_flicker.g = 0.25f;
					lightparam.ex_flicker.b = 0.25f;
					lightparam.ex_radius = 20;
					lightparam.ex_frequency = 0.8f;
					lightparam.ex_size = 1.f;
					lightparam.ex_speed = 0.65f;
					lightparam.ex_flaresize = 95.f;
					lightparam.extras =	EXTRAS_SEMIDYNAMIC | EXTRAS_EXTINGUISHABLE |
					                    EXTRAS_SPAWNFIRE   | EXTRAS_SPAWNSMOKE | EXTRAS_FIREPLACE | EXTRAS_FLARE;;
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);

					break;
				case IDOK:
					PrecalcIOLighting(NULL, 0, 1);
					LightApply(hWnd);
					CDP_LIGHTOptions = NULL;

					EndDialog(hWnd, true);
					break;
				case IDCANCEL:
					PrecalcIOLighting(NULL, 0, 1);
					CDP_LIGHTOptions = NULL;

					if ((CDP_EditLight) && (CDP_EditLight->tl != -1)) DynLight[CDP_EditLight->tl].exist = 0;

					EndDialog(hWnd, true);
					break;
			}
		}
		break;
		case WM_INITDIALOG:

			if (mainApp->m_pDeviceInfo->bWindowed)
			{
				long posx, posy;
				Danae_Registry_ReadValue("WND_LightOptions_POSX", &posx, 0);
				Danae_Registry_ReadValue("WND_LightOptions_POSY", &posy, 0);

				if ((posx != -1) && (posy != -1)
				        && (posx < 1000) && (posx > -1000)
				        && (posy < 1000) && (posy > -1000)
				   )
				{
					RECT rect1;
					GetWindowRect(mainApp->m_hWnd, &rect1);
					posx = rect1.left + posx;
					posy = rect1.left + posy;

					if (posx < 0) posx = 0;

					if (posy < 0) posy = 0;

					SetWindowPos(hWnd, NULL, posx, posy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			INITT = 1;

			if (CONSTANTUPDATELIGHT)
			{
				thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
				SetWindowText(thWnd, "Real-Time Update");
			}
			else
			{
				thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
				SetWindowText(thWnd, "No Real-Time Update");
			}

			if ((CDP_EditLight) && (CDP_EditLight->tl != -1))
			{
				DynLight[CDP_EditLight->tl].exist = 0;
				CDP_EditLight->tl = 0;
			}

			thWnd = GetDlgItem(hWnd, IDC_SLIDER11);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 2000));
			l = (long)(float)(lightparam.fallstart);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 2000));
			l = (long)(float)(lightparam.fallend);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER13);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 500));
			l = (long)(float)(lightparam.intensity * 100.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			if (lightparam.extras & EXTRAS_NOCASTED) SetCheck(hWnd, IDC_CAST_SHADOWS, CHECK);
			else SetCheck(hWnd, IDC_CAST_SHADOWS, UNCHECK);

			if (lightparam.extras & EXTRAS_SEMIDYNAMIC) SetCheck(hWnd, IDC_SEMIDYNAMIC, CHECK);
			else SetCheck(hWnd, IDC_SEMIDYNAMIC, UNCHECK);

			if (lightparam.extras & EXTRAS_EXTINGUISHABLE) SetCheck(hWnd, IDC_EXTINGUISH, CHECK);
			else SetCheck(hWnd, IDC_EXTINGUISH, UNCHECK);

			if (lightparam.extras & EXTRAS_STARTEXTINGUISHED) SetCheck(hWnd, IDC_EXTINGUISH2, CHECK);
			else SetCheck(hWnd, IDC_EXTINGUISH2, UNCHECK);

			if (lightparam.extras & EXTRAS_NO_IGNIT) SetCheck(hWnd, IDC_NO_IGNIT, CHECK);
			else SetCheck(hWnd, IDC_NO_IGNIT, UNCHECK);


			if (lightparam.extras & EXTRAS_SPAWNFIRE) SetCheck(hWnd, IDC_SPAWNFIRE, CHECK);
			else SetCheck(hWnd, IDC_SPAWNFIRE, UNCHECK);

			if (lightparam.extras & EXTRAS_SPAWNSMOKE) SetCheck(hWnd, IDC_SPAWNSMOKE, CHECK);
			else SetCheck(hWnd, IDC_SPAWNSMOKE, UNCHECK);

			if (lightparam.extras & EXTRAS_COLORLEGACY) SetCheck(hWnd, IDC_COLORLEGACY, CHECK);
			else SetCheck(hWnd, IDC_COLORLEGACY, UNCHECK);

			if (lightparam.extras & EXTRAS_FLARE) SetCheck(hWnd, IDC_FLARE, CHECK);
			else SetCheck(hWnd, IDC_FLARE, UNCHECK);

			if (lightparam.extras & EXTRAS_FIXFLARESIZE) SetCheck(hWnd, IDC_FIXFLARESIZE, CHECK);
			else SetCheck(hWnd, IDC_FIXFLARESIZE, UNCHECK);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER14);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_flicker.r * 100.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER15);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_flicker.g * 100.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER16);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_flicker.b * 100.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER17);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_radius);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER18);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1, 100));
			l = (long)((float)(lightparam.ex_frequency * 100.f));
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER19);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1, 30));
			l = (long)((float)(lightparam.ex_size * 10.f));
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER20);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 100));
			l = (long)((float)(lightparam.ex_speed) * 100.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER21);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 200));
			l = (long)((float)(lightparam.ex_flaresize));
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(l));
			INITT = 0;
			return true;
			break;
		case WM_NOTIFY:
			float val, val1, val2;
			char temp[64];
			thWnd = GetDlgItem(hWnd, IDC_SLIDER11);
			val1 = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC11);
			sprintf(temp, "%4.0f", val1);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
			val2 = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC12);
			sprintf(temp, "%4.0f", val2);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER13);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 100 );
			thWnd = GetDlgItem(hWnd, IDC_STATIC13);
			sprintf(temp, "%2.2f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER14);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC14);
			sprintf(temp, "%3ld%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER15);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC15);
			sprintf(temp, "%3ld%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER16);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC16);
			sprintf(temp, "%3ld%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER17);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC17);
			sprintf(temp, "%3ldcm", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER18);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC18);
			sprintf(temp, "%3ld%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER19);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC19);
			sprintf(temp, "%3ld", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER20);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC20);
			sprintf(temp, "%3ld%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER21);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC21);
			sprintf(temp, "%3ld", (long)val);
			SetWindowText(thWnd, temp);

			if (val2 < val1)
			{
				thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
				SendMessage(thWnd, TBM_SETPOS, false, (LPARAM)(val1));
			}

			if (!INITT)
				if (((int) wParam == IDC_SLIDER11)
				        ||	((int) wParam == IDC_SLIDER12)
				        ||	((int) wParam == IDC_SLIDER13)
				        ||	((int) wParam == IDC_SLIDER14)
				        ||	((int) wParam == IDC_SLIDER15)
				        ||	((int) wParam == IDC_SLIDER16)
				        ||	((int) wParam == IDC_SLIDER17)
				        ||	((int) wParam == IDC_SLIDER18)
				        ||	((int) wParam == IDC_SLIDER19)
				        ||	((int) wParam == IDC_SLIDER20)
				        ||	((int) wParam == IDC_SLIDER21))
					if (CONSTANTUPDATELIGHT) LightApply(hWnd);

			break;
	}

	return false;
}

FOG_DEF fogparam;
FOG_DEF fogcopy;

INT_PTR CALLBACK FogOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;
	float t;

	switch (uMsg)
	{
		case WM_COMMAND:
		{
			if (IDC_BUTTON_COLOR == LOWORD(wParam))
			{
				CHOOSECOLOR cc;
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = mainApp->m_hWnd;
				cc.hInstance = 0; //Ignored
				cc.rgbResult = (((long)(float)(fogparam.rgb.r * 255.f) & 255))
				               | (((long)(float)(fogparam.rgb.g * 255.f) & 255) << 8)
				               | (((long)(float)(fogparam.rgb.b * 255.f) & 255)) << 16;
				cc.lpCustColors = custcr;
				cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

				if (ChooseColor(&cc))
				{
					fogparam.rgb.b = (float)((cc.rgbResult >> 16 & 255)) * ( 1.0f / 255 );
					fogparam.rgb.g = (float)((cc.rgbResult >> 8 & 255)) * ( 1.0f / 255 );
					fogparam.rgb.r = (float)((cc.rgbResult & 255)) * ( 1.0f / 255 );
				}
			}

			if (IDAPPLY == LOWORD(wParam))
			{
				if (CDP_EditFog != NULL)
				{
					thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
					fogparam.rotatespeed = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) / 1000.f;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
					fogparam.speed = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
					fogparam.size = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
					fogparam.scale = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
					fogparam.tolive = (long)SendMessage(thWnd, TBM_GETPOS, true, 0) * 100;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
					fogparam.frequency = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_DIRECTIONAL);

					if (SendMessage(thWnd, BM_GETSTATE, 0, 0) == BST_CHECKED)
						fogparam.special |= FOG_DIRECTIONAL;

					fogparam.pos.x = CDP_EditFog->pos.x;
					fogparam.pos.y = CDP_EditFog->pos.y;
					fogparam.pos.z = CDP_EditFog->pos.z;
					memcpy(CDP_EditFog, &fogparam, sizeof(FOG_DEF));
				}
			}

			if (IDCOPY == LOWORD(wParam))
			{
				memcpy(&fogcopy, &fogparam, sizeof(FOG_DEF));
			}

			if (IDPASTE == LOWORD(wParam))
			{
				fogparam.angle.a = fogcopy.angle.a;
				fogparam.angle.b = fogcopy.angle.b;
				fogparam.angle.g = fogcopy.angle.g;
				fogparam.blend = fogcopy.blend;
				fogparam.frequency = fogcopy.frequency;
				fogparam.move.x = fogcopy.move.x;
				fogparam.move.y = fogcopy.move.y;
				fogparam.move.z = fogcopy.move.z;
				fogparam.rgb.r = fogcopy.rgb.r;
				fogparam.rgb.g = fogcopy.rgb.g;
				fogparam.rgb.b = fogcopy.rgb.b;
				fogparam.rotatespeed = fogcopy.rotatespeed;
				fogparam.scale = fogcopy.scale;
				fogparam.size = fogcopy.size;
				fogparam.special = fogcopy.special;
				fogparam.speed = fogcopy.speed;
				fogparam.tolive = fogcopy.tolive;
				SendMessage(hWnd, WM_INITDIALOG, 0, 0);
			}

			if (IDRESET == LOWORD(wParam))
			{
				fogparam.angle.a = 0.f;
				fogparam.angle.b = 0.f;
				fogparam.angle.g = 0.f;
				fogparam.move.x = 0.f;
				fogparam.move.y = 0.f;
				fogparam.move.z = 0.f;
				fogparam.special = (long)0.f;
				fogparam.blend = (long)0.f;
				fogparam.frequency = 17.f;
				fogparam.rgb.r = 0.3f;
				fogparam.rgb.g = 0.3f;
				fogparam.rgb.b = 0.5f;
				fogparam.rotatespeed = 0.001f;
				fogparam.scale = 8.f;
				fogparam.size = 80.f;
				fogparam.speed = 1.f;
				fogparam.tolive = 4500;
				SendMessage(hWnd, WM_INITDIALOG, 0, 0);
			}

			if (IDOK == LOWORD(wParam))
			{
				if (CDP_EditFog != NULL)
				{
					thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
					fogparam.rotatespeed = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) / 1000.f;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
					fogparam.speed = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
					fogparam.size = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
					fogparam.scale = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
					fogparam.tolive = (long)SendMessage(thWnd, TBM_GETPOS, true, 0) * 100;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
					fogparam.frequency = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);

					thWnd = GetDlgItem(hWnd, IDC_DIRECTIONAL);

					if (SendMessage(thWnd, BM_GETSTATE, 0, 0) == BST_CHECKED)
						fogparam.special |= FOG_DIRECTIONAL;

					memcpy(CDP_EditFog, &fogparam, sizeof(FOG_DEF));
				}

				CDP_FogOptions = NULL;
				EndDialog(hWnd, true);
			}

			if (IDCANCEL == LOWORD(wParam))
			{
				CDP_FogOptions = NULL;
				EndDialog(hWnd, true);
			}
		}
		break;
		case WM_INITDIALOG:
		{

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(-100, 100));
			t = (float)(long)(fogparam.rotatespeed * 1000.f);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(0, 100));
			t = (float)(long)(fogparam.speed);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1, 80));
			t = (float)(long)(fogparam.size);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(-80, 80));
			t = (float)(long)(fogparam.scale);
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1, 400));
			t = (float)(long)(fogparam.tolive) / 100;
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
			SendMessage(thWnd, TBM_SETRANGE, true, (LPARAM) MAKELONG(1, 99));
			t = (float)(long)fogparam.frequency;
			SendMessage(thWnd, TBM_SETPOS, true, (LPARAM)(t));



			if (fogparam.special & FOG_DIRECTIONAL) SetClick(hWnd, IDC_DIRECTIONAL);

			return true;
		}
		case WM_NOTIFY:
			float val;
			char temp[64];
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY_TEXT);
			sprintf(temp, "%2.0f%%", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
			val = ((float)SendMessage(thWnd, TBM_GETPOS, true, 0)) / 1000.f;
			thWnd = GetDlgItem(hWnd, IDC_STATIC2);
			sprintf(temp, "%1.3f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC3);
			sprintf(temp, "%3.0f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC4);
			sprintf(temp, "%3.0f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC5);
			sprintf(temp, "%3.0f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
			val = (float)SendMessage(thWnd, TBM_GETPOS, true, 0) * ( 1.0f / 10 );
			thWnd = GetDlgItem(hWnd, IDC_STATIC6);
			sprintf(temp, "%4.2fs", val);
			SetWindowText(thWnd, temp);
			break;
	}

	return false;
}
long SHOWWARNINGS = 0;
#define MAX_SCRIPT_SIZE 128000
char text1[MAX_SCRIPT_SIZE+1];
char text2[MAX_SCRIPT_SIZE+1];
 
//*************************************************************************************
//*************************************************************************************
 
extern HWND LastErrorPopupNO2;
extern HWND LastErrorPopupNO1;

UINT uFindReplaceMsg;

long IOScript_X = -1;
long IOScript_Y = -1;
long IOScript_XX = -1;
long IOScript_YY = -1;

INTERACTIVE_OBJ * edit_io = NULL;

//*************************************************************************************
//*************************************************************************************

INT_PTR CALLBACK IOOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	(void)lParam;
	
	HWND thWnd;

	switch (uMsg)
	{
		case WM_SIZE:
			RECT trec, rec, rec2, wndrect;
			GetWindowRect(hWnd, &wndrect);
			GetClientRect(hWnd, &trec);
			long space;
			space = ((trec.bottom - 60) / 2) - 25;

			rec.left = trec.left + 4;
			rec.right = trec.right - 6;
			rec.top = 60;
			rec.bottom = space;

			rec2.left = rec.left;
			rec2.right = rec.right;
			rec2.top = rec.top + rec.bottom + 25;
			rec2.bottom = space;


			thWnd = GetDlgItem(hWnd, IDC_EDIT1);
			SetWindowPos(thWnd, HWND_TOP, rec.left, rec.top, rec.right, rec.bottom, SWP_NOZORDER);
			UpdateWindow(thWnd);
			thWnd = GetDlgItem(hWnd, IDC_EDIT2);
			SetWindowPos(thWnd, HWND_TOP, rec2.left, rec2.top, rec2.right, rec2.bottom, SWP_NOZORDER);
			UpdateWindow(thWnd);

			// Primary win
			long px, py;
			px = rec.left;
			py = rec.top - 22;
			thWnd = GetDlgItem(hWnd, IDC_LOCSCR1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICCOL1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_COL1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICLINE1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_LINE1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);

			// Secondary win
			px = rec2.left;
			py = rec2.top - 22;
			thWnd = GetDlgItem(hWnd, IDC_LOCSCR2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICCOL2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_COL2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICLINE2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_LINE2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, true);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);


			UpdateWindow(hWnd);
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			KillLightThread();

			CDP_IOOptions = NULL;
			RECT _wndrect;
			GetWindowRect(hWnd, &_wndrect);
			IOScript_X = _wndrect.left;
			IOScript_Y = _wndrect.top;
			IOScript_XX = _wndrect.right - _wndrect.left;
			IOScript_YY = _wndrect.bottom - _wndrect.top;
			EndDialog(hWnd, true);
			break;
		case WM_COMMAND:
		{
			if (IDSYNTAX == LOWORD(wParam))
			{
				if (CDP_EditIO != NULL)
				{
					long SC = SYNTAXCHECKING;
					SYNTAXCHECKING = 1;
					SHOWWARNINGS = 1;
					CDP_EditIO->ioflags &= ~IO_FREEZESCRIPT;

					if (CheckScriptSyntax(CDP_EditIO) != true) CDP_EditIO->ioflags |= IO_FREEZESCRIPT;
					else CDP_EditIO->ioflags &= ~IO_FREEZESCRIPT;

					SHOWWARNINGS = 0;
					RECT rec;
					GetWindowRect(hWnd, &rec);

					if (LastErrorPopupNO2 != NULL)
					{
						SetWindowPos(LastErrorPopupNO2, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top + 200, 0, 0, SWP_NOSIZE);
						SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
						LastErrorPopupNO2 = NULL;
					}

					if (LastErrorPopupNO1 != NULL)
					{
						SetWindowPos(LastErrorPopupNO1, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top, 0, 0, SWP_NOSIZE);
						SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
						LastErrorPopupNO1 = NULL;
					}

					SYNTAXCHECKING = SC;
				}
			}
			else if (IDOK == LOWORD(wParam))
			{
				if (CDP_EditIO != NULL)
				{
					edit_io = CDP_EditIO;

					if (IsChecked(hWnd, IDC_FREEZESCRIPT))	CDP_EditIO->ioflags |= IO_FREEZESCRIPT;
					else CDP_EditIO->ioflags &= ~IO_FREEZESCRIPT;

					long i = 0;

					if (CDP_EditIO->script.data != NULL)
					{
						int n = strcmp(text1, CDP_EditIO->script.data);

						if (n) i += 1;
					}
					else if (strlen(text1) > 0) i += 1;

					if ((CDP_EditIO->over_script.data != NULL)
					        && (CDP_EditIO->ident != -1))
					{
						int n = strcmp(text2, CDP_EditIO->over_script.data);

						if (n) i += 2;
					}
					else if (strlen(text2) > 0) i += 2;

					switch (i)
					{
						case 3:

							if (OKBox("Save Changes to LOCAL & CLASS script ?", "SAVE Confirmation"))
							{
								if (CDP_EditIO->script.data != NULL)
								{
									free(CDP_EditIO->script.data);
									CDP_EditIO->script.data = NULL;
								}

								CDP_EditIO->script.size = strlen(text1) + 1;
								CDP_EditIO->script.data = (char *)malloc(CDP_EditIO->script.size);
								strcpy(CDP_EditIO->script.data, text1);

								if (CDP_EditIO->over_script.data != NULL)
								{
									free(CDP_EditIO->over_script.data);
									CDP_EditIO->over_script.data = NULL;
								}

								CDP_EditIO->over_script.size = strlen(text2) + 1;
								CDP_EditIO->over_script.data = (char *)malloc(CDP_EditIO->over_script.size);
								strcpy(CDP_EditIO->over_script.data, text2);

								if (CDP_EditIO->script.data != NULL)
									CDP_EditIO->over_script.master = (void *)&CDP_EditIO->script;
								else CDP_EditIO->over_script.master = NULL;

								SaveIOScript(CDP_EditIO, 1);
								SaveIOScript(CDP_EditIO, 2);
							}

							break;
						case 2:

							if (OKBox("Save Changes to LOCAL script ?", "SAVE Confirmation"))
							{
								if (CDP_EditIO->over_script.data != NULL)
								{
									free(CDP_EditIO->over_script.data);
									CDP_EditIO->over_script.data = NULL;
								}

								CDP_EditIO->over_script.size = strlen(text2) + 1;
								CDP_EditIO->over_script.data = (char *)malloc(CDP_EditIO->over_script.size);
								strcpy(CDP_EditIO->over_script.data, text2);

								if (CDP_EditIO->script.data != NULL)
									CDP_EditIO->over_script.master = (void *)&CDP_EditIO->script;
								else CDP_EditIO->over_script.master = NULL;

								SaveIOScript(CDP_EditIO, 2);
							}

							break;
						case 1:

							if (OKBox("Save Changes to CLASS script ?", "SAVE Confirmation"))
							{
								if (CDP_EditIO->script.data != NULL)
								{
									free(CDP_EditIO->script.data);
									CDP_EditIO->script.data = NULL;
								}

								CDP_EditIO->script.size = strlen(text1) + 1;
								CDP_EditIO->script.data = (char *)malloc(CDP_EditIO->script.size);
								strcpy(CDP_EditIO->script.data, text1);

								if (CDP_EditIO->script.data != NULL)
									CDP_EditIO->over_script.master = (void *)&CDP_EditIO->script;
								else CDP_EditIO->over_script.master = NULL;

								SaveIOScript(CDP_EditIO, 1);
							}

							break;
					}

					if (!(CDP_EditIO->ioflags & IO_FREEZESCRIPT))
						if (CheckScriptSyntax(CDP_EditIO) != true) CDP_EditIO->ioflags |= IO_FREEZESCRIPT;
				}

				CDP_IOOptions = NULL;
				RECT _wndrect;
				GetWindowRect(hWnd, &_wndrect);
				IOScript_X = _wndrect.left;
				IOScript_Y = _wndrect.top;
				IOScript_XX = _wndrect.right - _wndrect.left;
				IOScript_YY = _wndrect.bottom - _wndrect.top;
				EndDialog(hWnd, true);
			}

			if (IDCANCEL == LOWORD(wParam))
			{
			
				CDP_IOOptions = NULL;
				RECT _wndrect;
				GetWindowRect(hWnd, &_wndrect);
				IOScript_X = _wndrect.left;
				IOScript_Y = _wndrect.top;
				IOScript_XX = _wndrect.right - _wndrect.left;
				IOScript_YY = _wndrect.bottom - _wndrect.top;
				EndDialog(hWnd, true);
			}
		}
		break;
		case WM_INITDIALOG:
		{
			uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

			if (CDP_EditIO)
			{
				// Isis Athena
				char temp[256];
				thWnd = GetDlgItem(hWnd, IDC_OBJNAME);
				sprintf(temp, "%s_%04ld", GetName(CDP_EditIO->filename).c_str(), CDP_EditIO->ident);
				SetWindowText(thWnd, temp);

				thWnd = GetDlgItem(hWnd, IDC_EDIT1);

				if (CDP_EditIO->ioflags & IO_FREEZESCRIPT) SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);

				RECT rec;
				GetWindowRect(hWnd, &rec);

				if (LastErrorPopupNO2 != NULL)
				{
					SetWindowPos(LastErrorPopupNO2, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top + 200, 0, 0, SWP_NOSIZE);
					SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
				}

				if (LastErrorPopupNO1 != NULL)
				{
					SetWindowPos(LastErrorPopupNO1, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top, 0, 0, SWP_NOSIZE);
					SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
				}
			}

			if (IOScript_X != -1)
			{
				SetWindowPos(hWnd, NULL, IOScript_X, IOScript_Y, IOScript_XX, IOScript_YY, SWP_NOZORDER);
			}

			return true;
		}

	}

	return false;

}
extern HWND CDP_SOUNDOptions;

//*************************************************************************************
// Creates A Text Box
//*************************************************************************************
void TextBox(const char * title, char * text, long size)
{
	ARX_TIME_Pause();
	GTE_TITLE = title;
	GTE_TEXT = text;
	GTE_SIZE = size;
	DialogBox((HINSTANCE)GetWindowLongPtr(mainApp->m_hWnd, GWLP_HINSTANCE),
	          MAKEINTRESOURCE(IDD_GAIATEXTEDIT), mainApp->m_hWnd, GaiaTextEdit);
	ARX_TIME_UnPause();
}

void launchlightdialog()
{
	if ((LastSelectedLight != -1)
	        && (GLight[LastSelectedLight] != NULL)
	        && (GLight[LastSelectedLight]->exist))
	{
		if (!CDP_LIGHTOptions)
		{
			memcpy(&lightparam, GLight[LastSelectedLight], sizeof(EERIE_LIGHT));
			CDP_EditLight = GLight[LastSelectedLight];

			if (mainApp->m_pFramework->m_bIsFullscreen)
			{
				ARX_TIME_Pause();
				mainApp->Pause(true);
				DialogBox((HINSTANCE)GetWindowLongPtr(mainApp->m_hWnd, GWLP_HINSTANCE),
				          MAKEINTRESOURCE(IDD_LIGHTDIALOG), mainApp->m_hWnd, LightOptionsProc);
				mainApp->Pause(false);
				ARX_TIME_UnPause();
			}
			else
				CDP_LIGHTOptions = (CreateDialogParam((HINSTANCE)GetWindowLongPtr(mainApp->m_hWnd, GWLP_HINSTANCE),
				                                      MAKEINTRESOURCE(IDD_LIGHTDIALOG), mainApp->m_hWnd, LightOptionsProc, 0));
		}
	}
}

#endif // BUILD_EDITOR
