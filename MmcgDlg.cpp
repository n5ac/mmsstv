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

#include "MmcgDlg.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TMmcgDlgBox *MmcgDlgBox;
//---------------------------------------------------------------------
__fastcall TMmcgDlgBox::TMmcgDlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;

	if( MsgEng ){
		SBInc->Caption = "String";
		SBMask->Caption = "Area";
		CancelBtn->Caption = "Cancel";
	}

	m_DisEvent = 0;
}
//---------------------------------------------------------------------
void __fastcall TMmcgDlgBox::UpdateCaption(void)
{
	char bf[256];

	if( MsgEng ){
		if( mmcg.m_Call[0] ){
			sprintf(bf, "MMCG  [%s]  Find: %d", mmcg.m_Call, mmcg.m_FindCnt);
		}
		else {
			sprintf(bf, "MMCG  Find: %d", mmcg.m_FindCnt);
		}
	}
	else {
		if( mmcg.m_Call[0] ){
			sprintf(bf, "MMCG  [%s] Œó•â : %d", mmcg.m_Call, mmcg.m_FindCnt);
		}
		else {
			sprintf(bf, "MMCG  Œó•â: %d", mmcg.m_FindCnt);
		}
	}
	Caption = bf;
}

//---------------------------------------------------------------------
int __fastcall TMmcgDlgBox::Execute(AnsiString &call, AnsiString &qth, AnsiString &op)
{
	StrCopy(mmcg.m_Call, call.c_str(), MLCALL);
	mmcg.SetMask();
	SBMask->Down = TRUE;
	if( mmcg.m_mask == -1 ) SBMask->Enabled = FALSE;

	char bf[256];

	bf[0] = 0;
	if( isdigit(*op.c_str()) ){
		strcpy(bf, op.c_str());
	}
	else if( !qth.IsEmpty() ){
		strcpy(bf, qth.c_str());
	}
	m_DisEvent++;
	EditYomi->Text = bf;
	mmcg.Find(bf);
	UpdateCaption();
	m_DisEvent--;
	Grid->RowCount = mmcg.m_FindCnt ? mmcg.m_FindCnt + 1 : 2;
	if( ShowModal() == IDOK ){
		int n = Grid->Row - 1;
		if( (n >= 0)&&(n < mmcg.m_FindCnt) ){
			qth = mmcg.GetQTH(mmcg.m_fp[n]);
			op = mmcg.m_fp[n]->Code;
			return TRUE;
		}
	}
	return FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmcgDlgBox::GridDrawCell(TObject *Sender, int Col,
	  int Row, TRect &Rect, TGridDrawState State)
{
	char	bf[256];
	MMCG	*mp;

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		if( Row < mmcg.m_FindCnt ){
			mp = mmcg.m_fp[Row];
		}
		else {
			mp = NULL;
		}
		switch(Col){
			case 0:		// Code
				if( mp != NULL ) strcpy(bf, mp->Code);
				break;
			case 1:		// QTH
				if( mp != NULL ) strcpy(bf, mmcg.GetQTH(mp));
				break;
			case 2:		// Key
				if( mp != NULL ) strcpy(bf, mp->Key);
				break;
		}
		Grid->Canvas->TextOut(X, Y, bf);
	}
	else {		// ƒ^ƒCƒgƒ‹
		LPCSTR	_tt[]={
			"Code","QTH","Key",
		};
		Grid->Canvas->TextOut(X, Y, _tt[Col]);
	}

}
//---------------------------------------------------------------------------
void __fastcall TMmcgDlgBox::EditYomiChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	char bf[256];

	strcpy(bf, AnsiString(EditYomi->Text).c_str());	//ja7ude 0428
	mmcg.Find(bf);
	UpdateCaption();
	Grid->RowCount = mmcg.m_FindCnt ? mmcg.m_FindCnt + 1 : 2;
	Grid->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmcgDlgBox::EditYomiKeyPress(TObject *Sender, char &Key)
{
	if( Key == 0x20 ){
		EditYomi->Text = "";
		Key = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmcgDlgBox::EditYomiKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	switch(Key){
		case VK_UP:
			if( Grid->Row >= 2 ) Grid->Row--;
			Key = 0;
			break;
		case VK_DOWN:
			if( Grid->Row < Grid->RowCount - 1 ) Grid->Row++;
			Key = 0;
			break;
		case VK_HOME:
			Grid->Row = 1;
			Key = 0;
			break;
		case VK_END:
			Grid->Row = Grid->RowCount - 1;
			Key = 0;
			break;
		case VK_F1:
			SBInc->Down = SBInc->Down ? 0 : 1;
			SBIncClick(NULL);
			Key = 0;
			break;
		case VK_F2:
			SBMask->Down = SBMask->Down ? 0 : 1;
			SBMaskClick(NULL);
			Key = 0;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmcgDlgBox::EditYomiKeyUp(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	switch(Key){
		case VK_UP:
			Key = 0;
			break;
		case VK_DOWN:
			Key = 0;
			break;
		case VK_HOME:
			Key = 0;
			break;
		case VK_END:
			Key = 0;
			break;
		case VK_F1:
			Key = 0;
			break;
		case VK_F2:
			Key = 0;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmcgDlgBox::SBMaskClick(TObject *Sender)
{
	if( SBMask->Down ){
		mmcg.SetMask();
	}
	else {
		mmcg.m_mask = -1;
	}
	EditYomiChange(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmcgDlgBox::SBIncClick(TObject *Sender)
{
	if( SBInc->Down ){
		mmcg.m_sinc = 1;
	}
	else {
		mmcg.m_sinc = 0;
	}
	EditYomiChange(NULL);
}
//---------------------------------------------------------------------------


