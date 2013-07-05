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
#ifndef RxViewH
#define RxViewH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TRxViewDlg : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPaintBox *PBox;
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall PBoxClick(TObject *Sender);
	
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
		  TShiftState Shift);
private:	// ユーザー宣言
	Graphics::TBitmap *pBitmap;
	TRect   m_rc;
	int     m_MaxX;
	int     m_MaxY;
	int     m_DisEvent;
	int     m_Mode;
	int     m_RF;
public:		// ユーザー宣言
	__fastcall TRxViewDlg(TComponent* Owner);

	void __fastcall SetViewPos(int &x, int &y, int w, int h, Graphics::TBitmap *pbmp);
	void __fastcall GetViewPos(int &x, int &y, int &w, int &h);
	void __fastcall UpdateSyncTitle(double fq);
	void __fastcall UpdateTitle(int mode, int rf);
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif
