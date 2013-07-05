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
#ifndef OCBH
#define OCBH
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
//----------------------------------------------------------------------------
#include "Draw.h"
//----------------------------------------------------------------------------
class TColorBarDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TRadioGroup *RGType;
	TPanel *PC1;
	TPanel *PC2;
	TPanel *PC3;
	TPanel *PC4;
	TButton *SelBtn;
	TLabel *LLine;
	TComboBox *LineCnt;
	TButton *BitBtn;
	TCheckBox *CBColVert;
	TGroupBox *GB2;
	TEdit *Sound;
	void __fastcall PCClick(TObject *Sender);
	void __fastcall RGTypeClick(TObject *Sender);
	void __fastcall SelBtnClick(TObject *Sender);
	void __fastcall BitBtnClick(TObject *Sender);
	
private:
	void __fastcall UpdateUI(void);
	Graphics::TBitmap *pBitmap;
	CDrawTitle        *pItem;
public:
	virtual __fastcall TColorBarDlg(TComponent* AOwner);
	__fastcall TColorBarDlg::~TColorBarDlg();

	int __fastcall TColorBarDlg::Execute(CDrawTitle *pItem);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
