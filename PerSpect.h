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



//----------------------------------------------------------------------------
#ifndef PerSpectDlgH
#define PerSpectDlgH
//----------------------------------------------------------------------------
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
#include "ComLib.h"
#include "Draw.h"
//#define SPERSPECTMAX    16
//----------------------------------------------------------------------------
class TPerSpectDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
    TTrackBar *TBAX;
    TTrackBar *TBAY;
    TTrackBar *TBRX;
    TTrackBar *TBRY;
    TTrackBar *TBRZ;
    TPaintBox *PBox;
    TTrackBar *TBS;
    TTrackBar *TBPX;
    TTrackBar *TBPY;
    TTrackBar *TBPZ;
    TLabel *L1;
    TLabel *L2;
    TLabel *L3;
    TLabel *L4;
    TLabel *L5;
    TLabel *L6;
    TLabel *L7;
    TLabel *L8;
    TLabel *L9;
    TButton *DefBtn;
    TRadioGroup *RGTF;
    void __fastcall PBoxPaint(TObject *Sender);
    void __fastcall TBAXChange(TObject *Sender);
    void __fastcall DefBtnClick(TObject *Sender);
    void __fastcall RGTFClick(TObject *Sender);
    
private:
    Graphics::TBitmap *pBitmap;
    SPERSPECT m_S;
    CDrawText   m_Text;
    TColor      m_Back;
    void __fastcall UpdateBitmap(void);

//    AnsiString  m_ListName[SPERSPECTMAX];
//    SPERSPECT   m_ListPara[SPERSPECTMAX];

public:
	virtual __fastcall TPerSpectDlg(TComponent* AOwner);
    __fastcall ~TPerSpectDlg();

    int __fastcall Execute(CDrawText *pItem);
};
//----------------------------------------------------------------------------
//extern PACKAGE TPerSpectDlg *PerSpectDlg;
//----------------------------------------------------------------------------
#endif
