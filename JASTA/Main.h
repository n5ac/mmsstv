//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
#include "LogFile.h"
#include "Country.h"
#include "FEdit.h"

//---------------------------------------------------------------------------
class TMainWnd : public TForm
{
__published:	// IDE 管理のコンポーネント
	TMainMenu *MainMenu;
	TStringGrid *Grid;
	TMenuItem *KF;
	TMemo *Memo;
	TMenuItem *KFL;
	TOpenDialog *OpenDialog;
	TMenuItem *KFS;
	TMenuItem *KV;
	TMenuItem *KVL;
	TMenuItem *KVS;
	TMenuItem *KH;
	TMenuItem *KHT;
	TMenuItem *KHW;
	TMenuItem *N1;
	TMenuItem *KHM;
	TMenuItem *KHR;
	TMenuItem *N2;
	TMenuItem *KFR;
	TMenuItem *KVA;
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row,
		  TRect &Rect, TGridDrawState State);
	
	void __fastcall KFLClick(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall KFSClick(TObject *Sender);
	void __fastcall KVLClick(TObject *Sender);
	void __fastcall KVSClick(TObject *Sender);
	void __fastcall KVClick(TObject *Sender);
	void __fastcall MemoClick(TObject *Sender);
	
	void __fastcall KHWClick(TObject *Sender);
	void __fastcall KHTClick(TObject *Sender);
	void __fastcall KHMClick(TObject *Sender);
	void __fastcall KHRClick(TObject *Sender);
	void __fastcall GridDblClick(TObject *Sender);
	void __fastcall MemoDblClick(TObject *Sender);
	void __fastcall KFRClick(TObject *Sender);
	void __fastcall KFClick(TObject *Sender);
	void __fastcall KVAClick(TObject *Sender);
private:	// ユーザー宣言
	AnsiString  m_SrcName;
	SYSTEMTIME  m_now;
	int         m_FIndex;
	int     m_Year;
	int     m_Days;

	int     m_ShowErrCount;

	int     m_CountHF;
	int     m_CountVU;
	int     m_CountS;

	CMULT   m_MJA;
	CMULT   m_MDX;
	CMULT   m_DUP;

	int     m_errDUP;
	int     m_errINV;
	int     m_errNOF;
	int     m_errMLT;
	int     m_errBND;

	char    m_LogName[256];
	char    m_SumName[256];
	char    m_AnaName[256];

	TFileEdit   *m_pEditLog;
	TFileEdit   *m_pEditSum;
	TFileEdit   *m_pEditAna;

	CWebRef		WebRef;

	int		m_GridMove;
	int		m_ReCalc;

	void __fastcall SetTitle(void);
	void __fastcall SetLanguage(void);
	void __fastcall ReadRegister(void);
	void __fastcall WriteRegister(void);
	int __fastcall IsValidRST(LPSTR pRST);
	void __fastcall AdjustData(SDMMLOG *sp);
	void __fastcall ShowErr(StrText *tp, int n, SDMMLOG *sp, LPCSTR pKey);
	void __fastcall Exec(LPCSTR pName, int loadorg);
	void __fastcall Calc(LPCSTR pName);
	void __fastcall MoveGrid(int n);
	int __fastcall GetBNO(int b);
	void __fastcall MakeANA(void);

	LPCSTR __fastcall ConvADIF(LPCSTR pName, int loadorg);
	LPCSTR __fastcall ConvLog200(LPCSTR pName, int loadorg);
	LPCSTR __fastcall ConvHAMLOG(LPCSTR pName, int loadorg);
	void __fastcall AdjustHamlogRSV(SDMMLOG *sp);

public:		// ユーザー宣言
	__fastcall TMainWnd(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainWnd *MainWnd;
//---------------------------------------------------------------------------
#endif

