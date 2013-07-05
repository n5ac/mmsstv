//----------------------------------------------------------------------------
#ifndef TextEditH
#define TextEditH
//----------------------------------------------------------------------------
//ja7ude 0525
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
//----------------------------------------------------------------------------
class TTextEditDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TMemo *Memo;
	TButton *FontBtn;
	TFontDialog *FontDialog;
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	
	
	
	
	void __fastcall FormPaint(TObject *Sender);
	
	
private:
	int		InitFlag;
	LPCSTR	ap;

	CAlignList	AlignList;
	void __fastcall EntryAlignControl(void);
public:
	virtual __fastcall TTextEditDlg(TComponent* AOwner);

	int __fastcall Execute(AnsiString &as, int flag, LPCSTR pTitle = NULL);
};
//----------------------------------------------------------------------------
//extern TTextEditDlg *TextEditDlg;

extern void __fastcall ShowHelp(TForm *pForm, LPCSTR pName);
//----------------------------------------------------------------------------
#endif    
