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

#include "QsoDlg.h"
#include "Country.h"
#include "MmcgDlg.h"
#include "Main.h"
#include "TextEdit.h"
#include "LogPic.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TQSODlgBox *QSODlgBox;
//---------------------------------------------------------------------
__fastcall TQSODlgBox::TQSODlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	LStat->Font->Name = ((TForm *)AOwner)->Font->Name;
	LStat->Font->Charset = ((TForm *)AOwner)->Font->Charset;
	m_fp = NULL;

	EditHis->Items->Assign(Mmsstv->HisRST->Items);
	EditMy->Items->Assign(Mmsstv->MyRST->Items);
	if( MsgEng ){
		Grid->Font->Name = ((TForm *)AOwner)->Font->Name;
		Grid->Font->Charset = ((TForm *)AOwner)->Font->Charset;
		Caption = "QSO Data";
		LogPicBtn->Caption = "Pic.";
		CancelBtn->Caption = "Close";
		OKBtn->Caption = "Write";
	}
	switch(Log.m_LogSet.m_TimeZone){
		case 'I':
			if( lcid == LANG_KOREAN ){
				LTimeZone->Caption = "KST";
			}
			else {
				LTimeZone->Caption = "JST";
			}
			break;
		default:
			LTimeZone->Caption = "UTC";
			break;
	}
	if( Log.m_LogSet.m_DateType & 1 ){
		Grid->ColWidths[0] = (Grid->ColWidths[0] * 10) / 8;
	}
	m_EditFlag = 0;
}

void __fastcall TQSODlgBox::UpdateBtn(void)
{
	OKBtn->Enabled = m_EditFlag;
	LogPicBtn->Enabled = !EditQSL->Text.IsEmpty() && !sys.m_AutoSaveDir.IsEmpty() && IsLogPic(AnsiString(EditQSL->Text).c_str());	//ja7ude 0428
}

void __fastcall TQSODlgBox::SetCountry(void)
{
	m_Country = "";

	if( !Cty.IsData() ) return;
	LPCSTR	p = ClipCC(m_fp->GetText());
	if( !(*p) ) return;

	SYSTEMTIME	now;
	GetUTC(&now);

	WORD tim = WORD((now.wHour * 60 + now.wMinute) * 30 + now.wSecond/2);

	int		n;
	if( (n = Cty.GetNoP(p))!=0 ){
		CTL *cp = Cty.GetCTL(n-1);
		char bf[256];
		char tbf[5];
		tbf[0] = 0;
		if( cp->TD != NULL ){
			tim = AdjustRolTimeUTC(tim, *cp->TD);
			if( tim ){
				tim /= WORD(30);
				sprintf(tbf, "%02u%02u%c", tim / 60, tim % 60, *cp->TD);
			}
		}
		sprintf(bf, " %s/%s %s %s",
			(cp->Name!=NULL) ? cp->Name : "",
			(cp->Cont!=NULL) ? cp->Cont : "",
			tbf,
			(cp->QTH!=NULL) ? cp->QTH : ""
		);
		m_Country = bf;
	}
}

int __fastcall TQSODlgBox::Execute(CLogFind *fp, SDMMLOG *sp, int n)
{
	m_fp = fp;
	UpdateTextData(sp, n);
	Grid->RowCount = fp->GetCount() ? fp->GetCount() + 1 : 2;
//	Grid->TopRow = 1;
	Grid->Row = 1;
	Grid->Enabled = FALSE;
	for( int i = 0; i < fp->GetCount(); i++ ){
		if( n == fp->pFindTbl[i] ){
			Grid->Enabled = TRUE;
			Grid->Row = i + 1;
			Grid->TopRow = (i > 4) ? Grid->Row - 4 : 1;
			break;
		}
	}
	SetCountry();
	char bf[512];
	strcpy(bf, MsgEng ? "Initial Data":"QSOデータ初期値");
	if( Grid->Enabled == TRUE ){
		if( sp->btime ){
			sprintf(bf, "[%s]", fp->GetText());
			strcat(bf, m_Country.c_str());
		}
		else if( *fp->GetText() ){
			sprintf(bf, "[%s]", fp->GetText());
			strcat(bf, m_Country.c_str());
		}
	}
	Caption = bf;
	UpdateStat();
	m_EditFlag = 0;
	UpdateBtn();
	m_CurNo = n;
	if( ShowModal() == IDOK ){
		if( m_EditFlag ){
			UpdateCurData(&m_sd);
			Log.PutData(&m_sd, m_CurNo);
		}
	}
	if( m_CurNo == n ){
		memcpy(sp, &m_sd, sizeof(SDMMLOG));
	}
	else {
		Log.GetData(sp, n);
	}
	return TRUE;
}

int __fastcall TQSODlgBox::ShowFind(CLogFind *fp)
{
	m_fp = fp;
	if( fp->GetCount() ){
		Log.GetData(&m_sd, fp->pFindTbl[0]);
		m_CurNo = fp->pFindTbl[0];
	}
	else {
		memset(&m_sd, 0, sizeof(SDMMLOG));
		OKBtn->Enabled = FALSE;
	}
	SetCountry();
	char bf[512];
	UpdateTextData(&m_sd, fp->pFindTbl[0]);
	Grid->RowCount = fp->GetCount() ? fp->GetCount() + 1 : 2;
	Grid->TopRow = 1;
	Grid->Row = 1;
	sprintf(bf, "[%s]", fp->GetText());
	strcat(bf, m_Country.c_str());
	Caption = bf;
	UpdateStat();
	m_EditFlag = 0;
	UpdateBtn();
	if( ShowModal() == IDOK ){
		if( m_EditFlag ){
			UpdateCurData(&m_sd);
			Log.PutData(&m_sd, m_CurNo);
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void __fastcall TQSODlgBox::UpdateTextData(SDMMLOG *sp, int n)
{
	m_CurNo = n;
	memcpy(&m_sd, sp, sizeof(SDMMLOG));
	UpdateTextData(&m_sd);
}

void __fastcall TQSODlgBox::UpdateTextData(SDMMLOG *sp)
{
	SDMMLOG	sd;
	memcpy(&sd, sp, sizeof(SDMMLOG));
	if( Log.m_LogSet.m_TimeZone != 'I' ){
		JSTtoUTC(&sd);
	}
	EditDate->Text = Log.GetDateString(&sd);
	EditBgn->Text = Log.GetTimeString(sd.btime);
	EditEnd->Text = Log.GetTimeString(sd.etime);
	EditCall->Text = sp->call;
	EditName->Text = sp->name;
	EditQTH->Text = sp->qth;
	EditHis->Text = sp->ur;
	EditMy->Text = sp->my;
	EditBand->Text = Log.GetFreqString(sp->band, sp->fq);
	EditMode->Text = Log.GetModeString(sp->mode);
	EditRem->Text = sp->rem;
	EditQSL->Text = sp->qsl;
	if( sp->env ){
		EditEnv->Text = sp->env;
	}
	else {
		EditEnv->Text = "";
	}
	EditOpt1->Text = Log.GetOptStr(0, sp);
	EditOpt2->Text = Log.GetOptStr(1, sp);
	EditUsr1->Text = Log.GetOptStr(2, sp);
	EditUsr2->Text = Log.GetOptStr(3, sp);
	EditS->Text = sp->send;
	EditR->Text = sp->recv;
	EditM->Text = sp->cq;
	EditPow->Text = sp->pow;
	m_EditFlag = 0;
	UpdateBtn();
}

void __fastcall TQSODlgBox::UpdateCurData(SDMMLOG *sp)
{
	int y, m, d, h;

	switch(Log.m_LogSet.m_DateType){
		case 2:			// dd-mm-yy
		case 3:			// dd-mm-yyyy
			if( sscanf(AnsiString(EditDate->Text).c_str(), "%u.%u.%u", &d, &m, &y) == 3 ){	//ja7ude 0428
				sp->year = BYTE(y % 100);
				sp->date = WORD(m * 100 + d);
			}
			break;
		case 4:			// mm-dd-yy
		case 5:			// mm-dd-yyyy
			if( sscanf(AnsiString(EditDate->Text).c_str(), "%u.%u.%u", &m, &d, &y) == 3 ){	//ja7ude 0428
				sp->year = BYTE(y % 100);
				sp->date = WORD(m * 100 + d);
			}
			break;
		default:		// yy-mm-dd, yyyy-mm-dd
			if( sscanf(AnsiString(EditDate->Text).c_str(), "%u.%u.%u", &y, &m, &d) == 3 ){	//ja7ude 0428
				sp->year = BYTE(y % 100);
				sp->date = WORD(m * 100 + d);
			}
			break;
	}
	if( !EditBgn->Text.IsEmpty() ){
		if( sscanf(AnsiString(EditBgn->Text).c_str(), "%u", &d) == 1){	//ja7ude 0428
			h = d / 100;
			m = d % 100;
			if( (sp->btime / 30) != (h * 60 + m) ){
				sp->btime = WORD((h * 60 + m) * 30 + 1);
			}
		}
	}
	if( !EditEnd->Text.IsEmpty() ){
		if( sscanf(AnsiString(EditEnd->Text).c_str(), "%u", &d) == 1){	//ja7ude 0428
			h = d / 100;
			m = d % 100;
			if( (sp->etime / 30) != (h * 60 + m) ){
				sp->etime = WORD((h * 60 + m) * 30 + 1);
			}
		}
	}
	StrCopy(sp->call, AnsiString(EditCall->Text).c_str(), MLCALL);	//ja7ude 0428
	jstrupr(sp->call);
	StrCopy(sp->name, AnsiString(EditName->Text).c_str(), MLNAME);	//ja7ude 0428
	if( Log.m_LogSet.m_UpperName ) jstrupr(sp->name);
	StrCopy(sp->qth, AnsiString(EditQTH->Text).c_str(), MLQTH);	//ja7ude 0428
	if( Log.m_LogSet.m_UpperQTH ) jstrupr(sp->qth);
	StrCopy(sp->ur, AnsiString(EditHis->Text).c_str(), MLRST);	//ja7ude 0428
	jstrupr(sp->ur);
	StrCopy(sp->my, AnsiString(EditMy->Text).c_str(), MLRST);	//ja7ude 0428
	jstrupr(sp->my);
	Log.SetFreq(sp, AnsiString(EditBand->Text).c_str());	//ja7ude 0428
	Log.SetMode(sp, AnsiString(EditMode->Text).c_str());	//ja7ude 0428
	StrCopy(sp->rem, AnsiString(EditRem->Text).c_str(), MLREM);	//ja7ude 0428
	if( Log.m_LogSet.m_UpperREM ) jstrupr(sp->rem);
	StrCopy(sp->qsl, AnsiString(EditQSL->Text).c_str(), MLQSL);	//ja7ude 0428
	if( Log.m_LogSet.m_UpperQSL ) jstrupr(sp->qsl);
	if( sscanf(AnsiString(EditEnv->Text).c_str(), "%u", &d) == 1){	//ja7ude 0428
		sp->env = WORD(d);
	}
	else {
		sp->env = 0;
	}
	Log.SetOptStr(0, sp, AnsiString(EditOpt1->Text).c_str());	//ja7ude 0428
	Log.SetOptStr(1, sp, AnsiString(EditOpt2->Text).c_str());	//ja7ude 0428
	Log.SetOptStr(2, sp, AnsiString(EditUsr1->Text).c_str());	//ja7ude 0428
	Log.SetOptStr(3, sp, AnsiString(EditUsr2->Text).c_str());	//ja7ude 0428
	sp->send = *EditS->Text.c_str();
	sp->recv = *EditR->Text.c_str();
	sp->cq = *EditM->Text.c_str();
	sp->cq = char(toupper(sp->cq));
	StrCopy(sp->pow, AnsiString(EditPow->Text).c_str(), MLPOW);	//ja7ude 0428
	jstrupr(sp->pow);
	if( Log.m_LogSet.m_TimeZone != 'I' ){
		UTCtoJST(sp);
	}
}
//---------------------------------------------------------------------
void __fastcall TQSODlgBox::UpdateStat(void)
{
	char bf[256];

	int cmp1max = m_fp->m_FindCmp1Max;
	int cmp2max = m_fp->m_FindCmp2Max;
	if( cmp2max && (m_fp->pFindTbl[0] == Log.m_CurNo) ){
		cmp1max--;
		cmp2max--;
	}
	if( MsgEng ){
		if( !cmp2max ){
			strcpy(bf, "No Data");
		}
		else if( cmp1max != cmp2max ){
			sprintf(bf, "Perfect: %u  SameCall: %u", cmp1max, cmp2max - cmp1max);
		}
		else {
			sprintf(bf, "Perfect: %u", cmp1max );
		}
	}
	else {
		if( !cmp2max ){
			strcpy(bf, "一致なし");
		}
		else if( cmp1max != cmp2max ){
			sprintf(bf, "完全一致: %u件  同一コール: %u件", cmp1max, cmp2max - cmp1max);
		}
		else {
			sprintf(bf, "完全一致: %u件", cmp1max );
		}
	}
	LStat->Caption = bf;
}
//---------------------------------------------------------------------#if 0
void __fastcall TQSODlgBox::GridDrawCell(TObject *Sender, int Col, int Row,
	  TRect &Rect, TGridDrawState State)
{
	char	bf[256];
	SDMMLOG	sd;

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		if( Row < m_fp->GetCount() ){
			Log.GetData(&sd, m_fp->pFindTbl[Row]);
			if( ((Row+1) != Grid->Row)||(Col != Grid->Col) ){
				if( m_fp->pFindTbl[Row] == Log.m_CurNo ){
					Grid->Canvas->Font->Color = clLtGray;
				}
				else if( Row < m_fp->m_FindCmp1Max ){
					Grid->Canvas->Font->Color = clRed;
				}
				else if( Row < m_fp->m_FindCmp2Max ){
					Grid->Canvas->Font->Color = clGreen;
				}
			}
			if( Log.m_LogSet.m_TimeZone != 'I' ){
				JSTtoUTC(&sd);
			}
		}
		else {
			memset(&sd, 0, sizeof(SDMMLOG));
		}
		switch(Col){
			case 0:		// Date
				strcpy(bf, Log.GetDateString(&sd));
				break;
			case 1:		// Time
				strcpy(bf, Log.GetTimeString(sd.btime));
				break;
			case 2:		// Call
				strcpy(bf, sd.call);
				break;
			case 3:		// M
				bf[0] = sd.cq;
				bf[1] = 0;
				break;
			case 4:		// HisRST
				strcpy(bf, sd.ur);
				break;
			case 5:		// MyRST
				strcpy(bf, sd.my);
				break;
			case 6:		// Band
				strcpy(bf, Log.GetFreqString(sd.band, sd.fq));
				break;
			case 7:		// Mode
				strcpy(bf, Log.GetModeString(sd.mode));
				break;
			case 8:		// Pow
				strcpy(bf, sd.pow);
				break;
			case 9:		// Name
				strcpy(bf, sd.name);
				break;
			case 10:	// QTH
				strcpy(bf, sd.qth);
				break;
			case 11:	// S
				bf[0] = sd.send;
				bf[1] = 0;
				break;
			case 12:	// R
				bf[0] = sd.recv;
				bf[1] = 0;
				break;
			case 13:	// REM
				strcpy(bf, sd.rem);
				break;
			case 14:	// QSL
				strcpy(bf, sd.qsl);
				break;
			case 15:	// etime;
				strcpy(bf, Log.GetTimeString(sd.etime));
				break;
			case 16:	// Env
				if( sd.env ) sprintf(bf, "%u", sd.env);
				break;
			case 17:	// Opt1
				strcpy(bf, sd.opt1);
				break;
			case 18:	// Opt2
				strcpy(bf, sd.opt2);
				break;
			case 19:	// Usr1
				break;
			case 20:	// Usr2
				break;
		}
		Grid->Canvas->TextRect(Rect, X, Y, bf);
	}
	else {		// タイトル
		LPCSTR	_tt[]={
			"Date","JST","Call","M", "His","My","Band","Mode","Pow","Name","QTH","S","R",
			"Note","QSL", "End", "Env", "Opt1", "Opt2", "Usr1", "Usr2",
		};
		if( Col == 1 ){
			Grid->Canvas->TextRect(Rect, X, Y, LTimeZone->Caption);
		}
		else {
			Grid->Canvas->TextRect(Rect, X, Y, _tt[Col]);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TQSODlgBox::GridSelectCell(TObject *Sender, int Col,
	  int Row, bool &CanSelect)
{
	int n = Row - 1;
	if( n < m_fp->GetCount() ){
		if( m_CurNo != m_fp->pFindTbl[n] ){
			if( m_EditFlag ){
				UpdateCurData(&m_sd);
				Log.PutData(&m_sd, m_CurNo);
				m_EditFlag = 0;
			}
			m_CurNo = m_fp->pFindTbl[n];
			Log.GetData(&m_sd, m_CurNo);
			UpdateTextData(&m_sd, m_CurNo);
			Grid->Invalidate();
			UpdateBtn();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TQSODlgBox::SBMMCGClick(TObject *Sender)
{
	TMmcgDlgBox *pBox = new TMmcgDlgBox(this);

	AnsiString	qth = EditQTH->Text;
	AnsiString	opt = EditOpt1->Text;
	AnsiString  call = EditCall->Text;
	if( pBox->Execute(call, qth, opt) == TRUE ){	//ja7ude 0428
		EditQTH->Text = qth;
		EditOpt1->Text = opt;
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TQSODlgBox::FormKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	if( Key == VK_F7 ){
		Key = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TQSODlgBox::FormKeyUp(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	if( Key == VK_F7 ){
		SBMMCGClick(NULL);
		Key = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TQSODlgBox::EditDateChange(TObject *Sender)
{
	m_EditFlag = 1;
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TQSODlgBox::LogPicBtnClick(TObject *Sender)
{
	TLogPicDlg *pBox = new TLogPicDlg(this);
	if( !EditCall->Text.IsEmpty() ){
		char bf[128];
		sprintf(bf, "%s on %s %s",
			EditCall->Text.c_str(),
			EditDate->Text.c_str(),
			EditBgn->Text.c_str()
		);
		pBox->Caption = bf;
	}
	pBox->Execute(AnsiString(EditQSL->Text).c_str());	//ja7ude 0428
	delete pBox;
}
//---------------------------------------------------------------------------


