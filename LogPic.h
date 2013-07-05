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
class TLogPicDlg : public TForm
{
__published:
	TButton *OKBtn;
	TPanel *PS1;
	TPanel *PS2;
	TPanel *PS3;
	TPanel *PS;
	TPaintBox *PB1;
	TPaintBox *PB2;
	TPaintBox *PB3;
	TPaintBox *PB;
	TSpeedButton *SBCopy;
	TPanel *PC4;
	TPaintBox *PB4;
	TPanel *PC5;
	TPaintBox *PB5;
	TSpeedButton *SBZoom;
	void __fastcall PBPaint(TObject *Sender);
	void __fastcall PB1Paint(TObject *Sender);
	void __fastcall PB2Paint(TObject *Sender);
	void __fastcall PB3Paint(TObject *Sender);
	void __fastcall PB1Click(TObject *Sender);
	void __fastcall PB2Click(TObject *Sender);
	void __fastcall PB3Click(TObject *Sender);
	void __fastcall SBCopyClick(TObject *Sender);
	void __fastcall PB4Paint(TObject *Sender);
	void __fastcall PB4Click(TObject *Sender);
	void __fastcall PB5Click(TObject *Sender);
	void __fastcall PB5Paint(TObject *Sender);
	void __fastcall SBZoomClick(TObject *Sender);
private:
	AnsiString  m_List[5];
	Graphics::TBitmap *pBitmapS[5];
	Graphics::TBitmap *pBitmap;

	int         m_Cnt;
	int         m_Cur;
	TRect       m_RectS;
	void __fastcall LoadPic(void);
	void __fastcall PaintS(TPaintBox *tp, Graphics::TBitmap *pbmp);

public:
	virtual __fastcall TLogPicDlg(TComponent* AOwner);
	__fastcall ~TLogPicDlg();


	void __fastcall Execute(LPCSTR pNote);
};
int __fastcall IsDateFormat(LPCSTR p);
int __fastcall IsLogPic(LPCSTR pNote);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
