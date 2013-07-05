//----------------------------------------------------------------------------
#ifndef QSODlgH
#define QSODlgH
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
#include "LogFile.h"
//----------------------------------------------------------------------------
class TQSODlgBox : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TLabel *LOrg;
	TLabel *L1;
	TLabel *L2;
	TEdit *EditDXCC;
	TComboBox *EditCont;
private:
public:
	virtual __fastcall TQSODlgBox(TComponent* AOwner);

	int __fastcall Execute(SDMMLOG *sp);
};
//----------------------------------------------------------------------------
//extern PACKAGE TQSODlgBox *QSODlgBox;
//----------------------------------------------------------------------------
#endif
