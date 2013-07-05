//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "About.h"
#include "ComLib.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TAboutDlg *AboutDlg;
//--------------------------------------------------------------------- 
__fastcall TAboutDlg::TAboutDlg(HWND hWnd)
	: TForm(hWnd)
{
	if( sys.m_MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;
		Caption = TTL;
	}
	else {
		Caption = TTLJ;
	}
	LVer->Caption = VER" "AUT;
	HICON hIcon = ::LoadIcon(HInstance, (LPCSTR)8000);
	TIcon *pIcon = new TIcon;
	pIcon->Handle = hIcon;
	Image->Picture->Icon = pIcon;
}
//---------------------------------------------------------------------

