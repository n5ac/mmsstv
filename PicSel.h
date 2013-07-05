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
#ifndef PicSelH
#define PicSelH
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
#include "sstv.h"
//----------------------------------------------------------------------------
class TPicSelDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TRadioGroup *RGType;
	TPanel *Panel;
	TPaintBox *PBox;
	TButton *PasteBtn;
	TButton *LoadBtn;
	TTimer *Timer;
	TRadioGroup *RGShape;
	TButton *LineBtn;
	TCheckBox *CBASize;
	TGroupBox *GBC;
	TLabel *L1;
	TSpeedButton *SBFree;
	TSpeedButton *SBRect;
	TSpeedButton *SBRRect;
	TSpeedButton *SBElip;
	TLabel *L2;
	TCheckBox *CBSmz;
	TPanel *PCW;
	TPanel *PCB;
	TPanel *PCR;
	TPanel *PCG;
	TCheckBox *CBR;
	TGroupBox *GBT;
	TComboBox *CBT;
	TLabel *L3;
	TButton *CTBtn;
	TButton *CTRBtn;
	TButton *LdRBtn;
	TButton *PstRBtn;
	TButton *ClipBtn;
	TButton *EditBtn;
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall RGTypeClick(TObject *Sender);
	void __fastcall PasteBtnClick(TObject *Sender);
	void __fastcall LoadBtnClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall RGShapeClick(TObject *Sender);
	void __fastcall PBoxMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall LineBtnClick(TObject *Sender);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);

	void __fastcall SBRectClick(TObject *Sender);


	void __fastcall CBSmzClick(TObject *Sender);
	void __fastcall L2Click(TObject *Sender);
	void __fastcall PCWClick(TObject *Sender);
	void __fastcall CBRClick(TObject *Sender);
	void __fastcall CTBtnClick(TObject *Sender);
	void __fastcall CTRBtnClick(TObject *Sender);

	void __fastcall LdRBtnClick(TObject *Sender);
	void __fastcall PstRBtnClick(TObject *Sender);
	void __fastcall ClipBtnClick(TObject *Sender);
	void __fastcall EditBtnClick(TObject *Sender);
private:
	CDrawPic    *pItem;
	Graphics::TBitmap *pBitmap;
	Graphics::TBitmap *pBitmapS;
	CPolygon    m_Polygon;
	int         m_Make;
	int         m_Move;
	int         m_Del;
	DWORD       m_MoveCnt;
	int         m_AX;
	int         m_AY;
	int         m_CX;
	int         m_CY;
	int         m_FX;
	int         m_FY;
	int         m_ReqLineStyle;
	int         m_Count;
	int         m_DisEvent;
	void __fastcall DrawCursor(void);
	CSmooz      m_SmX;
	CSmooz      m_SmY;
	void __fastcall UpdateBtn(void);
	void __fastcall Smooz(void);
//    void __fastcall DownColor(Graphics::TBitmap *pbmp, int lm);
	void __fastcall CopyColor(int x, int y);
	void __fastcall DrawTransPoint(void);
	int         m_TPX;
	int         m_TPY;
	int         m_Sync;
	DYNAMIC void __fastcall AdjustSize(void);
	int         m_XW;
	int         m_YW;
	void __fastcall MakeBackup(void);
public:
	virtual __fastcall TPicSelDlg(TComponent* AOwner);
	__fastcall ~TPicSelDlg();

	int __fastcall TPicSelDlg::Execute(CDrawPic *pt);

    HWND	m_hClipNext;
#define	CM_EDITEXIT	WM_USER+404
protected:
	void __fastcall OnDrawClipboard(TMessage Message);
	void __fastcall OnChangeCbChain(TMessage Message);
	void __fastcall OnEditExit(TMessage Message);

BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_DRAWCLIPBOARD, TMessage, OnDrawClipboard)
    MESSAGE_HANDLER(WM_CHANGECBCHAIN, TMessage, OnChangeCbChain)
	MESSAGE_HANDLER(CM_EDITEXIT, TMessage, OnEditExit)
END_MESSAGE_MAP(TForm)
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
  
