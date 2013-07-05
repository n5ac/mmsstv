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
#include <Grids.hpp>
//----------------------------------------------------------------------------
#include "Draw.h"
//----------------------------------------------------------------------------
class TListTextDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TStringGrid *Grid;
	TButton *DelBtn;
	TButton *EditBtn;
	TButton *UpBtn;
	TButton *DwnBtn;
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row,
		  TRect &Rect, TGridDrawState State);
	void __fastcall DelBtnClick(TObject *Sender);
	void __fastcall GridClick(TObject *Sender);
	void __fastcall EditBtnClick(TObject *Sender);
	void __fastcall UpBtnClick(TObject *Sender);
	void __fastcall DwnBtnClick(TObject *Sender);
private:
	CDrawGroup *pList;
	int         m_Jpn;

	void __fastcall UpdateUI(void);

	void __fastcall DrawGrade(CDrawText *pItem, TRect &Rect);
	void __fastcall DrawShadow(CDrawText *pItem, TRect &Rect);

public:
	virtual __fastcall TListTextDlg(TComponent* AOwner);

	CDraw *__fastcall Execute(CDrawGroup *pg, int pos);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
