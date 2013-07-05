//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "OptDlg.h"
#include "Country.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TOptDlgBox *OptDlgBox;
//---------------------------------------------------------------------
__fastcall TOptDlgBox::TOptDlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TOptDlgBox::SetLanguage(void)
{
	if( RGL->ItemIndex ){
		Caption = "Setup";
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;

		RGL->Caption = "Language";
		RGL->Items->Strings[0] = "Japanese";
		RGL->Items->Strings[1] = "English";
		L1->Caption = "Callsign";
		L2->Caption = "Year";
		RGC->Caption = "Entry Section";
		RGC->Items->Strings[0] = "J (Japan)";
		RGC->Items->Strings[1] = "S";
		RGF->Caption = "First time entry";
		RGF->Items->Strings[0] = "No";
		RGF->Items->Strings[1] = "Yes";
		L3->Caption = "ZIP";
		L4->Caption = "Address";
//        L5->Caption = "E-Mail";
		L6->Caption = "Name";
		L7->Caption = "License";
		L9->Caption = "Size of T-shirts as an Activity Premium";

		CancelBtn->Caption = "Cancel";
	}
	else {
		Caption = "ƒZƒbƒgƒAƒbƒv";
		Font->Name = "‚l‚r ‚oƒSƒVƒbƒN";
		Font->Charset = SHIFTJIS_CHARSET;

		RGL->Caption = "Œ¾Œê (Language)";
		RGL->Items->Strings[0] = "“ú–{Œê(Japanese)";
		RGL->Items->Strings[1] = "‰pŒê(English)";
		L1->Caption = "º°Ù»²Ý";
		L2->Caption = "WŒv”N“x";
		RGC->Caption = "ŽQ‰Á•”–å";
		RGC->Items->Strings[0] = "J (“ú–{‘“à)";
		RGC->Items->Strings[1] = "S (ŠCŠO)";
		RGF->Caption = "‰ŽQ‰Á";
		RGF->Items->Strings[0] = "‰ß‹Ž‚É‚àŽQ‰Á‚µ‚½";
		RGF->Items->Strings[1] = "‰ŽQ‰Á";
		L3->Caption = "—X•Ö”Ô†";
		L4->Caption = "ZŠ";
//        L5->Caption = "E-Mail";
		L6->Caption = "Ž–¼";
		L7->Caption = "Ž‘Ši";
		L9->Caption = "±¸Ã¨ÌÞÜ‚Ì‚s|¼¬Â“–‘IŽž‚ÌŠó–]»²½Þ";
		CancelBtn->Caption = "·¬Ý¾Ù";
	}
}
//---------------------------------------------------------------------
int __fastcall TOptDlgBox::Execute(void)
{
	RGL->ItemIndex = MsgEng;
	SetLanguage();

	EditCall->Text = sys.m_Call;
	EditYear->Text = sys.m_Year;
	RGC->ItemIndex = sys.m_Category;
	RGF->ItemIndex = sys.m_First;
	EditZip->Text = sys.m_Zip;
	EditAdr->Text = sys.m_Adr;
	EditMail->Text = sys.m_Mail;
	EditName->Text = sys.m_Name;
	EditLicense->Text = sys.m_License;
	EditPow->Text = sys.m_Pow;
    CBSex->ItemIndex = sys.m_Sexual;
	CBTSize->ItemIndex = sys.m_TSize;

	if( ShowModal() == IDOK ){
		sys.m_Call = EditCall->Text.c_str();
		jstrupr(sys.m_Call.c_str());
		sscanf(AnsiString(EditYear->Text).c_str(), "%u", &sys.m_Year);

		sys.m_Category = RGC->ItemIndex;
		sys.m_First = RGF->ItemIndex;
		sys.m_Zip = EditZip->Text;
		sys.m_Adr = EditAdr->Text;
		sys.m_Mail = EditMail->Text;
		sys.m_Name = EditName->Text;
		sys.m_License = EditLicense->Text;
		sys.m_Pow = EditPow->Text;
	    sys.m_Sexual = CBSex->ItemIndex;
		sys.m_TSize = CBTSize->ItemIndex;

		MsgEng = RGL->ItemIndex;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TOptDlgBox::RGLClick(TObject *Sender)
{
	SetLanguage();
}
//---------------------------------------------------------------------------

