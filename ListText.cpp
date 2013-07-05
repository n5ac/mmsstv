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

#include "ComLib.h"
#include "Main.h"
#include "ListText.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
static int SaveTopRow;
static int SaveRow;
//---------------------------------------------------------------------
__fastcall TListTextDlg::TListTextDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	MultProc();
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Defined text";
		CancelBtn->Caption = "Cancel";
		DelBtn->Caption = "Delete";
		EditBtn->Caption = "Edit";
		UpBtn->Caption = "Up";
		DwnBtn->Caption = "Down";
		m_Jpn = 0;
	}
	else {
		m_Jpn = 1;
	}
	pList = NULL;
	MultProc();
}
//---------------------------------------------------------------------
void __fastcall TListTextDlg::UpdateUI(void)
{
	if( Grid->Row && (Grid->Row <= pList->m_Cnt) ){
		DelBtn->Enabled = TRUE;
		OKBtn->Enabled = TRUE;
	}
	else {
		DelBtn->Enabled = FALSE;
		OKBtn->Enabled = FALSE;
	}
	EditBtn->Enabled = Grid->Row ? TRUE : FALSE;
	UpBtn->Enabled = (Grid->Row > 1) && (Grid->Row <= pList->m_Cnt) ? TRUE : FALSE;
	DwnBtn->Enabled = (Grid->Row <= (pList->m_Cnt - 1)) ? TRUE : FALSE;
}
//---------------------------------------------------------------------
CDraw *__fastcall TListTextDlg::Execute(CDrawGroup *pg, int pos)
{
	pList = pg;
	Grid->RowCount = pList->m_Cnt + 2;
	if( pos < 0 ){
		Grid->Row = Grid->RowCount - 2;
		int n = Grid->Row - 11;
		if( n < 1 ) n = 1;
		Grid->TopRow = n;
	}
	else if( SaveRow && SaveTopRow ){
		Grid->TopRow = SaveTopRow;
		Grid->Row = SaveRow;
	}
	UpdateUI();
	MultProc();
	int r = ShowModal();
	MultProc();
	SaveTopRow = Grid->TopRow;
	SaveRow = Grid->Row;
	if( r == IDOK ){
		return (CDraw *)pList->pBase[Grid->Row - 1];
	}
	return NULL;
}
//---------------------------------------------------------------------
void __fastcall TListTextDlg::DrawGrade(CDrawText *pItem, TRect &Rect)
{
	TRect rc = Rect;
	rc.Top += 2;
	rc.Bottom -= 2;
	int xw = (Rect.Right - Rect.Left)/4;
	int yw = (Rect.Bottom - Rect.Top)/4;
	switch(pItem->m_Grade){
		case 0:
			Grid->Canvas->Brush->Color = pItem->m_Col1;
			Grid->Canvas->FillRect(rc);
			break;
		case 1:
			rc.Right =  rc.Left + xw;
			Grid->Canvas->Brush->Color = pItem->m_Col1;
			Grid->Canvas->FillRect(rc);
			rc.Left += xw;
			rc.Right += xw;
			Grid->Canvas->Brush->Color = pItem->m_Col2;
			Grid->Canvas->FillRect(rc);
			rc.Left += xw;
			rc.Right += xw;
			Grid->Canvas->Brush->Color = pItem->m_Col3;
			Grid->Canvas->FillRect(rc);
			rc.Left += xw;
			rc.Right += xw;
			Grid->Canvas->Brush->Color = pItem->m_Col4;
			Grid->Canvas->FillRect(rc);
			break;
		case 2:
			rc.Bottom = rc.Top + yw;
			Grid->Canvas->Brush->Color = pItem->m_Col1;
			Grid->Canvas->FillRect(rc);
			rc.Top += yw;
			rc.Bottom += yw;
			Grid->Canvas->Brush->Color = pItem->m_Col2;
			Grid->Canvas->FillRect(rc);
			rc.Top += yw;
			rc.Bottom += yw;
			Grid->Canvas->Brush->Color = pItem->m_Col3;
			Grid->Canvas->FillRect(rc);
			rc.Top += yw;
			rc.Bottom += yw;
			Grid->Canvas->Brush->Color = pItem->m_Col4;
			Grid->Canvas->FillRect(rc);
			break;
	}
}
//---------------------------------------------------------------------
void __fastcall TListTextDlg::DrawShadow(CDrawText *pItem, TRect &Rect)
{
	LPCSTR	_tt[2][9]={
		{"None","Siege", "Shadow1", "Shadow2", "Shadow3", "Shadow4", "Shadow5", "Shadow6", "?",},
		{"‚È‚µ","‰Žæ‚è", "‰e1", "‰e2","‰e3","‰e4","‰e5","‰e6","?"},
	};
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;
	TRect rc = Rect;
	rc.Top += 2;
	rc.Bottom -= 2;
	rc.Left = rc.Right - (Rect.Right - Rect.Left)/4;
	int n = pItem->m_Shadow;
	if( (n < 0) || (n > 8) ) n = 8;
	Grid->Canvas->TextOut(X, Y, _tt[m_Jpn][n]);
	if( pItem->m_Shadow ){
		Grid->Canvas->Brush->Color = pItem->m_ColS;
		Grid->Canvas->FillRect(rc);
	}
}
//---------------------------------------------------------------------
static void __fastcall ConvCRLF(LPSTR s)
{
	LPSTR p = s;
    for( ; *p; p++ ){
		if( (*p == CR) || (*p == LF) ){
			*p = '\\';
        }
    }
}
//---------------------------------------------------------------------
void __fastcall TListTextDlg::GridDrawCell(TObject *Sender, int Col,
	  int Row, TRect &Rect, TGridDrawState State)
{
	if( pList == NULL ) return;

	char	bf[256];

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		int ec = 4;
		if( Row < pList->m_Cnt ){
//			strcpy(bf, "-");
			bf[0] = 0;
			LPCSTR p;
			CDraw *pItem = pList->pBase[Row];
			CDrawLib *pLib = (CDrawLib *)pItem;
			CDrawText *pText = (CDrawText *)pItem;
			if( (pItem->m_Command != CM_TEXT) && (pItem->m_Command != CM_LIB) ) return;
			if( pItem->m_Command == CM_LIB ) ec = 6;
			switch(Col){
				case 0:		// No.
					sprintf(bf, "%u%s", Row+1, (pItem->m_Command == CM_LIB)?" c":"");
					break;
				case 1:		// text
					p = (pItem->m_Command == CM_TEXT) ? pText->m_Text.c_str() : pLib->GetItemText();
					StrCopy(bf, p, sizeof(bf)-1);
					ConvCRLF(bf);
					break;
				case 2:		// text
					p = (pItem->m_Command == CM_TEXT) ? pText->m_Text.c_str() : pLib->GetItemText();
					Mmsstv->MacroText(bf, p, sizeof(bf)-1);
					ConvCRLF(bf);
					break;
				case 3:		// text
					if( pItem->m_Command == CM_TEXT ){
						sprintf(bf, "%u", pText->pFont->Size);
					}
					break;
				case 4:		// color
					if( pItem->m_Command == CM_TEXT ){
						DrawGrade(pText, Rect);
					}
					break;
				case 5:     // Shadow
					if( pItem->m_Command == CM_TEXT ){
						DrawShadow(pText, Rect);
					}
					break;
			}
			if( pItem->m_Command == CM_TEXT ){
				if( ((Col == 1) || (Col == 2)) && (pText->pFont != NULL) ){
					Grid->Canvas->Font->Name = pText->pFont->Name;
					Grid->Canvas->Font->Charset = pText->pFont->Charset;
				}
			}
			else {
				Grid->Canvas->Font->Name = Font->Name;
				Grid->Canvas->Font->Charset = Font->Charset;
			}
		}
		if( Col < ec ){
			Grid->Canvas->TextRect(Rect, X, Y, bf);
		}
	}
	else {		// ƒ^ƒCƒgƒ‹
		LPCSTR	_tt[2][6]={
			{"No.","Defined Text", "Macro conversion", "Size", "Color", "Shadow",},
			{"No.","’è‹`•¶Žš—ñ", "ƒ}ƒNƒ•ÏŠ·", "»²½Þ","F","‰e",},
		};
		Grid->Canvas->TextRect(Rect, X, Y, _tt[m_Jpn][Col]);
	}
	if( !Col ) MultProc();
}
//---------------------------------------------------------------------------

void __fastcall TListTextDlg::DelBtnClick(TObject *Sender)
{
	if( Grid->Row && (Grid->Row <= pList->m_Cnt) ){
		pList->DeleteItem(pList->pBase[Grid->Row-1]);
		Grid->RowCount = pList->m_Cnt + 2;
		Grid->Invalidate();
		UpdateUI();
	}
}
//---------------------------------------------------------------------------

void __fastcall TListTextDlg::GridClick(TObject *Sender)
{
	UpdateUI();
}
//---------------------------------------------------------------------------

void __fastcall TListTextDlg::EditBtnClick(TObject *Sender)
{
	if( Grid->Row ){
		if( Grid->Row <= pList->m_Cnt ){
			if( pList->pBase[Grid->Row-1]->Edit() == TRUE ){
				Grid->Invalidate();
			}
		}
		else {
			CDrawText *pItem = (CDrawText *)pList->MakeItem(CM_TEXT);
			pItem->Start(Mmsstv->PBoxTemp->Canvas, 0, 0);
			if( pItem->Finish(0, 0) ){
				pList->AddItem(pItem);
				Grid->RowCount = pList->m_Cnt + 2;
				Grid->Invalidate();
				UpdateUI();
			}
			else {
				delete pItem;
			}
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TListTextDlg::UpBtnClick(TObject *Sender)
{
	if( Grid->Row > 1 ){
		Grid->Row--;
		pList->UpItem(Grid->Row);
		Grid->Invalidate();
		UpdateUI();
	}
}
//---------------------------------------------------------------------------

void __fastcall TListTextDlg::DwnBtnClick(TObject *Sender)
{
	if( Grid->Row <= (pList->m_Cnt - 1) ){
		pList->DownItem(Grid->Row-1);
		Grid->Row++;
		Grid->Invalidate();
		UpdateUI();
	}
}
//---------------------------------------------------------------------------


