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

#include "ColorBar.h"
#include "Main.h"
#include "ColorSet.h"
#include "BitMask.h"
#include "PicRect.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TColorBarDlg::TColorBarDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Color bar";
		CancelBtn->Caption = "Cancel";
		RGType->Caption = "Affects";
		RGType->Items->Strings[0] = "Mono tone";
		RGType->Items->Strings[1] = "Two tones";
		RGType->Items->Strings[2] = "Colors";
		RGType->Items->Strings[3] = "Bitmap";
		RGType->Items->Strings[4] = "Sound";
		CBColVert->Caption = "Vertical gradation";
		LLine->Caption = "Line";
		SelBtn->Caption = "Components";
		BitBtn->Caption = "Bitmap";
        GB2->Caption = "Musical scale (1-8)";
	}
	pBitmap = NULL;
	pItem = NULL;
}
//---------------------------------------------------------------------
__fastcall TColorBarDlg::~TColorBarDlg()
{
	if( pBitmap != NULL ){
		delete pBitmap;
	}
}
//---------------------------------------------------------------------
void __fastcall TColorBarDlg::UpdateUI(void)
{
	switch(RGType->ItemIndex){
		case 3:
			PC1->Visible = FALSE;
			PC2->Visible = FALSE;
			PC3->Visible = FALSE;
			PC4->Visible = FALSE;
			SelBtn->Enabled = FALSE;
			BitBtn->Enabled = TRUE;
			CBColVert->Enabled = FALSE;
			LineCnt->Enabled = TRUE;
            LLine->Enabled = TRUE;
            Sound->Enabled = FALSE;
        	break;
        case 4:
			PC1->Visible = FALSE;
			PC2->Visible = FALSE;
			PC3->Visible = FALSE;
			PC4->Visible = FALSE;
			SelBtn->Enabled = FALSE;
			BitBtn->Enabled = FALSE;
			CBColVert->Enabled = FALSE;
			LineCnt->Enabled = FALSE;
            LLine->Enabled = FALSE;
            Sound->Enabled = TRUE;
        	break;
        default:
			PC1->Visible = TRUE;
			PC2->Visible = RGType->ItemIndex >= 1 ? TRUE : FALSE;
			PC3->Visible = RGType->ItemIndex >= 2 ? TRUE : FALSE;
			PC4->Visible = RGType->ItemIndex >= 2 ? TRUE : FALSE;
			CBColVert->Enabled = PC2->Visible;
			SelBtn->Enabled = RGType->ItemIndex >= 2 ? TRUE : FALSE;
			BitBtn->Enabled = FALSE;
			LineCnt->Enabled = TRUE;
            LLine->Enabled = TRUE;
            Sound->Enabled = FALSE;
        	break;
    }
}
//---------------------------------------------------------------------
int __fastcall TColorBarDlg::Execute(CDrawTitle *p)
{
	pItem = p;
	RGType->ItemIndex = pItem->m_Type;
	PC1->Color = pItem->m_Col1;
	PC2->Color = pItem->m_Col2;
	PC3->Color = pItem->m_Col3;
	PC4->Color = pItem->m_Col4;
	LineCnt->Text = pItem->m_Y2 - pItem->m_Y1 + 1;
	CBColVert->Checked = pItem->m_ColVert;
    Sound->Text = pItem->m_Sound;
	if( pItem->pBitmap != NULL ){
		pBitmap = DupeBitmap(pItem->pBitmap, -1);
	}
	UpdateUI();
	if( ShowModal() == IDOK ){
		pItem->m_Type = RGType->ItemIndex;
		pItem->m_Col1 = PC1->Color;
		pItem->m_Col2 = PC2->Color;
		pItem->m_Col3 = PC3->Color;
		pItem->m_Col4 = PC4->Color;
		pItem->m_ColVert = CBColVert->Checked;
	    pItem->m_Sound = Sound->Text;
		int d;
		if( pItem->m_Type == 4 ){
			d = strlen(pItem->m_Sound.c_str()) - 1;
            if( d < 3 ) d = 3;
			pItem->m_Y2 = pItem->m_Y1 + d;
        }
		else if( sscanf(AnsiString(LineCnt->Text).c_str(), "%u", &d) == 1 ){	//ja7ude 0428
			if( (d >= 1) && (d <= 256) ){
				pItem->m_Y2 = pItem->m_Y1 + d - 1;
				if( pItem->m_Y2 > 255 ){
					d = pItem->m_Y2 - 255;
					pItem->m_Y1 -= d;
					pItem->m_Y2 -= d;
				}
			}
		}
		if( (RGType->ItemIndex == 3) && (pBitmap != NULL) ){
			if( pItem->pBitmap == NULL ){
				pItem->pBitmap = new Graphics::TBitmap();
				AdjustBitmapFormat(pItem->pBitmap);
			}
			pItem->pBitmap->Width = pBitmap->Width;
			pItem->pBitmap->Height = pBitmap->Height;
			pItem->pBitmap->Canvas->Draw(0, 0, pBitmap);
		}
		else if( pItem->pBitmap != NULL ){
			delete pItem->pBitmap;
			pItem->pBitmap = NULL;
		}
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TColorBarDlg::PCClick(TObject *Sender)
{
	TPanel *tp = (TPanel *)Sender;
	TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);
	AddCustomColor(pDialog, PC1->Color);
	AddCustomColor(pDialog, PC2->Color);
	AddCustomColor(pDialog, PC3->Color);
	AddCustomColor(pDialog, PC4->Color);
	pDialog->Color = tp->Color;
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		tp->Color = pDialog->Color;
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------

void __fastcall TColorBarDlg::RGTypeClick(TObject *Sender)
{
	UpdateUI();    
}
//---------------------------------------------------------------------------

void __fastcall TColorBarDlg::SelBtnClick(TObject *Sender)
{
	TColorSetDlg *pBox = new TColorSetDlg(this);
	TColor col[4];
	col[0] = PC1->Color;
	col[1] = PC2->Color;
	col[2] = PC3->Color;
	col[3] = PC4->Color;
	if( pBox->Execute(col) == TRUE ){
		PC1->Color = col[0];
		PC2->Color = col[1];
		PC3->Color = col[2];
		PC4->Color = col[3];
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TColorBarDlg::BitBtnClick(TObject *Sender)
{
	TOpenDialog *pDialog = Mmsstv->OpenDialog;
	pDialog->Options >> ofCreatePrompt;
	pDialog->Options << ofFileMustExist;
	if( MsgEng ){
		pDialog->Title = "Load picture from file";
	}
	else {
		pDialog->Title = "‰æ‘œ‚Ìƒ[ƒh";
	}
	pDialog->Filter = GetPicFilter();
	pDialog->FileName = "";
	pDialog->DefaultExt = "jpg";
	pDialog->InitialDir = BitmapDir;
	SetDisPaint();
	NormalWindow(this);
	if( Mmsstv->OpenDialogExecute(TRUE) == TRUE ){
		SetDirName(BitmapDir, AnsiString(pDialog->FileName).c_str());	//ja7ude 0428
		TPicRectDlg *pBox = new TPicRectDlg(this);
		if( ::LoadImage(pBox->pBitmap, AnsiString(pDialog->FileName).c_str()) == TRUE ){	//ja7ude 0428
			Graphics::TBitmap *pBmp = new Graphics::TBitmap();
			AdjustBitmapFormat(pBmp);
			pBmp->Width = pBox->pBitmap->Width < 320 ? pBox->pBitmap->Width : 320;
			int H = 16;
			int d;
			if( sscanf(AnsiString(LineCnt->Text).c_str(), "%u", &d) == 1 ){	//ja7ude 0428
				if( (d >= 1) && (d <= 256) ){
					H = d;
				}
			}
			pBmp->Height = pBox->pBitmap->Height < H ? pBox->pBitmap->Height : H;
			if( pBox->Execute(pBmp) == TRUE ){
				if( pBitmap == NULL ){
					pBitmap = new Graphics::TBitmap();
				}
				AdjustBitmapFormat(pBitmap);
				pBitmap->Width = 320;
				pBitmap->Height = H;
				int x, y;
				for( y = 0; y < pBitmap->Height; y += pBmp->Height){
					for( x = 0; x < pBitmap->Width; x += pBmp->Width){
						pBitmap->Canvas->Draw(x, y, pBmp);
					}
				}
			}
			delete pBmp;
		}
		delete pBox;
	}
	ResDisPaint();
	TopWindow(this);
}
//---------------------------------------------------------------------------

