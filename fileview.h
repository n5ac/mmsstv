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
#ifndef FileViewH
#define FileViewH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <SHELLAPI.H>
#include "ComLib.h"
//---------------------------------------------------------------------------
#define THUMBFID    0x9055aa01
#define THUMBWIND   8
#pragma option -a-	// パックの指示
typedef struct {
	USHORT  crc;
	int     size;
}DIND;
#pragma option -a	// パック解除
class CThumb {
public:
	int             m_UpdateBmp;    // アップデートフラグ
	int             m_UpdateIdx;
	int             m_FolderIndex;
	int             m_FileCount;
	int             m_Top;
	int             m_SizeX;
	int             m_SizeY;
	int             m_XW;
	DIND            *pITBL;         // インデックスのテーブル
	Graphics::TBitmap *pBitmap;     // ビットマップ
public:
	__fastcall CThumb();
	__fastcall ~CThumb(){
		CloseFolder();
	};
	void __fastcall SetSize(int sx, int sy);
	void __fastcall OpenFolder(int index, int page, int size, USHORT crc);
	void __fastcall CloseFolder(void);
	void __fastcall CloseBitmap(void);
	int __fastcall UpdateBitmap(int n);
	void __fastcall SaveThumb(int n, Graphics::TBitmap *pbmp, TRect rc, USHORT crc, int size);
	int __fastcall LoadThumb(int n, Graphics::TBitmap *pbmp, TRect rc, USHORT crc, int &size);
	void __fastcall Reset(void);
	int __fastcall IsOpen(void){return pITBL != NULL;};
};
//---------------------------------------------------------------------------
#pragma option -a-	// パックの指示
typedef struct {
	LPCSTR  pName;
	USHORT  crc;
}CFD;
#pragma option -a	// パック解除
class CFILEL {
public:
	int     AMax;
	int     Count;
	CFD     *pBase;
private:
	void __fastcall Alloc(void);
public:
	CFILEL(){
		pBase = NULL;
		Delete();
	};
	~CFILEL(){
		Delete();
	};
	void __fastcall Delete(void);
	void __fastcall Add(LPCSTR pn, USHORT crc);
	LPCSTR __fastcall Get(USHORT &crc, int n);
	LPCSTR __fastcall Get(int n);
	void __fastcall Sort(void);
};
//---------------------------------------------------------------------------
#define AHDMAX  64
class CFILEV {
public:
	int         m_Type;
	AnsiString  m_Folder;                   // フォルダ名
	AnsiString  m_Name;                     // ページの名前
	int         m_Size[AHDMAX];             // 画像サイズの配列
	int         m_CurPage;                  // 現在のページ番号
	int         m_MaxPage;                  // 最大ページ数
	int         m_UseIndex;                 // インデックスを使う
	CFILEL      *pList;                     // ファイル名のリスト
	Graphics::TBitmap *pBitmapS;            // Col x Lineのビットマップ
	CThumb      m_Thumb;
public:
	__fastcall CFILEV();
	inline __fastcall ~CFILEV(){
		Delete();
	};
	void __fastcall Delete(void);
};
//---------------------------------------------------------------------------
class TFileViewDlg : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPanel *Panel;
	TSpeedButton *SBNew;
	TUpDown *UD;
	TLabel *LPage;
	TSpeedButton *SBMode;
	TSpeedButton *SBUpdate;
	TPopupMenu *Popup;
	TMenuItem *KTXS;
	TMenuItem *KTX;
	TMenuItem *N2;
	TMenuItem *KC;
	TMenuItem *N1;
	TMenuItem *KTT;
	TMenuItem *KTK;
	TMenuItem *KTS;
	TMenuItem *KS;
	TMenuItem *KS1;
	TMenuItem *KS2;
	TMenuItem *KS3;
	TMenuItem *KS4;
	TMenuItem *KT;
	TMenuItem *KT4;
	TMenuItem *KT9;
	TMenuItem *KT16;
	TMenuItem *N3;
	TMenuItem *KP;
	TMenuItem *KPO;
	TMenuItem *KPE;
	TMenuItem *KPP;
	TMenuItem *KNP;
	TMenuItem *KDP;
	TMenuItem *KRP;
	TMenuItem *N4;
	TMenuItem *KPI;
	TMenuItem *KPC;
	TMenuItem *KBP;
	TMenuItem *KPOL;
	TMenuItem *KD;
	TMenuItem *KV;
	void __fastcall FormResize(TObject *Sender);
	void __fastcall UDClick(TObject *Sender, TUDBtnType Button);
	void __fastcall SBNewClick(TObject *Sender);
	void __fastcall SBUpdateClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall KTXSClick(TObject *Sender);
	void __fastcall KTXClick(TObject *Sender);
	void __fastcall KCClick(TObject *Sender);
	void __fastcall KTTClick(TObject *Sender);
	void __fastcall PopupPopup(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
		  TShiftState Shift);

	void __fastcall KTKClick(TObject *Sender);
	void __fastcall FormDeactivate(TObject *Sender);
	void __fastcall KTSClick(TObject *Sender);
	void __fastcall KSClick(TObject *Sender);
	void __fastcall KT4Click(TObject *Sender);

	void __fastcall KPClick(TObject *Sender);
	void __fastcall KPEClick(TObject *Sender);
	void __fastcall KNPClick(TObject *Sender);
	void __fastcall KDPClick(TObject *Sender);
	void __fastcall KRPClick(TObject *Sender);
	void __fastcall KPIClick(TObject *Sender);
	void __fastcall KPCClick(TObject *Sender);
	void __fastcall UDMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	
	void __fastcall PanelDragOver(TObject *Sender, TObject *Source, int X,
		  int Y, TDragState State, bool &Accept);
	void __fastcall PanelDragDrop(TObject *Sender, TObject *Source, int X,
		  int Y);
	void __fastcall KBPClick(TObject *Sender);
	void __fastcall KPOLClick(TObject *Sender);
	void __fastcall KDClick(TObject *Sender);
	void __fastcall KVClick(TObject *Sender);
private:	// ユーザー宣言
	TPanel            *pPanel[AHDMAX];
	TPaintBox         *pBox[AHDMAX];
	TPopupMenu        *pPopup;
	TRect m_RectS;

	int     m_Max;          // 表示可能なサムネイルの数
	int     m_Col;          // 表示可能なサムネイルのカラム数
	int     m_Line;         // 表示可能なサムネイルの行数
	int     m_DisEvent;

	int __fastcall GetCH(int ch);
	int __fastcall IsDrag(void);
	void __fastcall PBClick(int n);
	void __fastcall PBPaint(TObject *Sender);
	void __fastcall PBMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBMouseMove(TObject *Sender,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBDblClick(TObject *Sender);

	int     m_XX, m_YY;

	int     m_CurFile;
	void __fastcall UpdateTitle(void);
	void __fastcall UpdateStat(void);

	int __fastcall NewFolder(void);
	void __fastcall MakeThImage(int mm);

	void __fastcall TabSChange(TObject *Sender);
	void __fastcall LoadFile(Graphics::TBitmap *pBitmap, LPCSTR pName);
	void __fastcall GetRect(TRect &rc, int n);
	void __fastcall SetBitmapSize(void);

public:
	int     m_MyIndex;
	int     m_Overlap;      // ウインドウの重なりを調べる
	int     m_Suspend;      // 表示の一時的な停止
	int     m_TitleBar;
	CFILEV  *pFileV[FPAGEMAX];

	int     m_MaxPage;
	int     m_CurPage;
	CFILEV  *pCurPage;

	AnsiString  m_Name;
	void __fastcall CheckOverlap(void);
	int     m_SSize;        // サムネイルの形状サイズ
	void __fastcall UpdateSize(int sw);

	TTabControl *pTabS;
	int __fastcall GetWriteFileName(AnsiString &as, int no);

public:		// ユーザー宣言
	__fastcall TFileViewDlg(TComponent* Owner);
	__fastcall ~TFileViewDlg();

	void __fastcall UpdateList(void);
	void __fastcall LoadFileList(void);
	void __fastcall LoadImage(void);
	int __fastcall CopyStretchBitmap(Graphics::TBitmap *pBitmap);
	int __fastcall CopyRectBitmap(Graphics::TBitmap *pBitmap);
	int __fastcall CopyBitmap(Graphics::TBitmap *pBitmap);
	void __fastcall LoadCurrentBitmap(Graphics::TBitmap *pBitmap);
	Graphics::TBitmap* __fastcall MakeCurrentBitmap(void);
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
		CheckOverlap();
	};
	void __fastcall UpdateBitmap(void);
	int __fastcall IsPBox(TObject *Sender);
	void __fastcall SetPopup(TPopupMenu *tp);
	void __fastcall UpdateTitlebar(void);
	void __fastcall GetSize(int &w, int &h);
	int __fastcall GetType(void);
	void __fastcall MakeFileV(void);
	void __fastcall Reset(void);
	void __fastcall SetCurPage(void){
		pCurPage = pFileV[m_CurPage];
	};
	void __fastcall GetPageName(AnsiString &as, int n);
	void __fastcall GetCurFileName(AnsiString &as);

#define	CM_EDITEXIT	WM_USER+404
protected:
	void __fastcall OnMove(TMessage *Message);
	void __fastcall OnEditExit(TMessage Message);
BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_MOVE, TMessage*, OnMove)
	MESSAGE_HANDLER(CM_EDITEXIT, TMessage, OnEditExit)
END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif

