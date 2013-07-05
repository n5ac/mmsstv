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
//----------------------------------------------------------------------------
class TAboutDlg : public TForm
{
__published:
	TButton *OKBtn;
	TLabel *LVer;
	TImage *Image;
private:
public:
	virtual __fastcall TAboutDlg(HWND hWnd);
};
//----------------------------------------------------------------------------
//extern PACKAGE TAboutDlg *AboutDlg;
//----------------------------------------------------------------------------
#endif
