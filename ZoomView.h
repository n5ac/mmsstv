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
#ifndef ZoomViewH
#define ZoomViewH
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
class TZoomViewDlg : public TForm
{
__published:
	TTimer *Timer;void __fastcall TimerTimer(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall FormClick(TObject *Sender);
private:
	Graphics::TBitmap *pBitmap;
	Graphics::TBitmap *pBitmapS;

	int     m_MaxX;
	int     m_MaxY;
	int		m_ViewXW;
    int		m_ViewYW;
	int     m_DisEvent;
	void __fastcall UpdateTitle(void);
	void __fastcall UpdateViewSize(void);
public:
	virtual __fastcall TZoomViewDlg(TComponent* AOwner);
	__fastcall ~TZoomViewDlg();

	void __fastcall Execute(Graphics::TBitmap *pbmp, int sw);
	void __fastcall StretchCopy(Graphics::TBitmap *pbmp);
	void __fastcall SetInitSize(int xw, int yw);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
