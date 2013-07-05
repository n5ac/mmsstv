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
#ifndef BitMaskH
#define BitMaskH
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
#include "Draw.h"
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
extern void __fastcall MakeBitmapPtn(Graphics::TBitmap *pBmp, int hw, int sw, TColor col1, TColor col2);
//----------------------------------------------------------------------------
class TBitMaskDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TPanel *Panel;
	TGroupBox *GB2;
	TButton *LoadBtn;
	TPaintBox *PBox;
	TPanel *PC1;
	TPanel *PC2;
	TButton *PaintBtn;
	TScrollBar *SBV;
	TScrollBar *SBH;
	TPaintBox *PBoxCS;
	TComboBox *CBSize;
	TLabel *L1;
	TPaintBox *PBoxText;
	TSpeedButton *SBPaste;
	TSpeedButton *SBColAdj;
	TSpeedButton *SBCopy;
	TSpeedButton *SBClip;
	TGroupBox *GB1;
	TPaintBox *PBoxU;
	TPaintBox *PBoxL;
	TLabel *L2;
	TUpDown *UDL;
	TLabel *LP;
	TLabel *L3;
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall LoadBtnClick(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PCClick(TObject *Sender);
	void __fastcall PBoxMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PaintBtnClick(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	
	void __fastcall PBoxCSPaint(TObject *Sender);
	void __fastcall PBoxCSMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxCSMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall CBSizeChange(TObject *Sender);
	
	void __fastcall PBoxTextPaint(TObject *Sender);
	void __fastcall SBPasteClick(TObject *Sender);
	void __fastcall SBCopyClick(TObject *Sender);
	void __fastcall SBColAdjClick(TObject *Sender);
	void __fastcall SBClipClick(TObject *Sender);
	void __fastcall PBoxLPaint(TObject *Sender);
	void __fastcall PBoxUPaint(TObject *Sender);
	void __fastcall PBoxLMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxLDragDrop(TObject *Sender, TObject *Source, int X,
          int Y);
	void __fastcall PBoxLDragOver(TObject *Sender, TObject *Source, int X,
          int Y, TDragState State, bool &Accept);
	void __fastcall PBoxUDragOver(TObject *Sender, TObject *Source, int X,
          int Y, TDragState State, bool &Accept);
	void __fastcall PBoxUDragDrop(TObject *Sender, TObject *Source, int X,
          int Y);
	
	void __fastcall UDLClick(TObject *Sender, TUDBtnType Button);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall PBoxUMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
private:
	int		m_DisEvent;

	Graphics::TBitmap *pBitmap;
	CDrawText			m_Text;

	int m_XW;
	int m_YW;
	int m_X;
	int m_Y;
	int m_X2;
	int m_Y2;
	int m_Size;
	void __fastcall DrawCursor(void);
	void __fastcall BtnClick(int n);
	void __fastcall UpdateSB(void);

	Graphics::TBitmap *m_pCustomBmp;
	int	m_MIndex;
	void __fastcall UpdateCustom(void);
	void __fastcall UpdateText(void);
	void __fastcall UpdateCBSize(int xw, int yw);
	DYNAMIC void __fastcall AdjustSize(void);

	void __fastcall UpdateSrc(BOOL fSize);

	AnsiString		m_ListName;
	Graphics::TBitmap *m_pUserBmp;
	Graphics::TBitmap *m_pListBmp;
	void __fastcall UpdateUser(void);
	int __fastcall IsMList(int n);
	int __fastcall IsSameBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc, int fSize);

    HWND	m_hClipNext;
public:
	virtual __fastcall TBitMaskDlg(TComponent* AOwner);

	int __fastcall Execute(Graphics::TBitmap *pBmp, CDrawText *pText);

#define	CM_EDITEXIT	WM_USER+404
protected:
	void __fastcall OnEditExit(TMessage Message);
	void __fastcall OnDrawClipboard(TMessage Message);
	void __fastcall OnChangeCbChain(TMessage Message);

BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(CM_EDITEXIT, TMessage, OnEditExit)
    MESSAGE_HANDLER(WM_DRAWCLIPBOARD, TMessage, OnDrawClipboard)
    MESSAGE_HANDLER(WM_CHANGECBCHAIN, TMessage, OnChangeCbChain)
END_MESSAGE_MAP(TForm)
};
//----------------------------------------------------------------------------
//extern PACKAGE TBitMaskDlg *BitMaskDlg;
//----------------------------------------------------------------------------
#endif
 
