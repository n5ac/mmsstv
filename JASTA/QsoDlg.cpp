//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "QsoDlg.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//TQSODlgBox *QSODlgBox;
//---------------------------------------------------------------------
__fastcall TQSODlgBox::TQSODlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
	if( MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;

		L2->Caption = "Continent";
		CancelBtn->Caption = "Cancel";
	}
}
//---------------------------------------------------------------------
int __fastcall TQSODlgBox::Execute(SDMMLOG *sp)
{
	char bf[256];

	sprintf(bf, "%s %2u/%02u %02u%02u %s %s %s %s\n",
		_band[sp->band],
		sp->date / 100, sp->date % 100,
		(sp->btime/30)/60, (sp->btime/30)%60,
		sp->call,
		sp->my,
		sp->opt1,
		sp->opt2
	);
	LOrg->Caption = bf;
	EditDXCC->Text = sp->opt1;
	EditCont->Text = sp->opt2;
	if( ShowModal() == IDOK ){
		StrCopy(sp->opt1, AnsiString(EditDXCC->Text).c_str(), MLOPT);	//ja7ude 0525
		jstrupr(sp->opt1);
		StrCopy(sp->opt2, AnsiString(EditCont->Text).c_str(), MLOPT);	//ja7ude 0525
		jstrupr(sp->opt2);
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
