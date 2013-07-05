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

#include "MacroKey.h"
#include "ComLib.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TMacroKeyDlg *MacroKeyDlg;
MACKEY	mackeycom[]={
	{1, "%m","My callsign", NULL},
	{1, "%c","His/her callsign", NULL},
	{1, "%r","His/her RSV", NULL},
	{1, "%n","His/her name", NULL},
	{2, "%J","His/her name (+サン)", NULL},
	{1, "%q","His/her QTH", NULL},
	{1, "%s","My RSV", NULL},
	{1, "%R","His/her RSVのRSV部分のみ","Only the RSV part of his/her RSV"},
	{1, "%N","His/her RSVのコンテストナンバ部分のみ","Only the contest number part of his/her RSV"},
	{1, "%M","My RSVのコンテストナンバ部分のみ","Only the contest number part of my RSV"},
	{1, "%g","GOOD MORNING/AFTERNOON/EVENING", NULL},
	{1, "%f","GM/GA/GE", NULL},
	{1, "%D","UTC date (e.g., 2000-SEP-05)", NULL},
	{1, "%T","UTC time (e.g., 12:34)", NULL},
	{1, "%t","UTC time (e.g., 1234)", NULL},
	{1, "%L","Local date (e.g., 2000-SEP-05)", NULL},
	{1, "%U","Local time (e.g., 12:34)", NULL},
	{1, "%u","Local time (e.g., 1234)", NULL},
	{1, "%B","周波数", "frequency"},
	{1, "%b","バンド", "band"},
	{1, "%o","Note", NULL},
	{1, "%X","Receipt time(UTC) of the current image", NULL},
	{1, "%v","MMSSTV version", NULL},
	{1, "%V","Beta version", NULL},
	{0, NULL, NULL},
};
static int intPos;	//ja7ude 0521
static int TopPos;
//---------------------------------------------------------------------
__fastcall TMacroKeyDlg::TMacroKeyDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		CancelBtn->Caption = "Cancel";
		OKBtn->Caption = "Insert";
	}
}
//---------------------------------------------------------------------
void __fastcall TMacroKeyDlg::UpdateUI(int row)
{
	int r = row - 1;
	if( r >= 0 ){
		OKBtn->Enabled = TRUE;
	}
	else {
		OKBtn->Enabled = FALSE;
	}
}
//---------------------------------------------------------------------
int __fastcall TMacroKeyDlg::AddMacKey(MACKEY *mp, int n)
{
	for( ; mp->r; mp++){
		if( (mp->r == 2) && MsgEng ) continue;
		mackey[n] = *mp;
		n++;
	}
	return n;
}
//---------------------------------------------------------------------
int __fastcall TMacroKeyDlg::Execute(AnsiString &as)
{
	int n = 0;
	n = AddMacKey(mackeycom, n);
	Grid->RowCount = 1 + n;
	if( (intPos > 0) && (intPos < Grid->RowCount) ){	//ja7ude 0521
		Grid->Row = intPos;
		Grid->TopRow = TopPos;
	}
	UpdateUI(Grid->Row);
	int r = ShowModal();
	intPos = Grid->Row;	//ja7ude 0521
	TopPos = Grid->TopRow;
	if( r == IDOK ){
		if( intPos ){	//ja7ude 0521
			r = intPos - 1;
			as = mackey[r].pKey;
			return TRUE;
		}
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TMacroKeyDlg::GridDrawCell(TObject *Sender, int Col,
	  int Row, TRect &Rect, TGridDrawState State)
{
	char	bf[256];

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		LPCSTR pCom;
		switch(Col){
			case 0:
				strcpy(bf, mackey[Row].pKey);
				break;
			case 1:
				if( MsgEng ){
					pCom = mackey[Row].pEng;
					if( pCom == NULL ){
						pCom = mackey[Row].pJpn;
					}
				}
				else {
					pCom = mackey[Row].pJpn;
				}
				strcpy(bf, pCom);
				break;
		}
		Grid->Canvas->TextOut(X, Y, bf);
	}
	else {		// タイトル
		LPCSTR	_tt[]={
			"Key","Comments",
		};
		Grid->Canvas->TextOut(X, Y, _tt[Col]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMacroKeyDlg::GridSelectCell(TObject *Sender, int Col,
	  int Row, bool &CanSelect)
{
	UpdateUI(Row);
}
//---------------------------------------------------------------------------


