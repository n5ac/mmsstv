//Copyright+LGPL

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000-2013 Makoto Mori, Nobuyuki Oba
//-----------------------------------------------------------------------------------------------------------------------------------------------
// This file is part of MMSSTV.

// MMSSTV is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// MMSSTV is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License along with MMTTY.  If not, see 
// <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "LogSet.h"
#include "country.h"
#include "Loglink.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TLogSetDlg *LogSetDlg;
static int PageIndex = 0;
//---------------------------------------------------------------------
__fastcall TLogSetDlg::TLogSetDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_DisEvent = 1;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		TabConv->Caption = "Conversion";
		TabMisc->Caption = "Misc";
		CancelBtn->Caption = "Cancel";
		Caption = "Setup Logging";
		TabFlag->Caption = "Input";
		GrpConv->Caption = "Convert UpperCase";
		GrpCopy->Caption = "Copy Before Data";
		CopyFreq->Caption = "Copy Band or Freq.";
		CopyFreq->Items->Strings[0] = "Copy Band";
		CopyFreq->Items->Strings[1] = "Copy Freq.";
		DefMyRST->Caption = "595 Default";

		RGDupe->Caption = "Dupe Check (Show Red)";
		RGDupe->Items->Strings[0] = "Callsign only";
		RGDupe->Items->Strings[1] = "Hit on the same band";
		AutoZone->Caption = "Ignore daylight saving";
		CBBackup->Caption = "Make backup";
        CBFSKNR->Caption = "Add contest NR to the FSKID";
		CBRemC->Visible = FALSE;
        L1->Caption = "Delay time";
        GB7->Caption = "Template";
        GB9->Caption = "Mode";
	}
	else {
		LLink->Caption = "Hamlogの環境設定-設定1で\r\n「交信履歴表示で一旦停止」のチェックは付けないで下さい.";
	}
	if( lcid == LANG_KOREAN ){
		TimeZone->Items->Strings[0] = "Korea";
	}
	else {
		TimeZone->Items->Strings[0] = "Japan";
	}
	m_MMList.QueryList("MML");
	for( int i = 0; i < m_MMList.GetCount(); i++ ){
		CBMMLink->Items->Add(m_MMList.GetItemName(i));
	}
}
//---------------------------------------------------------------------
void __fastcall TLogSetDlg::UpdateUI(void)
{
	int f = !AutoZone->Checked;
	TimeOff->Enabled = f;
	MinOff->Enabled = f;
	UDOffset->Enabled = f;
	UDMin->Enabled = f;
	ClearOff->Enabled = f;
	CBPoll->Enabled = RGLink->ItemIndex ? TRUE : FALSE;
	if( !MsgEng ){
		LLink->Visible = (RGLink->ItemIndex == 1) ? TRUE : FALSE;
	}
	CBMMLink->Enabled = (RGLink->ItemIndex == 2) ? TRUE : FALSE;
	LT->Enabled = CBMMLink->Enabled;
}
//---------------------------------------------------------------------
int __fastcall TLogSetDlg::Execute(void)
{
	int i;
	for( i = 0; i < 5; i++ ){
		m_MacroStr[i] = Log.m_LogSet.m_QSOMacroStr[i];
		m_MacroKey[i] = Log.m_LogSet.m_QSOMacroKey[i];
	}
	UpperName->Checked = Log.m_LogSet.m_UpperName;
	UpperQTH->Checked = Log.m_LogSet.m_UpperQTH;
	UpperREM->Checked = Log.m_LogSet.m_UpperREM;
	UpperQSL->Checked = Log.m_LogSet.m_UpperQSL;

	DefMyRST->Checked = Log.m_LogSet.m_DefMyRST;

	CopyFreq->ItemIndex = Log.m_LogSet.m_CopyFreq;
	CopyHis->ItemIndex = Log.m_LogSet.m_CopyHis;
	CopyName->Checked = Log.m_LogSet.m_CopyName;
	CopyQTH->Checked = Log.m_LogSet.m_CopyQTH;
	CopyREM->Checked = Log.m_LogSet.m_CopyREM;
	CopyQSL->Checked = Log.m_LogSet.m_CopyQSL;
	CBBackup->Checked = Log.m_LogSet.m_Backup;
	CBRemC->Checked = Log.m_LogSet.m_CopyREMB4;
	CBFSKNR->Checked = Log.m_LogSet.m_FSKNR;

	if( Log.m_LogSet.m_TimeZone != 'I' ){
		TimeZone->ItemIndex = 1;
	}
	else {
		TimeZone->ItemIndex = 0;
	}
	AutoSave->Checked = Log.m_LogSet.m_AutoSave;
	RGDupe->ItemIndex = Log.m_LogSet.m_CheckBand;

	THRTTY->Text = Log.m_LogSet.m_THRTTY;
	THSSTV->Text = Log.m_LogSet.m_THSSTV;
	THTZ->ItemIndex = Log.m_LogSet.m_THTZ;
	ClipRSTADIF->Checked = Log.m_LogSet.m_ClipRSTADIF;
	DateType->ItemIndex = Log.m_LogSet.m_DateType;
	AutoZone->Checked = sys.m_AutoTimeOffset;
	UDOffset->Position = short(sys.m_TimeOffset);
	UDMin->Position = short(sys.m_TimeOffsetMin);
	RGLink->ItemIndex = sys.m_LogLink;
	CBPoll->Checked = LogLink.IsPolling();
    CBPTT->Checked = LogLink.GetPTTEnabled();
	CBMMLink->ItemIndex = CBMMLink->Items->IndexOf(LogLink.GetItemName());
	if( m_MMList.GetCount() ){
		RGLink->Controls[2]->Enabled = TRUE;
		if( CBMMLink->ItemIndex < 0 ){
			CBMMLink->ItemIndex = 0;
		}
	}
	else {
		RGLink->Controls[2]->Enabled = FALSE;
	}
    CBTDelay->ItemIndex = sys.m_TempDelay;
	if( (PageIndex >= 0) && (PageIndex < Page->PageCount) ){
		if( Page->Pages[PageIndex]->TabVisible == FALSE ){
			PageIndex = 0;
		}
		Page->ActivePage = Page->Pages[PageIndex];
	}
	int r = FALSE;
	UpdateUI();
	m_DisEvent = 0;
	if( ShowModal() == IDOK ){
		Log.m_LogSet.m_UpperName = UpperName->Checked;
		Log.m_LogSet.m_UpperQTH = UpperQTH->Checked;
		Log.m_LogSet.m_UpperREM = UpperREM->Checked;
		Log.m_LogSet.m_UpperQSL = UpperQSL->Checked;

		Log.m_LogSet.m_DefMyRST = DefMyRST->Checked;

		Log.m_LogSet.m_CopyFreq = CopyFreq->ItemIndex;
		Log.m_LogSet.m_CopyHis = CopyHis->ItemIndex;
		Log.m_LogSet.m_CopyName = CopyName->Checked;
		Log.m_LogSet.m_CopyQTH = CopyQTH->Checked;
		Log.m_LogSet.m_CopyREM = CopyREM->Checked;
		Log.m_LogSet.m_CopyQSL = CopyQSL->Checked;
		Log.m_LogSet.m_CopyREMB4 = ( Font->Charset != SHIFTJIS_CHARSET ) ? 0 : CBRemC->Checked;

		if( TimeZone->ItemIndex ){
			Log.m_LogSet.m_TimeZone = 'Z';
		}
		else {
			Log.m_LogSet.m_TimeZone = 'I';
		}

		Log.m_LogSet.m_AutoSave = AutoSave->Checked;
		Log.m_LogSet.m_CheckBand = RGDupe->ItemIndex;

		Log.m_LogSet.m_THRTTY = THRTTY->Text;
		Log.m_LogSet.m_THSSTV = THSSTV->Text;
		Log.m_LogSet.m_THTZ = THTZ->ItemIndex;
		Log.m_LogSet.m_ClipRSTADIF = ClipRSTADIF->Checked;
		Log.m_LogSet.m_DateType = DateType->ItemIndex;
		Log.m_LogSet.m_Backup = CBBackup->Checked;
		Log.m_LogSet.m_FSKNR = CBFSKNR->Checked;

		sys.m_AutoTimeOffset = AutoZone->Checked;
		sys.m_TimeOffset = UDOffset->Position;
		sys.m_TimeOffsetMin = UDMin->Position;
		if( sys.m_AutoTimeOffset ){
			SetTimeOffsetInfo(sys.m_TimeOffset, sys.m_TimeOffsetMin);
		}
		sys.m_LogLink = RGLink->ItemIndex;
		LogLink.SetPolling(CBPoll->Checked);
        LogLink.SetPTTEnabled(CBPTT->Checked);
		if( CBMMLink->ItemIndex >= 0 ){
			LogLink.SetItemName(AnsiString(CBMMLink->Items->Strings[CBMMLink->ItemIndex]).c_str());	//ja7ude 0428
		}
	    sys.m_TempDelay = CBTDelay->ItemIndex;
		r = TRUE;
	}
	PageIndex = GetActiveIndex(Page);
	return r;
}
//---------------------------------------------------------------------
void __fastcall TLogSetDlg::ClearOffClick(TObject *Sender)
{
	UDOffset->Position = 0;
	UDMin->Position = 0;
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::AutoZoneClick(TObject *Sender)
{
	if( AutoZone->Checked ){
		int hour, min;
		SetTimeOffsetInfo(hour, min);
		UDOffset->Position = short(hour);
		UDMin->Position = short(min);
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::RGLinkClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateUI();
}
//---------------------------------------------------------------------------

