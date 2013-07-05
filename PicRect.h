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
#ifndef PicRectH
#define PicRectH
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
#include "ComLib.h"
#include <Menus.hpp>
//----------------------------------------------------------------------------
class TPicRectDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TPanel *Panel;
	TScrollBar *SBH;
	TScrollBar *SBV;
	TPaintBox *PBox;
	TSpeedButton *SB256;
	TSpeedButton *SB240T;
	TSpeedButton *SB240B;
	TPaintBox *PBoxM;
	TSpeedButton *SBRatio;
	TLabel *Label1;
	TSpeedButton *SBRL;
	TSpeedButton *SBRR;
	TSpeedButton *SBMIR;
	TSpeedButton *SBFil;
	TSpeedButton *SBStrach;
	TComboBox *SMode;
	TSpeedButton *SBNStrach;
	TSpeedButton *SBMax;
	TSpeedButton *SBC;
	TMainMenu *MMenu;
	TMenuItem *KF;
	TMenuItem *KE;
	TMenuItem *KV;
	TMenuItem *KFS;
	TMenuItem *KFSC;
	TMenuItem *KEC;
	TMenuItem *KECC;
	TMenuItem *KEP;
	TMenuItem *KVAS;
	TMenuItem *N7;
	TMenuItem *KEFC;
	TMenuItem *KEFCM;
	TMenuItem *KEFCP;
	TMenuItem *KEF;
	TMenuItem *KEFD;
	TMenuItem *N8;
	TMenuItem *KEMC;
	TMenuItem *KEMM;
	TMenuItem *N1;
	TMenuItem *KER;
	TSpeedButton *SBRot;
	TMenuItem *N2;
	TMenuItem *KEU;
	TMenuItem *N3;
	TMenuItem *KEE;
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall SBVChange(TObject *Sender);
	
	void __fastcall SBRatioClick(TObject *Sender);
	void __fastcall SBRLClick(TObject *Sender);
	void __fastcall SBRRClick(TObject *Sender);
	void __fastcall SBMIRClick(TObject *Sender);
	void __fastcall SBFilClick(TObject *Sender);
	void __fastcall KEFCMClick(TObject *Sender);
	void __fastcall KEFClick(TObject *Sender);
	void __fastcall KECClick(TObject *Sender);
	void __fastcall KECCClick(TObject *Sender);
	void __fastcall KFSClick(TObject *Sender);
	void __fastcall KFSCClick(TObject *Sender);
	void __fastcall KEFDClick(TObject *Sender);
	void __fastcall KEMCClick(TObject *Sender);
	void __fastcall KEMMClick(TObject *Sender);
	void __fastcall SBMaxClick(TObject *Sender);
	void __fastcall KVASClick(TObject *Sender);
	void __fastcall KEFCPClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	
	
	void __fastcall SBCClick(TObject *Sender);
	void __fastcall SBCMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBMaxMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	void __fastcall KEPClick(TObject *Sender);
	
	void __fastcall KFClick(TObject *Sender);
	void __fastcall KERClick(TObject *Sender);
	
	void __fastcall SBRotClick(TObject *Sender);
	void __fastcall SBRotMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall KEUClick(TObject *Sender);
	void __fastcall KEEClick(TObject *Sender);
	
private:
	Graphics::TBitmap *pwBmp;
	Graphics::TBitmap *pBmpSrc;

	CAlignList	AlignList;
	void __fastcall EntryAlignControl(void);
	void __fastcall UpdateTitle(void);

	void __fastcall AdjustRatio(void);

	int     m_X1, m_Y1;
	int     m_X2, m_Y2;
	int     m_CW, m_CH;
	int     m_Cursor;
	int     m_MouseCapture;
	int     m_MouseColor;
	int     m_Space;
	void __fastcall DrawCursor(void);
	void __fastcall SetPBoxSize(void);
	Graphics::TBitmap * __fastcall GetClipBitmap(void);

	TRect   m_Src;
	double  m_sx, m_sy;
	double  m_sc;
	int     m_Stretch;
	void __fastcall AdjustXY(int &X, int &Y);
	void __fastcall SetCenter(void);
	void __fastcall RotPic(double deg);
	void __fastcall SaveSrc(void);

	double	m_RotDeg;
    int		m_RotFlag;
    int		m_RotX1, m_RotY1, m_RotX2, m_RotY2;

public:
	virtual __fastcall TPicRectDlg(TComponent* AOwner);
	__fastcall ~TPicRectDlg();

	int __fastcall Execute(Graphics::TBitmap *pbmp);
	int __fastcall Copy(Graphics::TBitmap *pbmp);

	Graphics::TBitmap *pBitmap;

#define	CM_EDITEXIT	WM_USER+404
protected:
	void __fastcall OnEditExit(TMessage Message);

BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(CM_EDITEXIT, TMessage, OnEditExit)
END_MESSAGE_MAP(TForm)
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
