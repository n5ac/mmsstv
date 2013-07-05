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



//---------------------------------------------------------------------------
#ifndef HistViewH
#define HistViewH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class THistViewDlg : public TForm
{
__published:	// IDE 管理のコンポーネント
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall FormDeactivate(TObject *Sender);
private:	// ユーザー宣言
#define AHDMAX  64
	Graphics::TBitmap *pBitmapH[AHDMAX];
	TPanel            *pPanel[AHDMAX];
	TPaintBox         *pBox[AHDMAX];
	TPopupMenu        *pPopup;

	TRect m_RectS;

	int     m_Max;
	int     m_Col;
	int     m_Line;
	int     m_DisEvent;

	int     m_XX;
	int     m_YY;

	void __fastcall PBClick(int n);
	void __fastcall PBPaint(TObject *Sender);
	void __fastcall PBMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y);
	void __fastcall PBDblClick(TObject *Sender);


public:		// ユーザー宣言
	__fastcall THistViewDlg(TComponent* Owner);
	__fastcall ~THistViewDlg();

	int m_TitleBar;
	void __fastcall LoadHist(int b);
	void __fastcall Add(Graphics::TBitmap *pBitmap);
	void __fastcall GetViewPos(int &x, int &y, int &w, int &h){
		x = Left;
		y = Top;
		w = ClientWidth;
		h = ClientHeight;
	};
	void __fastcall SetViewPos(int x, int y, int w, int h){
		Left = x;
		Top = y;
		ClientWidth = w;
		ClientHeight = h;
	};
	void __fastcall UpdateBitmap(void);
	int __fastcall IsPBox(TObject *Sender);
	void __fastcall SetPopup(TPopupMenu *tp);
	void __fastcall UpdateTitlebar(void);
	void __fastcall UpdateTitle(LPCSTR pTime);
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif
