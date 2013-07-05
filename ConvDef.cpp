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

#include "ConvDef.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TConvDefDlg *ConvDefDlg;
//---------------------------------------------------------------------
__fastcall TConvDefDlg::TConvDefDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Define Conversion";
		ConvSel->Caption = "Delimiter Type";
		ConvSel->Items->Strings[0] = "comma(,)";
		ConvSel->Items->Strings[1] = "TAB";
		ConvSel->Items->Strings[2] = "NONE";
		LConv->Caption = "Conv.";
		CheckDBL->Caption = "Double";
		GrpRef->Caption = "Ref.";
		SBBack->Caption = "<";
		SBNext->Caption = ">";
		CancelBtn->Caption = "Cancel";
		InitBtn->Caption = "All Del";
		UpBtn->Caption = "Up";
		DownBtn->Caption = "Down";
		InitMax->Caption = "Init Max";
	}

	Grid->RowCount = TEXTCONVMAX + 1;
	m_Mode = 0;
	m_Bgn = m_End = m_Cur = 0;
	m_DisEvent = 0;
}
//---------------------------------------------------------------------
void __fastcall TConvDefDlg::UpdateUI(int n)
{
	SBBack->Enabled = m_Cur > m_Bgn ? TRUE : FALSE;
	SBNext->Enabled = m_Cur < m_End ? TRUE : FALSE;
	UpBtn->Enabled = (n > 0) ? TRUE : FALSE;
	DownBtn->Enabled = (n < TEXTCONVMAX - 1) ? TRUE : FALSE;
	CheckDBL->Enabled = m_Mode && (ConvSel->ItemIndex < 2);
}
//---------------------------------------------------------------------
void __fastcall TConvDefDlg::GridDrawCell(TObject *Sender, int Col,
	  int Row, TRect &Rect, TGridDrawState State)
{
	char	bf[1024];
	SDMMLOG	sd;

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		switch(Col){
			case 0:		// Key
				strcpy(bf, Conv[Row].Key.c_str());
				break;
			case 1:		// Size
				if( !Conv[Row].Key.IsEmpty() && (Conv[Row].Key != "%EOD") ){
					sprintf(bf, "%u", Conv[Row].w);
				}
				break;
			case 2:		// Data
				if( m_Mode ){
					if( !Conv[Row].Key.IsEmpty() && (Conv[Row].Key != "%EOD") ){
						Log.GetData(&sd, m_Cur);
						MMLOG2Text(bf, &sd, Conv[Row].Key);
					}
				}
				else {
					strcpy(bf, m_line[Row].c_str());
				}
				break;
		}
		Grid->Canvas->TextOut(X, Y, bf);
	}
	else if( MsgEng ){
		LPCSTR	_tte[]={
			"Conv.","Max","Ref.",
		};
		Grid->Canvas->TextOut(X, Y, _tte[Col]);
	}
	else {		// タイトル
		LPCSTR	_tt[]={
			"変換式","Max","参照",
		};
		Grid->Canvas->TextOut(X, Y, _tt[Col]);
	}
}
//---------------------------------------------------------------------
void __fastcall TConvDefDlg::SetupComBox(void)
{
	m_DisEvent++;
	ConvText->Clear();
	int i;
	for( i = 0; ConvTbl[i] != NULL; i++ ){
		ConvText->Items->Add(ConvTbl[i]);
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------
int __fastcall TConvDefDlg::Execute(TCONV *tp, int &delm, int &utc, int &dbl, int b, int e)
{
	m_DisEvent++;
	int i;
	for( i = 0; i < TEXTCONVMAX; i++ ){
		Conv[i].Key = tp[i].Key;
		Conv[i].w = tp[i].w;
	}
	m_Mode = 1;
	m_Cur = m_Bgn = b;
	m_End = e;
	CheckUTC->Checked = utc;
	ConvSel->ItemIndex = delm;
	CheckDBL->Checked = dbl;
	SetupComBox();
	ConvText->Text = Conv[0].Key;
	UpdateUI(Grid->Row - 1);
	m_DisEvent--;
	if( ShowModal() == IDOK ){
		for( i = 0; i < TEXTCONVMAX; i++ ){
			tp[i].Key = Conv[i].Key;
			tp[i].w = Conv[i].w;
		}
		delm = ConvSel->ItemIndex;
		utc = CheckUTC->Checked;
		dbl = CheckDBL->Checked;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
int __fastcall TConvDefDlg::Execute(TCONV *tp, int &delm, int &utc, int &dbl, LPCSTR pName)
{
	m_DisEvent++;
	int i;
	for( i = 0; i < TEXTCONVMAX; i++ ){
		Conv[i].Key = tp[i].Key;
		Conv[i].w = tp[i].w;
	}
	m_Mode = 0;
	CheckUTC->Checked = utc;
	ConvSel->ItemIndex = delm;
	CheckDBL->Checked = dbl;
	SetupComBox();
	ConvText->Text = Conv[0].Key;
	LoadText(pName);
	UpdateUI(Grid->Row - 1);
	UpdateLine();
	m_DisEvent--;
	if( ShowModal() == IDOK ){
		for( i = 0; i < TEXTCONVMAX; i++ ){
			tp[i].Key = Conv[i].Key;
			tp[i].w = Conv[i].w;
		}
		delm = ConvSel->ItemIndex;
		utc = CheckUTC->Checked;
		dbl = CheckDBL->Checked;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TConvDefDlg::LoadText(LPCSTR pName)
{
	FILE *fp;
	char	bf[2048];

	m_Cur = m_Bgn = m_End = 0;
	if( (fp = fopen(pName, "rt")) != NULL ){
		while(!feof(fp)){
			if( fgets(bf, sizeof(bf), fp) != NULL ){
				ClipLF(bf);
				m_text[m_End] = bf;
				m_End++;
				if( m_End >= PREREADMAX ) break;
			}
		}
		fclose(fp);
	}
	if( m_End ) m_End--;
}
//---------------------------------------------------------------------
void __fastcall TConvDefDlg::UpdateLine(void)
{
	char	rbf[2048];

	StrCopy(rbf, m_text[m_Cur].c_str(), 2043);

	char	bf[512];
	char	dlm = ConvSel->ItemIndex ? TAB : ',';
	LPSTR	p, t;

	int i;
	p = rbf;
	for( i = 0; i < TEXTCONVMAX; i++ ){
		if( ConvSel->ItemIndex == 2 ){
			StrCopy(bf, p, Conv[i].w);
			p += strlen(bf);
			t = bf;
		}
		else {
			p = StrDlm(t, p, dlm);
		}
		clipsp(t);
		t = SkipSpace(t);
		m_line[i] = t;
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::ConvTextChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	if( Grid->Row ){
		int Row = Grid->Row - 1;
		Conv[Row].Key = ConvText->Text;
		Grid->Invalidate();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::GridSelectCell(TObject *Sender, int Col,
	  int Row, bool &CanSelect)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	if( Row ){
		Row--;
		ConvText->Text = Conv[Row].Key;
		UpdateUI(Row);
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::SBBackClick(TObject *Sender)
{
	if( m_Cur > m_Bgn ){
		m_Cur--;
		UpdateUI(Grid->Row - 1);
		if( !m_Mode ) UpdateLine();
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::SBNextClick(TObject *Sender)
{
	if( m_Cur < m_End ){
		m_Cur++;
		UpdateUI(Grid->Row - 1);
		if( !m_Mode ) UpdateLine();
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::GridGetEditText(TObject *Sender, int ACol,
	  int ARow, AnsiString &Value)
{
	char bf[512];

	if( ARow ) ARow--;
	bf[0] = 0;
	switch(ACol){
		case 0:		// Key
			strcpy(bf, Conv[ARow].Key.c_str());
			break;
		case 1:		// Size
			if( !Conv[ARow].Key.IsEmpty() && (Conv[ARow].Key != "%EOD") ){
				sprintf(bf, "%u", Conv[ARow].w);
			}
			break;
		case 2:		// Data
			break;
	}
	Value = bf;
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::GridSetEditText(TObject *Sender, int ACol,
	  int ARow, const AnsiString Value)
{
	int	d;

	if( ARow ) ARow--;
	switch(ACol){
		case 0:		// Key
			Conv[ARow].Key = Value;
			Grid->Invalidate();
			break;
		case 1:		// Size
			sscanf(Value.c_str(), "%u", &d);
			if( (d >= 0) && (d<=256) ){
				Conv[ARow].w = d;
				if( !m_Mode ){
					UpdateLine();
					Grid->Invalidate();
				}
			}
			break;
		case 2:		// Data
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::InitBtnClick(TObject *Sender)
{
	if( YesNoMB( MsgEng ? "Delete All setup. Are you sure?":"すべての定義を消去しますか？" ) == IDYES ){
		for( int i = 0; i < TEXTCONVMAX; i++ ){
			Conv[i].Key = "";
			Conv[i].w = 0;
		}
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::UpBtnClick(TObject *Sender)
{
	int n = Grid->Row - 1;
	if( n ){
		TCONV	td = Conv[n];
		Conv[n] = Conv[n-1];
		Conv[n-1] = td;
		Grid->Row--;
		UpdateUI(Grid->Row - 1);
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::DownBtnClick(TObject *Sender)
{
	int n = Grid->Row - 1;

	if( n < (TEXTCONVMAX - 1) ){
		TCONV	td = Conv[n];
		Conv[n] = Conv[n+1];
		Conv[n+1] = td;
		Grid->Row++;
		UpdateUI(Grid->Row - 1);
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::InitMaxClick(TObject *Sender)
{
	if( YesNoMB( MsgEng ? "Delete All Length. Are you sure?":"すべての文字列長の制限を無くしますか？" ) == IDYES ){
		for( int i = 0; i < TEXTCONVMAX; i++ ){
			Conv[i].w = 0;
		}
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TConvDefDlg::ConvSelClick(TObject *Sender)
{
	UpdateUI(Grid->Row-1);
	if( !m_Mode ){
		UpdateLine();
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

