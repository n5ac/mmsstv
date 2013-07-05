//----------------------------------------------------------------------------
#ifndef TextDlgH
#define TextDlgH
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
#include "ComLib.h"
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include "CodeVw.h"
//----------------------------------------------------------------------------
#define	UseDraft	FALSE
//----------------------------------------------------------------------------
class TTextDlgBox : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TButton *FontBtn;
	TMemo *Memo;
	TGroupBox *GBG;
	TSpeedButton *SBM;
	TSpeedButton *SBG1;
	TSpeedButton *SBG2;
	TSpeedButton *SBG3;
	TSpeedButton *SBG4;
	TSpeedButton *SBG5;
	TPanel *PG1;
	TPanel *PG2;
	TPanel *PG3;
	TPanel *PG4;
	TPanel *PG5;
	TPanel *PG6;
	TSpeedButton *SBGRev;
	TSpeedButton *SBGRand;
	TSpeedButton *SBGRot;
	TSpeedButton *SBGA;
	TSpeedButton *SBC;
	TGroupBox *GBS;
	TSpeedButton *SBS1;
	TSpeedButton *SBS2;
	TSpeedButton *SBS3;
	TSpeedButton *SBS4;
	TPanel *PB1;
	TPanel *PB2;
	TPanel *PB3;
	TUpDown *UDL;
	TGroupBox *GB3D;
	TCheckBox *CB3D;
	TUpDown *UDY;
	TUpDown *UDX;
	TSpeedButton *SB3DI;
	TSpeedButton *SB3DD;
	TLabel *L1;
	TLabel *LY;
	TLabel *LX;
	TGroupBox *GBA;
	TSpeedButton *SBA1;
	TSpeedButton *SBA2;
	TSpeedButton *SBA3;
	TCheckBox *CBZero;
	TComboBox *CBList;
	TSpeedButton *SBLW;
	TSpeedButton *SBLD;
	TGroupBox *GBY;
	TComboBox *CBY;
	TLabel *L2;
	TSpeedButton *SBYW;
	TSpeedButton *SBYD;
	TPaintBox *PBox;
	TFontDialog *FontDialog;
	TColorDialog *ColorDialog;
	TTimer *Timer;
	TSpeedButton *SBFD;
	TSpeedButton *SBFI;
	TSpeedButton *SBA4;
	TMemo *Msg;
	TSpeedButton *SBMSG;
	TLabel *L33;
	TComboBox *CBOYW;
	TUpDown *UDOYW;
	TLabel *L34;
	TComboBox *CBRot;
	TUpDown *UDRot;
	TSpeedButton *SBF1;
	TSpeedButton *SBF2;
	TSpeedButton *SBF3;
	TSpeedButton *SBF4;
	TSpeedButton *SBF5;
	TSpeedButton *SBB;
	TSpeedButton *SBI;
	TPageControl *Page;
	TTabSheet *TabChar;
	TTabSheet *TabPer;


	TTrackBar *TBAX;
	TTrackBar *TBAY;
	TTrackBar *TBRX;
	TTrackBar *TBRY;
	TTrackBar *TBRZ;
	TTrackBar *TBS;
	TTrackBar *TBPX;
	TTrackBar *TBPY;
	TTrackBar *TBPZ;
	TLabel *L51;
	TLabel *L52;
	TLabel *L53;
	TLabel *L54;
	TLabel *L55;
	TLabel *L56;
	TLabel *L57;
	TLabel *L58;
	TLabel *L59;
	TButton *DefBtn;
	TTrackBar *TBR;
	TLabel *L60;
	TPanel *Panel;
	TSpeedButton *SB;
	TSpeedButton *SBZ1;
	TSpeedButton *SBZ2;
	TSpeedButton *SBZ3;
	TSpeedButton *SBZ4;
	TSpeedButton *SBZ5;
	TSpeedButton *SBZ6;
	TSpeedButton *SBZ7;
	TSpeedButton *SBZ8;
	TSpeedButton *SBZ9;
	TSpeedButton *SBZ10;
	TSpeedButton *SBZ11;
	TSpeedButton *SBZ12;
	TSpeedButton *SBZ13;
	TSpeedButton *SBZ14;
	TPaintBox *PBoxPer;
	TSpeedButton *SBZ15;
	TSpeedButton *SBZ16;
	TSpeedButton *SBZ17;
	TButton *OKBtn2;
	TButton *CancelBtn2;
	TTabSheet *TabMask;
	TCheckBox *CBBMask;
	TLabel *L80;
	TGroupBox *GB1;
	TPaintBox *PBoxMask;
	TButton *OKBtn3;
	TButton *CancelBtn3;
	TGroupBox *GB2;
	TGroupBox *GB3;
	TPanel *PM;
	TPaintBox *PBoxSrc;
	TSpeedButton *SBMLoad;
	TPanel *PC1;
	TPanel *PC2;
	TComboBox *CBMSize;
	TSpeedButton *SBMPaste;
	TSpeedButton *SBMRot;
	TSpeedButton *SBMMir;
	TSpeedButton *SBAdjCol;
	TSpeedButton *SBMCopy;
	TSpeedButton *SBRotCol;
	TScrollBar *SBH;
	TScrollBar *SBV;
	TGroupBox *GB4;
	TPaintBox *PBoxMList;
	TSpeedButton *SBMList;
	TPaintBox *PBoxMLSrc;
	TLabel *L90;
	TPaintBox *PBoxBM;
	TPaintBox *PBoxCS;
	TSpeedButton *SBInvCol;
	TSpeedButton *SBMEdit;
	TSpeedButton *SBPtn;
	TSpeedButton *SBZ18;
	TSpeedButton *SBZ19;
	TLabel *L61;
	TSpeedButton *SBR;
	TSpeedButton *SBZ20;
	TSpeedButton *SBZ21;
	TSpeedButton *SBZ22;
	TScrollBar *TBSC;
	TSpeedButton *SBZ23;
	TSpeedButton *SBZ24;
	TSpeedButton *SBZ25;
	TSpeedButton *SBZ26;
	TSpeedButton *SBZ27;
	TSpeedButton *SBZ28;
	TSpeedButton *SBZ29;
	TSpeedButton *SBZ30;
	TSpeedButton *SBZ31;
	TSpeedButton *SBZ32;
	TSpeedButton *SBZ33;
	TSpeedButton *SBZ34;
	TSpeedButton *SBZ35;
	TSpeedButton *SBZ36;
	TSpeedButton *SBZ37;
	TSpeedButton *SBZ38;
	TGroupBox *GB5;
	TSpeedButton *SBFN;
	TSpeedButton *SBFA;
	TSpeedButton *SBFB;
	TScrollBar *TBF;
	TSpeedButton *SBClip;
	TSpeedButton *SBL;
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall MemoChange(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall SBGClick(TObject *Sender);
	void __fastcall PB1Click(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall FormDeactivate(TObject *Sender);
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall SBMClick(TObject *Sender);
	void __fastcall SBCClick(TObject *Sender);
	void __fastcall SBLWClick(TObject *Sender);
	void __fastcall SBLDClick(TObject *Sender);
	void __fastcall CBListChange(TObject *Sender);
	void __fastcall SBYWClick(TObject *Sender);
	void __fastcall SBYDClick(TObject *Sender);
	void __fastcall CBYChange(TObject *Sender);
	void __fastcall CBYKeyPress(TObject *Sender, char &Key);
	void __fastcall CBYDropDown(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);

	void __fastcall SB3DIClick(TObject *Sender);
	void __fastcall SBMSGClick(TObject *Sender);
	void __fastcall MsgDblClick(TObject *Sender);
	void __fastcall MemoStartDrag(TObject *Sender,
		  TDragObject *&DragObject);
	void __fastcall MsgDragDrop(TObject *Sender, TObject *Source, int X,
		  int Y);
	void __fastcall MsgDragOver(TObject *Sender, TObject *Source, int X,
		  int Y, TDragState State, bool &Accept);
	void __fastcall MemoMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	
	void __fastcall MemoMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall MemoMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxMouseMove(TObject *Sender, TShiftState Shift, int X,
		  int Y);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall FontBtnMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBFClick(TObject *Sender);
	void __fastcall SBFMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBBClick(TObject *Sender);
	void __fastcall SBIClick(TObject *Sender);
	void __fastcall SBFDMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall DefBtnClick(TObject *Sender);
	
	void __fastcall SBRClick(TObject *Sender);
	void __fastcall PBoxPerMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxPerMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall PBoxPerMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	void __fastcall TBSChange(TObject *Sender);

	void __fastcall CBBMaskClick(TObject *Sender);

	void __fastcall L80Click(TObject *Sender);
	void __fastcall PBoxSrcPaint(TObject *Sender);
	void __fastcall PBoxSrcMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxSrcMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall PBoxSrcMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBMLoadClick(TObject *Sender);
	
	void __fastcall CBMSizeChange(TObject *Sender);
	void __fastcall SBMPasteClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall SBAdjColClick(TObject *Sender);
	void __fastcall SBMCopyClick(TObject *Sender);
	void __fastcall SBRotColMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	
	
	void __fastcall SBMListClick(TObject *Sender);
	void __fastcall PBoxMListPaint(TObject *Sender);
	void __fastcall PBoxMListMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxMLSrcPaint(TObject *Sender);
	void __fastcall PBoxMListDragDrop(TObject *Sender, TObject *Source, int X,
          int Y);
	void __fastcall PBoxMListDragOver(TObject *Sender, TObject *Source, int X,
          int Y, TDragState State, bool &Accept);
	
	void __fastcall PBoxMLSrcMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxMLSrcDragOver(TObject *Sender, TObject *Source, int X,
          int Y, TDragState State, bool &Accept);
	void __fastcall PBoxMLSrcDragDrop(TObject *Sender, TObject *Source, int X,
          int Y);
	void __fastcall SBMMirMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBMRotMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	void __fastcall PBoxBMPaint(TObject *Sender);
	void __fastcall PBoxBMMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxBMMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxCSPaint(TObject *Sender);
	void __fastcall PBoxCSMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall PBoxCSMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBInvColClick(TObject *Sender);
	void __fastcall SBMEditClick(TObject *Sender);
	
	void __fastcall SBPtnMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall LDefMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	void __fastcall TBSCChange(TObject *Sender);
	
	void __fastcall UDLClick(TObject *Sender, TUDBtnType Button);
	
	
	void __fastcall SBClipClick(TObject *Sender);
	
	void __fastcall SBG1Click(TObject *Sender);
	void __fastcall SBGRevClick(TObject *Sender);
	
	void __fastcall SBGRandClick(TObject *Sender);
	void __fastcall SBGRotClick(TObject *Sender);
	void __fastcall SBGAClick(TObject *Sender);
	
	
	void __fastcall SBLClick(TObject *Sender);
	void __fastcall PageChange(TObject *Sender);
private:
	int     m_DisEvent;
	int		m_DropProfile;
	Graphics::TBitmap *m_pBitmap;
	CItem   m_Item;
	int     m_Change;
	AnsiString  m_FText;
	int		m_MouseXC, m_MouseYC, m_MouseHold;
	int     m_MemoX, m_MemoY, m_MemoDown;

	void __fastcall TopMost(void);
	void __fastcall NoTopMost(void);
	void __fastcall UpdateMemoFont(BYTE charset);
	void __fastcall UpdateFText(void);
	void __fastcall UpdateUI(void);
	void __fastcall UpdateItem(CItem *pItem);
	void __fastcall UpdateDialog(CItem *pItem);
	void __fastcall UpdateBitmap(void);
	void __fastcall UD3D(int a);
	void __fastcall UpdateFontBtnHint(void);
	int __fastcall GetSBFIndex(TObject *Sender);
	void __fastcall SetSBF(int n);

	int         m_XC, m_YC;
	double      m_SCX, m_SCY;
	double      m_Deg, m_SDeg;
	double      m_Dist, m_SDistX, m_SDistY;

	int		m_Wave;
	void __fastcall UpdateWaveBtn(int dir);
	void __fastcall UpdateHint(void);
	void __fastcall FillPBox(TPaintBox *pBox);

#if UseDraft
    int			m_Draft;
#endif
    int			m_FontHeight;

	int			m_DirTB[10];
	void __fastcall UpdateTB(void);
    int			m_SBar[10];

	Graphics::TBitmap *m_pBuiltInBmp;

	Graphics::TBitmap *m_pMaskBak;
	Graphics::TBitmap *m_pMaskBmpSrc;
	Graphics::TBitmap *m_pMaskBmp;
	int			m_MaskMouseCapture;
	int			m_MaskX, m_MaskY, m_MaskXW, m_MaskYW;
    int			m_MaskAX, m_MaskAY, m_MaskAXW, m_MaskAYW;
	void __fastcall DrawMaskCursor(int x1, int y1, int x2, int y2);
	void __fastcall DrawMaskCursor(void);
	void __fastcall UpdateSBSrc(void);
	void __fastcall UpdateMaskSrc(BOOL sw);
	void __fastcall UpdateMaskSrc(HBITMAP hb);
	void __fastcall UpdateItemMask(void);

	int	m_MIndex;

   	Graphics::TBitmap *m_pCustomBmp;
	void __fastcall UpdateCustom(void);

	void __fastcall UpdateCBMSize(int xw, int yw);
	void __fastcall UpdateCBMSize(void);

	void __fastcall DeleteMaskSrc(void);
	int				m_RotCol;

   	Graphics::TBitmap *m_pMListBmp;
	Graphics::TBitmap *m_pMLSrcBmp;
	int				m_MLIndex;
	int __fastcall IsMList(int n);
	int __fastcall GetMLIndex(int x, int y);

//	void __fastcall DrawCrack(TCanvas *pCanvas, int xw, int yw, double &x, double &y, double &dir);
//	void __fastcall CreateCrack(void);

	int			m_ExecPB;
    int			m_ExecPBCount;
	AnsiString	m_MaskTempName;
	BOOL __fastcall CheckPBFinished(void);
	void __fastcall LoadEditedMask(void);
	void __fastcall QuitPB(BOOL fWait);

	TPanel* __fastcall GetPG(int index);

	TCodeView*	m_pCodeView;

    HWND	m_hClipNext;
public:
	virtual __fastcall TTextDlgBox(TComponent* AOwner);

	int __fastcall Execute(CItem *pItem);

protected:
	void __fastcall OnDrawClipboard(TMessage Message);
	void __fastcall OnChangeCbChain(TMessage Message);
	void __fastcall OnActivateApp(TMessage Message);
	void __fastcall OnCodeViewClose(TMessage Message);
	void __fastcall OnCodeInsert(TMessage Message);
BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_DRAWCLIPBOARD, TMessage, OnDrawClipboard)
    MESSAGE_HANDLER(WM_CHANGECBCHAIN, TMessage, OnChangeCbChain)
    MESSAGE_HANDLER(WM_ACTIVATEAPP, TMessage, OnActivateApp)
    MESSAGE_HANDLER(WM_FORMCLOSE, TMessage, OnCodeViewClose)
    MESSAGE_HANDLER(WM_CODEINSERT, TMessage, OnCodeInsert)
END_MESSAGE_MAP(TForm)
};
//----------------------------------------------------------------------------
//extern PACKAGE TTextDlgBox *TextDlgBox;
//----------------------------------------------------------------------------
#endif
  
 