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

#include "Comlib.h"
#include "LineSet.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TLineSetDlg *LineSetDlg;
LPCSTR pStyle[2][9]={
	{"‚È‚µ","ŽÀü", "”jü", "“_ü", "ˆê“_½ü", "“ñ“_½ü","‰Žæ‚èü", "‰Žæ‚èü(×)", NULL},
	{"None","Solid", "Dash", "Dot", "DashDot", "DashDotDot", "Siege", "Siege(narrow)", NULL},
};
int LineTable[]={-1, psSolid, psDash, psDot, psDashDot, psDashDotDot, 5, 6};
//---------------------------------------------------------------------
__fastcall TLineSetDlg::TLineSetDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Line";
		CancelBtn->Caption = "Cancel";
		LS->Caption = "Style";
		LW->Caption = "Width";
		LC->Caption = "Color";
		LT->Caption = "Shape";
		m_lmode = 1;
	}
	else {
		m_lmode = 0;
	}
	LineStyle->Clear();
	for( int i = 0; pStyle[m_lmode][i] != NULL; i++ ){
		LineStyle->Items->Add(pStyle[m_lmode][i]);
	}
}
//---------------------------------------------------------------------
void __fastcall TLineSetDlg::UpdateUI(void)
{
	int f = (LineStyle->ItemIndex != 1) ? FALSE : TRUE;
	LW->Enabled = f;
	LineWidth->Enabled = f;
}
//---------------------------------------------------------------------
int __fastcall TLineSetDlg::Execute(CDraw *pItem)
{
	int cmd = pItem->m_Command;
	LineStyle->ItemIndex = 1;
	int i;
	for( i = 0; pStyle[0][i] != NULL; i++ ){
		if( LineTable[i] == pItem->m_LineStyle ){
			LineStyle->ItemIndex = i;
		}
	}
	LineWidth->Text = pItem->m_LineWidth;
	CDrawText *pText = (CDrawText *)pItem;
	CDrawPic *pPic = (CDrawPic *)pItem;
	LPCSTR *pTbl = NULL;
	switch(cmd){
		case CM_PIC:
			if( pPic->m_Type < 2 ){
				pTbl = MsgEng ? _ShapePIC[0] : _ShapePIC[1];
			}
			break;
		case CM_BOX:
		case CM_BOXS:
			pTbl = MsgEng ? _ShapeBOX[0] : _ShapeBOX[1];
			break;
	}
	if( pTbl != NULL ){
		while(*pTbl){
			BoxStyle->Items->Add(*pTbl++);
		}
	}
	BoxStyle->ItemIndex = cmd != CM_PIC ? pItem->m_BoxStyle : pPic->m_Shape;
	if( cmd == CM_TEXT ){
		pItem->m_LineStyle = psSolid;
		LineStyle->ItemIndex = 1;
		LS->Enabled = FALSE;
		LineStyle->Enabled = FALSE;
		PCol->Color = ( pText->m_Shadow == 6 ) ? pText->m_LineColor : pText->m_ColS;
	}
	else {
		PCol->Color = pItem->m_LineColor;
	}
	if( pTbl != NULL ){
		LT->Enabled = TRUE;
		BoxStyle->Enabled = TRUE;
	}
	else {
		LT->Enabled = FALSE;
		BoxStyle->Enabled = FALSE;
	}
	UpdateUI();
	if( ShowModal() == IDOK ){
		pItem->m_LineStyle = TPenStyle(LineTable[LineStyle->ItemIndex]);
		int d;
		sscanf(AnsiString(LineWidth->Text).c_str(), "%d", &d);	//ja7ude 0521
		if( (d >= 1) && (d < 1024) ){
			pItem->m_LineWidth = d;
		}
		if( LineStyle->ItemIndex != 1 ){
			pItem->m_LineWidth = 1;
		}
		if( (pItem->m_Command == CM_TEXT) && (pText->m_Shadow != 6) ){
			pText->m_ColS = PCol->Color;
		}
		else {
			pItem->m_LineColor = PCol->Color;
		}
		pItem->m_BoxStyle = 0;
		if( pTbl != NULL ){
			if( cmd == CM_PIC ){
				pPic->m_Shape = BoxStyle->ItemIndex;
			}
			else {
				pItem->m_BoxStyle = BoxStyle->ItemIndex;
			}
			if( pItem->m_BoxStyle == 5 ){
				int xl = pItem->m_X2 - pItem->m_X1;
				int yl = pItem->m_Y2 - pItem->m_Y1;
				if( xl != yl ){
					if( xl > yl ){
						pItem->m_Y2 = pItem->m_Y1 + xl;
					}
					else {
						pItem->m_X2 = pItem->m_X1 + yl;
					}
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TLineSetDlg::PColClick(TObject *Sender)
{
	TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);
	AddCustomColor(pDialog, PCol->Color);
	pDialog->Color = PCol->Color;
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		PCol->Color = pDialog->Color;
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------

void __fastcall TLineSetDlg::LineStyleChange(TObject *Sender)
{
	UpdateUI();
}
//---------------------------------------------------------------------------

