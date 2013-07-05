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

#include "PerSpect.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TPerSpectDlg *PerSpectDlg;
//---------------------------------------------------------------------
__fastcall TPerSpectDlg::TPerSpectDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
        Caption = "Perspective transformation";
		CancelBtn->Caption = "Cancel";
        DefBtn->Caption = "Default";
    }
    memset(&m_S, 0, sizeof(m_S));
    m_S.v = 10.0;   m_S.s = 5.0;
    pBitmap = new Graphics::TBitmap;
    pBitmap->Width = PBox->Width;
    pBitmap->Height = PBox->Height;
    pBitmap->PixelFormat = pf24bit;
}

__fastcall TPerSpectDlg::~TPerSpectDlg()
{
    delete pBitmap;
}

int __fastcall TPerSpectDlg::Execute(CDrawText *pItem)
{
    m_Text.Start(PBox->Canvas, 0, 0);       // キャンバスのセット
    m_Text.Copy(pItem);
    m_Text.m_PerSpect = 0;
    m_Text.UpdateText();
    if( m_Text.pBitmap != NULL ){
        m_Back = m_Text.pBitmap->Canvas->Pixels[0][0];
        m_Text.pBitmap->Transparent = FALSE;
    }

    TBAX->Position = pItem->m_sperspect.ax * 20.0;
    TBAY->Position = pItem->m_sperspect.ay * 20.0;
    TBPX->Position = pItem->m_sperspect.px * 40.0;
    TBPY->Position = pItem->m_sperspect.py * 40.0;
    TBPZ->Position = pItem->m_sperspect.pz * 40.0;
    TBRX->Position = pItem->m_sperspect.rx;
    TBRY->Position = pItem->m_sperspect.ry;
    TBRZ->Position = pItem->m_sperspect.rz;
    TBS->Position = pItem->m_sperspect.s * 10.0;
    RGTF->ItemIndex = pItem->m_PerSpect;

    UpdateBitmap();
    if( ShowModal() == IDOK ){
        pItem->m_sperspect.ax = double(TBAX->Position)/20.0;
        pItem->m_sperspect.ay = double(TBAY->Position)/20.0;
        pItem->m_sperspect.px = double(TBPX->Position)/40.0;
        pItem->m_sperspect.py = double(TBPY->Position)/40.0;
        pItem->m_sperspect.pz = double(TBPZ->Position)/40.0;
        pItem->m_sperspect.rx = TBRX->Position;
        pItem->m_sperspect.ry = TBRY->Position;
        pItem->m_sperspect.rz = TBRZ->Position;
        pItem->m_sperspect.s = double(TBS->Position)/10.0;
        pItem->m_PerSpect = RGTF->ItemIndex;
        return TRUE;
    }
    return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TPerSpectDlg::UpdateBitmap(void)
{
    if( m_Text.pBitmap == NULL ) return;

    m_Text.m_sperspect.ax = double(TBAX->Position)/20.0;
    m_Text.m_sperspect.ay = double(TBAY->Position)/20.0;

    m_Text.m_sperspect.px = double(TBPX->Position)/40.0;
    m_Text.m_sperspect.py = double(TBPY->Position)/40.0;
    m_Text.m_sperspect.pz = double(TBPZ->Position)/40.0;

    m_Text.m_sperspect.rx = TBRX->Position;
    m_Text.m_sperspect.ry = TBRY->Position;
    m_Text.m_sperspect.rz = TBRZ->Position;
    m_Text.m_sperspect.s = double(TBS->Position)/10.0;
    pBitmap->Width = m_Text.pBitmap->Width;
    pBitmap->Height = m_Text.pBitmap->Height;
    Perspect(m_Text.pBitmap, pBitmap, &m_Text.m_sperspect, m_Back);
    MultProc();
}
//---------------------------------------------------------------------
void __fastcall TPerSpectDlg::PBoxPaint(TObject *Sender)
{
    if( RGTF->ItemIndex ){
        PBox->Canvas->Draw(0, 0, pBitmap);
    }
    else {
        PBox->Canvas->Draw(0, 0, m_Text.pBitmap);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::TBAXChange(TObject *Sender)
{
    RGTF->ItemIndex = 1;
    UpdateBitmap();
    PBox->Canvas->Draw(0, 0, pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::DefBtnClick(TObject *Sender)
{
    TBAX->Position = 40;
    TBAY->Position = 120;
    TBPX->Position = 2;
    TBPY->Position = 0;
    TBPZ->Position = 0;
    TBRX->Position = -70;
    TBRY->Position = 0;
    TBRZ->Position = -180;
    TBS->Position = 49;
    RGTF->ItemIndex = 1;
    TBAXChange(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::RGTFClick(TObject *Sender)
{
    PBox->Invalidate();
}
//---------------------------------------------------------------------------

