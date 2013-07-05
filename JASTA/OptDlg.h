//----------------------------------------------------------------------------
#ifndef OCBH
#define OCBH
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#include "ComLib.h"
//----------------------------------------------------------------------------
class TOptDlgBox : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TLabel *L1;
	TEdit *EditCall;
	TLabel *L2;
	TComboBox *EditYear;
	TRadioGroup *RGC;
	TRadioGroup *RGF;
	TLabel *L3;
	TEdit *EditZip;
	TLabel *L4;
	TEdit *EditAdr;
	TLabel *L5;
	TEdit *EditMail;
	TLabel *L6;
	TEdit *EditName;
	TLabel *L7;
	TEdit *EditLicense;
	TLabel *L8;
	TEdit *EditPow;
	TRadioGroup *RGL;
	TComboBox *CBSex;
	TLabel *L9;
	TComboBox *CBTSize;
	void __fastcall RGLClick(TObject *Sender);
private:
	void __fastcall SetLanguage(void);

public:
	virtual __fastcall TOptDlgBox(TComponent* AOwner);
	int __fastcall Execute(void);
};
//----------------------------------------------------------------------------
//extern PACKAGE TOptDlgBox *OptDlgBox;
//----------------------------------------------------------------------------
#endif
