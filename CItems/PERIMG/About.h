//----------------------------------------------------------------------------
#ifndef OCRH
#define OCRH
//----------------------------------------------------------------------------
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <StdCtrls.hpp>
#include <Controls.hpp>
#include <Forms.hpp>
#include <Graphics.hpp>
#include <Classes.hpp>
#include <SysUtils.hpp>
#include <Windows.hpp>
#include <System.hpp>
#include "ComLib.h"
//----------------------------------------------------------------------------
class TAboutDlg : public TForm
{
__published:
	TButton *OKBtn;
	TLabel *LVer;
	TImage *Image;
private:
public:
#if UseHWND
	virtual __fastcall TAboutDlg(HWND hWnd);
#else
	virtual __fastcall TAboutDlg(TComponent* AOwner);
#endif
};
//----------------------------------------------------------------------------
//extern PACKAGE TAboutDlg *AboutDlg;
//----------------------------------------------------------------------------
#endif
