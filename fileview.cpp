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
#include <vcl.h>
#pragma hdrstop

#include "Main.h"
#include "FileView.h"
#include "PicRect.h"
#include "ZoomView.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TFileViewDlg::TFileViewDlg(TComponent* Owner)
	: TForm(Owner)
{
	pTabS = NULL;
	Font->Name = sys.m_WinFontName;
	Font->Charset = sys.m_WinFontCharset;
	if( MsgEng ){
		SBNew->Hint = "Change folder";
		SBUpdate->Hint = "Update recent files";
		SBMode->Hint = "Stretch copy";

		KTXS->Caption = "&Stretch copy to TX window";
		KTX->Caption = "Copy to TX window (via Clipper)...";
		KPO->Caption = "Overlay to Template";
		KPOL->Caption = "Embed to Template as OLE";
//		KPE->Caption = "&Edit...";
		KTT->Caption = "Show title bar";
		KTK->Caption = "Show the image as keeping aspect ratio";
		KNP->Caption = "Create new page";
		KDP->Caption = "Delete page";
		KRP->Caption = "Rename page";
		KBP->Caption = "Bottom tab";
		KPI->Caption = "Fast access with index";
	}
	KC->Caption = Mmsstv->KRC->Caption;
	KPC->Caption = Mmsstv->KTP->Caption;
	KD->Caption = Mmsstv->KTD->Caption;
	KV->Caption = Mmsstv->KView->Caption;
	KTS->Caption = Mmsstv->KTSD->Caption;
	KT->Caption = Mmsstv->KSMS->Caption;
	KS->Caption = Mmsstv->KHVS->Caption;
	KS1->Caption = Mmsstv->KHVS1->Caption;
	KS2->Caption = Mmsstv->KHVS2->Caption;
	KS3->Caption = Mmsstv->KHVS3->Caption;
	KS4->Caption = Mmsstv->KHVS4->Caption;
	KPP->Caption = Mmsstv->KTX->Caption;
	KPE->Caption = Mmsstv->KSE->Caption;

	for( int i = 0; i < AHDMAX; i++ ){
		pPanel[i] = NULL;
		pBox[i] = NULL;
	}
	pPopup = NULL;
	m_Col = 6;
	m_Line = 1;
	m_SSize = 2;
	m_RectS.Left = 0;
	m_RectS.Top = 0;
	m_RectS.Right = 80;
	m_RectS.Bottom = 64;
	m_TitleBar = 1;
	m_Max = m_Col = m_Line = 0;
	m_MaxPage = 1;
	m_CurPage = 0;
	m_MyIndex = 0;
	for( int i = 0; i < FPAGEMAX; i++ ){
		pFileV[i] = NULL;
	}
	pFileV[0] = new CFILEV;
	pCurPage = pFileV[0];
	m_CurFile = -1;
	m_Overlap = 0;
	m_Name = "";
	m_DisEvent = 0;
}
//---------------------------------------------------------------------------
__fastcall TFileViewDlg::~TFileViewDlg()
{
	g_ExecPB.Cancel();
	for( int i = 0; i < FPAGEMAX; i++ ){
		if( pFileV[i] != NULL ){
			pFileV[i]->m_Thumb.CloseFolder();
			delete pFileV[i];
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UpdateTitlebar(void)
{
	if( m_TitleBar ){
		BorderStyle = bsSizeable;
	}
	else {
		BorderStyle = bsNone;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::CheckOverlap(void)
{
	RECT rc;

	::GetWindowRect(Mmsstv->TabS->Handle, &rc);
	rc.top += Mmsstv->TabS->TabHeight;
	int yw = (rc.bottom - rc.top) / 8;
	rc.bottom -= yw;
	m_Overlap = 0;
	if( ((Left > rc.left) && (Left < rc.right)) || (((Left+Width) > rc.left) && ((Left+Width) < rc.right))){
		if( (Top > rc.top) && (Top < rc.bottom) ){
			m_Overlap = 1;
		}
		if( ((Top+Height) > rc.top) && ((Top+Height) < rc.bottom) ){
			m_Overlap = 1;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::OnMove(TMessage *Message)
{
	CheckOverlap();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::MakeFileV(void)
{
	for( int i = 0; i < m_MaxPage; i++ ){
		if( pFileV[i] == NULL ){
			pFileV[i] = new CFILEV;
		}
	}
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::GetCH(int ch)
{
	if( pTabS != NULL ){
		return ch + Panel->Height + pTabS->TabHeight + 3;
	}
	else {
		return ch + Panel->Height;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::GetRect(TRect &rc, int n)
{
	int xw = m_RectS.Right;
	int x = n * xw;
	rc.Left = x; rc.Top = 0;
	rc.Right = x + xw; rc.Bottom = m_RectS.Bottom;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::SetBitmapSize(void)
{
	CFILEV *cp = pCurPage;
	if( cp->pBitmapS == NULL ){
		cp->pBitmapS = new Graphics::TBitmap();
		AdjustBitmapFormat(cp->pBitmapS);
	}
	cp->pBitmapS->Width = m_Max * m_RectS.Right;
	cp->pBitmapS->Height = m_RectS.Bottom;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::FormResize(TObject *Sender)
{
	if( m_DisEvent ) return;

	int WW = m_RectS.Right + 6;
	int HH = m_RectS.Bottom + 6;
	if( BorderStyle == bsNone ){
		int ch = m_Line * HH;
		int cw = m_Col * WW;
		m_DisEvent++;
		ClientHeight = GetCH(ch);
		ClientWidth = cw;
		m_DisEvent--;
		return;
	}
	int OldMax = m_Max;
	int h = (pTabS != NULL) ? ClientHeight - pTabS->TabHeight - 6 : ClientHeight;
	h = int((double(h - Panel->Height)/ HH) + 0.5);
	if( !h ) h = 1;
	int w = int((double(ClientWidth)/ WW) + 0.5);
	if( !w ) w = 1;
	while( (h * w) > AHDMAX ){
		if( w > 1 ){
			w--;
		}
		else if( h > 1 ){
			h--;
		}
	}
	int ch = h * HH;
	int cw = w * WW;
	if( cw < (UD->Left + UD->Width) ){
		w++;
		cw += WW;
	}
	m_Col = w;
	m_Line = h;
	m_Max = h * w;
	SetBitmapSize();
	TWinControl *pCtr = this;
	if( (m_MaxPage > 1) && (pTabS == NULL) ){
		Panel->Align = alNone;
		RemoveControl(Panel);
		pTabS = new TTabControl(this);
		pTabS->Width = ClientWidth;
		pTabS->Height = ClientHeight;
		pTabS->TabPosition = KBP->Checked ? tpBottom : tpTop;
		InsertControl(pTabS);
		pTabS->Parent = this;
		pTabS->Align = alClient;
		for( int j = 0; j < m_MaxPage; j++ ){
			AnsiString as;
			GetPageName(as, j);
			pTabS->Tabs->Add(as);
		}
		pTabS->Font->Height = -16;
		pTabS->TabHeight = 20;
		pTabS->InsertControl(Panel);
		Panel->Parent = pTabS;
		if( KBP->Checked ){
			Panel->Top = 0;
		}
		else {
			Panel->Top = pTabS->TabHeight + 3;
		}
		pTabS->TabIndex = m_CurPage;
		pTabS->OnChange = TabSChange;
		pCtr = pTabS;
	}
	pCurPage = pFileV[m_CurPage];
	if( UD->Max < pCurPage->m_CurPage ) UD->Max = SHORT(pCurPage->m_CurPage + 1);
	UD->Position = SHORT(pCurPage->m_CurPage);
	int i;
	for( i = 0; i < m_Max; i++ ){
		if( pPanel[i] == NULL ){
			pPanel[i] = new TPanel(this);
			TPanel *pn = pPanel[i];
			pn->BorderStyle = bsSingle;
			pn->Width = WW;
			pn->Height = HH;
			pCtr->InsertControl(pn);
			pn->Parent = pCtr;
			pBox[i] = new TPaintBox(this);
			TPaintBox *pb = pBox[i];
			pn->InsertControl(pb);
			pb->Parent = pn;
			pb->Align = alClient;
			pb->OnPaint = PBPaint;
			pb->OnMouseDown = PBMouseDown;
			pb->OnMouseMove = PBMouseMove;
			pb->OnDblClick = PBDblClick;
			pb->OnDragOver = PanelDragOver;
			pb->OnDragDrop = PanelDragDrop;
			pb->PopupMenu = pPopup;
		}
		else {
			pPanel[i]->Visible = FALSE;
			pPanel[i]->Width = WW;
			pPanel[i]->Height = HH;
		}
		MultProc();
	}
	for( ; i < AHDMAX; i++ ){
		if( pPanel[i] != NULL ){
			pPanel[i]->Visible = FALSE;
		}
		MultProc();
	}
	m_DisEvent++;
	ClientHeight = GetCH(ch);
	ClientWidth = cw;
	for( i = 0; i < m_Max; i++ ){
		pPanel[i]->Left = (i % m_Col) * WW;
		pPanel[i]->Top = Panel->Top + Panel->Height + (i / m_Col) * HH;
		pPanel[i]->Visible = TRUE;
	}
	if( pTabS != NULL ) Panel->Width = pTabS->ClientWidth;
	m_DisEvent--;
	if( m_Max != OldMax ){
		for( i = 0; i < m_MaxPage; i++ ){
			if( i != m_CurPage ){
				delete pFileV[i]->pList;
				pFileV[i]->pList = NULL;
			}
		}
		LoadImage();
	}
	SetPopup(Popup);
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::IsPBox(TObject *Sender)
{
	for( int i = 0; i < m_Max; i++ ){
		if( Sender == pBox[i] ) return i;
	}
	return -1;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UpdateBitmap(void)
{
	for( int i = 0; i < m_Max; i++ ){
		pBox[i]->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::SetPopup(TPopupMenu *tp)
{
	pPopup = tp;
	for( int i = 0; i < m_Max; i++ ){
		pBox[i]->PopupMenu = tp;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::GetSize(int &w, int &h)
{
	int n = m_CurFile - (pCurPage->m_CurPage * m_Max);
	w = pCurPage->m_Size[n] & 0x00007fff;
	h = pCurPage->m_Size[n] >> 16;
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::GetType(void)
{
	if( (m_CurFile >= pCurPage->pList->Count) || (m_CurFile < 0) ) return 0;
	LPCSTR pExt = GetEXT(pCurPage->pList->Get(m_CurFile));
	if( strcmpi(pExt, "JPG") ) return 2;   // bmp, wmf
	return 1;                              // jepg
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UpdateTitle(void)
{
	char bf[256];

	AnsiString Size;
	LPCSTR pName;
	if( (pCurPage->pList != NULL) && (m_CurFile < pCurPage->pList->Count) && (m_CurFile >= 0) ){
		pName = pCurPage->pList->Get(m_CurFile);
		if( sys.m_FileViewShowSize ){
			int w, h;
			GetSize(w, h);
			sprintf(bf, "%ux%u ", w, h);
			Size = bf;
		}
	}
	else {
		pName = "";
	}
	sprintf(bf, "%s%s%s", Size.c_str(), pCurPage->m_Folder.c_str(), pName);
	Caption = bf;
	SBMode->Enabled = pCurPage->m_Type ? FALSE : TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UpdateStat(void)
{
	char bf[64];
	pCurPage->m_MaxPage = 0;
	int FileCnt = 0;
	if( (pCurPage->pList != NULL) && m_Max ){
		FileCnt = pCurPage->pList->Count;
		pCurPage->m_MaxPage = (FileCnt + m_Max - 1) / m_Max;
		UD->Max = SHORT(pCurPage->m_MaxPage-1);
		if( pCurPage->m_CurPage > UD->Max ) pCurPage->m_CurPage = UD->Max;
		if( pCurPage->m_CurPage < 0 ) pCurPage->m_CurPage = 0;
		if( !m_Name.IsEmpty() ){
			for( int i = 0; i < FileCnt; i++ ){
				if( !strcmpi(m_Name.c_str(), pCurPage->pList->Get(i)) ){
					pCurPage->m_CurPage = i / m_Max;
					break;
				}
			}
			m_Name = "";
		}
		UD->Position = SHORT(pCurPage->m_CurPage);
		UD->Enabled = TRUE;
		UD->Repaint();
	}
	else {
		UD->Enabled = FALSE;
	}
	sprintf(bf, "%u/%u (%u files)", UD->Position + 1, pCurPage->m_MaxPage, FileCnt);
	LPage->Caption = bf;
	UpdateTitle();
}
//---------------------------------------------------------------------------
USHORT CalcCRC(USHORT crc, BYTE c)
{
	int		i;

	crc = USHORT(crc ^ (c << 8));
	for( i = 0; i < 8; i++ ){
		if( crc & 0x8000 ){
			crc = USHORT((crc << 1) ^ 0x1021);
		}
		else {
			crc = USHORT(crc << 1);
		}
	}
	return crc;
}
//---------------------------------------------------------------------------
USHORT __fastcall AddCRC(USHORT d, const BYTE *p, int n)
{
	for( ; n; n--, p++ ){
		d = CalcCRC(d, *p);
	}
	return d;
}
//---------------------------------------------------------------------------
USHORT __fastcall GetCRC(LPCSTR p)
{
	USHORT d = 0;
	for( ; *p; p++ ){
		d = CalcCRC(d, *p);
	}
	if( !d ) d++;
	return d;
}
//---------------------------------------------------------------------------
USHORT __fastcall GetCRC(WIN32_FIND_DATA *fp)
{
	USHORT d = GetCRC(fp->cFileName);
	d = AddCRC(d, (const BYTE *)&fp->ftLastWriteTime, sizeof(FILETIME));
	d = AddCRC(d, (const BYTE *)&fp->nFileSizeLow, sizeof(DWORD));
	if( !d ) d++;
	return d;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::LoadFileList(void)
{
	if( pCurPage->pList == NULL ){
		pCurPage->pList = new CFILEL;
	}
	pCurPage->pList->Delete();

	WIN32_FIND_DATA FileData;
	HANDLE hSearch;

	MultProc();
	if( pCurPage->m_Folder.IsEmpty() || (::SetCurrentDirectory(pCurPage->m_Folder.c_str()) != TRUE) ){
		if( NewFolder() == TRUE ){
			if( ::SetCurrentDirectory(pCurPage->m_Folder.c_str()) != TRUE ){
				pCurPage->m_Folder = "\\";
				::SetCurrentDirectory(pCurPage->m_Folder.c_str());
			}
		}
		else {
			pCurPage->m_Folder = "\\";
			::SetCurrentDirectory(pCurPage->m_Folder.c_str());
		}
	}
	MultProc();

	hSearch = FindFirstFile("*.*", &FileData);
	if(hSearch == INVALID_HANDLE_VALUE){
		UpdateStat();
		return;
	}

	MultProc();
	while(1){
		LPCSTR pExt = GetEXT(FileData.cFileName);
		int f = 0;
		switch(pCurPage->m_Type){
			case 0:
				if( IsPic(pExt) ) f = 1;
				break;
			case 1:
				if( !strcmpi(pExt, "MTM") ) f = 1;
				break;
			case 2:
				if( !strcmpi(pExt, "MTI") ) f = 1;
				break;
			case 3:
				if( !strcmpi(pExt, "DLL") ) f = 1;
				break;
			case 4:
				if( !(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){
					if( *pExt && strcmpi(pExt, "DLL") && strcmpi(pExt, "EXE") ) f = 1;
				}
				break;
		}
		if( f ){
			pCurPage->pList->Add(FileData.cFileName, GetCRC(&FileData));
		}
		if(!FindNextFile(hSearch, &FileData)) break;
		MultProcA();
	}
	FindClose(hSearch);
	pCurPage->pList->Sort();
	if( pCurPage->m_UseIndex && pCurPage->pList->Count ){
		pCurPage->m_Thumb.OpenFolder(m_MyIndex, m_CurPage, pCurPage->pList->Count, GetCRC(pCurPage->m_Folder.c_str()));
		pCurPage->m_Thumb.SetSize(m_RectS.Right, m_RectS.Bottom);
	}
	UpdateStat();
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::LoadFile(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	if( pCurPage->m_Type >= 3 ){
		int xw = m_RectS.Right;
		int yw = m_RectS.Bottom;
		pBitmap->Width = xw;
		pBitmap->Height = yw;
		FillBitmap(pBitmap, clBtnFace);
		WORD Icon = 0;
		HICON hIcon = ::ExtractAssociatedIcon(HInstance, (LPSTR)pName, &Icon);
		if( hIcon != NULL ){
			TIcon *pIcon = new TIcon;
			pIcon->Handle = hIcon;
			pBitmap->Canvas->Draw((xw - pIcon->Width)/2, (yw >= 64) ? (yw - pIcon->Height)/2 : 0, pIcon);
			delete pIcon;
		}
		pBitmap->Canvas->Font->Height = -12;
		int fw = pBitmap->Canvas->TextWidth(pName);
		int fh = pBitmap->Canvas->TextHeight(pName);
		int tm = ::SetBkMode(pBitmap->Canvas->Handle, TRANSPARENT);
		int x = (xw - fw) / 2;
		if( x < 0 ) x = 0;
		pBitmap->Canvas->TextOut(x, hIcon != NULL ? yw - fh : (yw - fh)/2, pName);
		::SetBkMode(pBitmap->Canvas->Handle, tm);
	}
	else if( pCurPage->m_Type ){
		CDrawGroup *pGroup = new CDrawGroup;
		if( LoadTemplate(pGroup, pName, pBitmap->Canvas) == TRUE ){
			pBitmap->Width = pGroup->m_SX;
			pBitmap->Height = pGroup->m_SY;
			FillBitmap(pBitmap, pGroup->m_TransCol);
			MultProc();
			pGroup->Draw(pBitmap->Canvas);
		}
		pGroup->FreeItem();
		delete pGroup;
	}
	else {
		::LoadImage(pBitmap, pName);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::LoadImage(void)
{
	if( !m_Max ) return;
	if( pCurPage->pList == NULL ) LoadFileList();
	m_CurFile = -1;
	UpdateStat();

	CWaitCursor wait;
	MultProc();
	SetCurrentDirectory(pCurPage->m_Folder.c_str());
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	pBitmap->PixelFormat = pf24bit;
	pBitmap->Width = 16;
	pBitmap->Height = 16;

	MultProc();
	int i;
	int n = pCurPage->m_CurPage * m_Max;
	SetBitmapSize();
	for( i = 0; i < m_Max; i++, n++ ){
		TRect rc;
		GetRect(rc, i);
		if( n < pCurPage->pList->Count ){
			USHORT crc;
			LPCSTR pn = pCurPage->pList->Get(crc, n);
			if( !pCurPage->m_Thumb.LoadThumb(n, pCurPage->pBitmapS, rc, crc, pCurPage->m_Size[i]) ){
				LoadFile(pBitmap, pn);
				pCurPage->m_Size[i] = (pBitmap->Height << 16) + pBitmap->Width;
				Graphics::TBitmap *pBitmapT = CreateBitmap(m_RectS.Right, m_RectS.Bottom, -1);
				::SetStretchBltMode(pBitmapT->Canvas->Handle, HALFTONE);
				MultProcA();
				if( sys.m_FileViewKeep ){
					FillBitmap(pBitmapT, clGray);
					if( ((pBitmap->Width <= pBitmapT->Width) && (pBitmap->Height <= pBitmapT->Height)) ){
						pBitmapT->Canvas->Draw(0, 0, pBitmap);
					}
					else {
						KeepAspectDraw(pBitmapT->Canvas, pBitmapT->Width, pBitmapT->Height, pBitmap);
					}
				}
				else {
					pBitmapT->Canvas->StretchDraw(m_RectS, pBitmap);
				}
				pCurPage->m_Thumb.SaveThumb(n, pBitmapT, m_RectS, crc, pCurPage->m_Size[i]);
				pCurPage->pBitmapS->Canvas->CopyRect(rc, pBitmapT->Canvas, m_RectS);
				if( pBox[i] != NULL ){
					pBox[i]->Canvas->Draw(0, 0, pBitmapT);
				}
				delete pBitmapT;
			}
		}
		else {
			pCurPage->pBitmapS->Canvas->Brush->Style = bsSolid;
			pCurPage->pBitmapS->Canvas->Brush->Color = clWhite;
			pCurPage->pBitmapS->Canvas->FillRect(rc);
		}
		MultProc();
	}
	delete pBitmap;
	UpdateBitmap();
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::CopyStretchBitmap(Graphics::TBitmap *pBitmap)
{
	Graphics::TBitmap *pBmp = MakeCurrentBitmap();
	CopyAutoSize(pBitmap, pBmp);
	delete pBmp;
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::CopyRectBitmap(Graphics::TBitmap *pBitmap)
{
	int r = TRUE;

	TPicRectDlg *pBox = new TPicRectDlg(this);
	LoadCurrentBitmap(pBox->pBitmap);
	if( pBox->Execute(pBitmap) != TRUE ){
		r = FALSE;
	}
	delete pBox;
	return r;
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::CopyBitmap(Graphics::TBitmap *pBitmap)
{
	if( SBMode->Down ){
		return CopyStretchBitmap(pBitmap);
	}
	else {
		return CopyRectBitmap(pBitmap);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::LoadCurrentBitmap(Graphics::TBitmap *pBitmap)
{
	if( (m_CurFile < pCurPage->pList->Count) && (pCurPage->pList != NULL) ){
		MultProc();
		SetCurrentDirectory(pCurPage->m_Folder.c_str());
		MultProc();
		if( ::LoadImage(pBitmap, pCurPage->pList->Get(m_CurFile)) == FALSE ){
			FillBitmap(pBitmap, clWhite);
		}
	}
	else {
		FillBitmap(pBitmap, clWhite);
	}
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall TFileViewDlg::MakeCurrentBitmap(void)
{
	Graphics::TBitmap *pBmp = new Graphics::TBitmap();
	pBmp->PixelFormat = pf24bit;
	pBmp->Width = 320;
	pBmp->Height = 256;
	LoadCurrentBitmap(pBmp);
	return pBmp;
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::IsDrag(void)
{
	for( int i = 0; i < m_Max; i++ ){
		if( pBox[i]->Dragging() ) return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PBPaint(TObject *Sender)
{
	if( pCurPage == NULL ) return;
	if( pCurPage->pBitmapS == NULL ) return;

	int r = IsPBox(Sender);
	if( r >= 0 ){
		TRect sc;
		GetRect(sc, r);
		pBox[r]->Canvas->CopyRect(m_RectS, pCurPage->pBitmapS->Canvas, sc);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PBClick(int n)
{
	m_CurFile = n + (pCurPage->m_CurPage * m_Max);
	UpdateTitle();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PBMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( pCurPage->pList == NULL ) return;

	int r = IsPBox(Sender);
	if( r >= 0 ){
		PBClick(r);
		if( (Button == mbLeft) && (m_CurFile < pCurPage->pList->Count) ){
			m_XX = X; m_YY = Y;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PBMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( pCurPage->pList == NULL ) return;
	if( m_CurFile >= pCurPage->pList->Count ) return;

	TShiftState sc1, sc2;
	sc1 << ssLeft;
	sc2 << ssLeft;
	sc1 *= Shift;
	if( sc1 == sc2 ){			// Left button
		if( (ABS(m_XX-X) + ABS(m_YY-Y)) >= 10 ){
			((TPaintBox *)Sender)->BeginDrag(TRUE,0);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PBDblClick(TObject *Sender)
{
	if( pCurPage->pList == NULL ) return;
	if( m_CurFile >= pCurPage->pList->Count ) return;
	if( Mmsstv->SBTX->Down ) return;

	if( pCurPage->m_Type ){     // テンプレート
		CWaitCursor w;
		Mmsstv->AdjustPage(pgTemp);
		AnsiString as;
		GetCurFileName(as);
		if( pCurPage->m_Type == 4 ){
			Mmsstv->DropOle(-1, 0, as.c_str(), (Sender == KP) ? 0 : 1);
		}
		else if( pCurPage->m_Type == 3 ){    // DLL
			Mmsstv->DropLib(-1, 0, as.c_str());
		}
		else if( pCurPage->m_Type == 2 ){
			CDrawGroup Group;
			LoadTemplate(&Group, as.c_str(), NULL);
			Mmsstv->AddTemplate(&Group);
		}
		else {
			LoadTemplate(&Mmsstv->DrawMain, as.c_str(), NULL);
		}
		Mmsstv->UpdatePic();
	}
	else if( SBMode->Down ){
		KTXSClick(Sender);
	}
	else {
		KTXClick(Sender);
	}
#if 0
	if( Sender->ClassNameIs("TPaintBox") ){
		TPaintBox *pBox = (TPaintBox *)Sender;
		if( pBox->Dragging() ) pBox->EndDrag(FALSE);
	}
#endif
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PanelDragOver(TObject *Sender,
	  TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = FALSE;
	switch(pCurPage->m_Type){
		case 0:         // 画像
			if( (Source == Mmsstv->PBoxTX)||(Source == Mmsstv->PBoxHist) ) Accept = TRUE;
			break;
		case 1:         // テンプレート
		case 2:
			if( Source == Mmsstv->PBoxTemp ) Accept = TRUE;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PanelDragDrop(TObject *Sender,
	  TObject *Source, int X, int Y)
{
	int n = IsPBox(Sender);
	if( n >= 0 ){
		n += (pCurPage->m_CurPage * m_Max);
	}
	AnsiString Name;
	GetWriteFileName(Name, n);
	switch(pCurPage->m_Type){
		case 0:
			{
			Graphics::TBitmap *pBitmap = NULL;
			if( Source == Mmsstv->PBoxTX ){
				pBitmap = Mmsstv->pBitmapTXM;
			}
			else if( Source == Mmsstv->PBoxHist ){
				pBitmap = Mmsstv->pBitmapHist;
			}
			if( pBitmap != NULL ){
				if( Mmsstv->SaveBitmapMenu(pBitmap, Name.c_str(), pCurPage->m_Folder.c_str()) == TRUE ){
					UpdateList();
				}
			}
			}
			break;
		case 1:
		case 2:
			if( Source == Mmsstv->PBoxTemp ){
				if( Mmsstv->SaveTemplateMenu(&Mmsstv->DrawMain, Name.c_str(), pCurPage->m_Folder.c_str(), (pCurPage->m_Type == 2) ? 1 : 0) == TRUE ){
					UpdateList();
				}
			}
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UDClick(TObject *Sender, TUDBtnType Button)
{
	pCurPage->m_CurPage = UD->Position;
	LoadImage();
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UDMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		int n = UD->Position;
		int w = UD->Width/2;
		int m = n % 5;
		if( X >= w ){
			n += (5 - m);
		}
		else if( m ){
			n -= m;
		}
		else {
			n -= 5;
		}
		int max = UD->Max + 1;
		while( n < 0 ) n += max;
		while( n >= max ) n -= max;
		n = n - (n % 5);
		UD->Position = SHORT(n);
		UDClick(NULL, btNext);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UpdateList(void)
{
	LoadFileList();
	LoadImage();
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::SBUpdateClick(TObject *Sender)
{
	if( pCurPage->m_Thumb.IsOpen() ) pCurPage->m_Thumb.Reset();
	UpdateList();
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::NewFolder(void)
{
	int r = FALSE;

	TOpenDialog *pDialog = Mmsstv->OpenDialog;
	pDialog->Options >> ofCreatePrompt;
	pDialog->Options >> ofFileMustExist;
	if( MsgEng ){
		pDialog->Title = "Set folder";
	}
	else {
		pDialog->Title = "フォルダの選択";
	}
	char bf[512];
	sprintf(bf, "%s|%s|%s|%s", GetPicFilter(), GetTempFilter(), GetLibFilter(), "OLE objects(*.*)|*.*");
	pDialog->Filter = bf;
	pDialog->FileName = "Dummy";
	pDialog->DefaultExt = "jpg";
	pDialog->InitialDir = pCurPage->m_Folder.c_str();
	pDialog->FilterIndex = pCurPage->m_Type + 1;
	SetDisPaint();
	NormalWindow(Mmsstv);
	if( Mmsstv->OpenDialogExecute(TRUE) == TRUE ){
		SetDirName(bf, AnsiString(pDialog->FileName).c_str());	//ja7ude 0428
		pCurPage->m_Folder = bf;
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char name[_MAX_FNAME];
		char ext[_MAX_EXT];
		AnsiString	Name;
		::_splitpath( AnsiString(pDialog->FileName).c_str(), drive, dir, name, ext );
		m_Name = name;
		m_Name += ext;
#if 1
		pCurPage->m_Type = pDialog->FilterIndex - 1;
#else
		LPCSTR pExt = GetEXT(m_Name.c_str());
		if( !strcmpi(pExt, "MTI") ){
			pCurPage->m_Type = 2;
		}
		else if( !strcmpi(pExt, "MTM") ){
			pCurPage->m_Type = 1;
		}
		else if( !strcmpi(pExt, "DLL") ){
			pCurPage->m_Type = 3;
		}
		else if( IsPic(pExt) ){
			pCurPage->m_Type = 0;
		}
		else {
			pCurPage->m_Type = 4;
		}
#endif
		r = TRUE;
	}
	ResDisPaint();
	TopWindow(Mmsstv);
	return r;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::SBNewClick(TObject *Sender)
{
	if( NewFolder() == TRUE ){
		UpdateList();
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::FormClose(TObject *Sender,
	  TCloseAction &Action)
{
	Mmsstv->m_FileViewClose = 1;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KTXSClick(TObject *Sender)
{
	Mmsstv->pBitmapTXM = RemakeBitmap(Mmsstv->pBitmapTXM, -1);
	if( CopyStretchBitmap(Mmsstv->pBitmapTXM) == TRUE ){
		Mmsstv->AdjustPage(-1);
		Mmsstv->UpdatePic();
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KTXClick(TObject *Sender)
{
	if( CopyRectBitmap(Mmsstv->pBitmapTXM) == TRUE ){
		Mmsstv->AdjustPage(-1);
		Mmsstv->UpdatePic();
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KCClick(TObject *Sender)
{
	Graphics::TBitmap *pBmp = MakeCurrentBitmap();
	Mmsstv->CopyBitmap(pBmp);
	delete pBmp;
}
//---------------------------------------------------------------------------
int __fastcall TFileViewDlg::GetWriteFileName(AnsiString &as, int no)
{
	if( (no >= 0) && (no < pCurPage->pList->Count) ){
		as = pCurPage->pList->Get(no);
		return TRUE;
	}
	else {
		SYSTEMTIME tim;
		GetLocal(&tim);
		char bf[128];
		sprintf(bf, "%02u%02u%02u%02u%02u%02u",
			tim.wYear % 100,
			tim.wMonth,
			tim.wDay,
			tim.wHour,
			tim.wMinute,
			tim.wSecond
		);
		as = bf;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KPCClick(TObject *Sender)
{
	if( m_CurFile < 0 ) return;

	AnsiString Name;
	GetWriteFileName(Name, m_CurFile);
	Graphics::TBitmap *pBmp = new Graphics::TBitmap();
	if( Mmsstv->PasteBitmap(pBmp, 1) == TRUE ){
		if( Mmsstv->SaveBitmapMenu(pBmp, Name.c_str(), pCurPage->m_Folder.c_str()) == TRUE ){
			UpdateList();
		}
	}
	delete pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KTTClick(TObject *Sender)
{
	m_TitleBar = m_TitleBar ? 0 : 1;
	UpdateTitlebar();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::PopupPopup(TObject *Sender)
{
	KTT->Checked = m_TitleBar;
	KTS->Checked = sys.m_FileViewShowSize;
	KTK->Checked = sys.m_FileViewKeep;
	switch(m_SSize){
		case 0:
			KS1->Checked = TRUE;
			break;
		case 1:
			KS2->Checked = TRUE;
			break;
		case 2:
			KS3->Checked = TRUE;
			break;
		default:
			KS4->Checked = TRUE;
			break;
	}
	KS->Enabled = m_TitleBar;
	KT->Enabled = pCurPage->pList->Count && (!pCurPage->m_Type);
	int f = GetType();
	KD->Enabled = f;
	KP->Enabled = f;
	switch(pCurPage->m_Type){
		case 1:
			KP->Caption = MsgEng ? "Copy to Template" : "テンプレートにコピー";
			break;
		default:
			KP->Caption = MsgEng ? "Paste to Template" : "テンプレートに貼り付け";
			break;
	}
	KPO->Enabled = f && (!pCurPage->m_Type || (pCurPage->m_Type == 4));
	f = (f && !pCurPage->m_Type) ? 1 : 0;
	KC->Enabled = f;
	KV->Enabled = f;
	KPOL->Enabled = f;
	KPE->Enabled = f;
	if( f ) f = !Mmsstv->SBTX->Down;
	KTX->Enabled = f;
	KTXS->Enabled = f;
	KNP->Enabled = (m_MaxPage < FPAGEMAX) ? TRUE : FALSE;
	KDP->Enabled = (m_MaxPage >= 2) ? TRUE : FALSE;
	KRP->Enabled = KDP->Enabled;
	KPI->Checked = pCurPage->m_UseIndex;
	KPC->Enabled = !pCurPage->m_Type && ::IsClipboardFormatAvailable(CF_BITMAP);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyDown(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::FormKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyUp(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KTKClick(TObject *Sender)
{
	sys.m_FileViewKeep = sys.m_FileViewKeep ? FALSE : TRUE;
	Mmsstv->LoadAllFileView();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::FormDeactivate(TObject *Sender)
{
	if( !IsDrag() ){
		m_CurFile = -1;
		UpdateTitle();
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KTSClick(TObject *Sender)
{
	sys.m_FileViewShowSize = sys.m_FileViewShowSize ? FALSE : TRUE;
	UpdateTitle();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::UpdateSize(int sw)
{
	SetThumbnailSize(m_RectS, m_SSize);
	if( sw ){
		for( int i = 0; i < m_MaxPage; i++ ){
			CFILEV *cp = pFileV[i];
			if( cp->m_Thumb.IsOpen() ){
				cp->m_Thumb.SetSize(m_RectS.Right, m_RectS.Bottom);
			}
			delete cp->pBitmapS;
			cp->pBitmapS = NULL;
		}
		FormResize(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KSClick(TObject *Sender)
{
	if( Sender == KS1 ){
		m_SSize = 0;
	}
	else if( Sender == KS2 ){
		m_SSize = 1;
	}
	else if( Sender == KS3 ){
		m_SSize = 2;
	}
	else {
		m_SSize = 3;
	}
	UpdateSize(1);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::MakeThImage(int mm)
{
	int mc = mm*mm;
	if( (m_CurFile < 0) || (pCurPage->pList == NULL) ) return;

	Mmsstv->pBitmapTXM = RemakeBitmap(Mmsstv->pBitmapTXM, -1);
	FillBitmap(Mmsstv->pBitmapTXM, sys.m_PicClipColor);
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	Graphics::TBitmap *pBitmapQ = new Graphics::TBitmap();

	MultProc();
	SetCurrentDirectory(pCurPage->m_Folder.c_str());
	MultProc();
	int Y1 = 0;
//    int Y2 = Mmsstv->pBitmapTXM->Height;
	int Y2 = Mmsstv->m_TXPH;
	int X2 = Mmsstv->pBitmapTXM->Width;
	if( Mmsstv->SBUseTemp->Down && Mmsstv->DrawMain.m_Cnt ){
		Mmsstv->DrawMain.GetColorBarPos(Y1, Y2);
	}
	int XW = X2 / mm;
	int YW = (Y2-Y1) / mm;
	int x, y, i;
	int n = m_CurFile;
	for( i = 0; i < mc; i++, n++ ){
		MultProc();
		if( n >= pCurPage->pList->Count ) n = 0;
		::LoadImage(pBitmap, pCurPage->pList->Get(n));
		x = (i % mm) * XW;
		y = Y1 + ((i / mm) * YW);
		if( ((i % mm) == (mm - 1)) && (x + XW) != X2 ){
			pBitmapQ->Width = X2 - x;
		}
		else {
			pBitmapQ->Width = XW;
		}
		if( ((i / mm) == (mm - 1)) && (y + YW) != Y2 ){
			pBitmapQ->Height = Y2 - y;
		}
		else {
			pBitmapQ->Height = YW;
		}
		MultProc();
		StretchCopyBitmapHW(pBitmapQ, pBitmap);
		Mmsstv->pBitmapTXM->Canvas->Draw(x, y, pBitmapQ);
		MultProc();
	}
	delete pBitmapQ;
	delete pBitmap;
	Mmsstv->UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KT4Click(TObject *Sender)
{
	int n;
	if( Sender == KT4 ){
		n = 2;
	}
	else if( Sender == KT9 ){
		n = 3;
	}
	else {
		n = 4;
	}
	MakeThImage(n);
	Mmsstv->AdjustPage(pgTX);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KDClick(TObject *Sender)
{
	if( (m_CurFile < 0) || (pCurPage->pList == NULL) ) return;

	SetCurrentDirectory(pCurPage->m_Folder.c_str());
	AnsiString as;
	GetCurFileName(as);
	if( YesNoMB( ( Font->Charset != SHIFTJIS_CHARSET )?"Delete '%s'":"'%s'を消去します", as.c_str() ) == IDYES ){
		if( unlink(as.c_str()) ){
			ErrorFWrite(as.c_str());
		}
		else {
			UpdateList();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KVClick(TObject *Sender)
{
	Graphics::TBitmap *pBmp = MakeCurrentBitmap();
	TZoomViewDlg *pBox = new TZoomViewDlg(this);
	pBox->SetInitSize(pBmp->Width, pBmp->Height);
	pBox->Execute(pBmp, FALSE);
	delete pBox;
	delete pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KPClick(TObject *Sender)
{
	if( pCurPage->m_Type ){
		PBDblClick(Sender);
	}
	else {
		Graphics::TBitmap *pBmp = MakeCurrentBitmap();
		Mmsstv->DropPic(-1, 0, pBmp, (Sender == KP) ? 1 : 2);
		delete pBmp;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KPOLClick(TObject *Sender)
{
	if( (m_CurFile < 0) || (pCurPage->pList == NULL) ) return;

	Mmsstv->AdjustPage(pgTemp);
	AnsiString as;
	GetCurFileName(as);
	if( IsFile(as.c_str()) ){
		CDrawOle *pDraw = (CDrawOle *)Mmsstv->DrawMain.MakeItem(CM_OLE);
		LPCSTR pExt = GetEXT(as.c_str());
		if( !strcmpi(pExt, "BMP")){
			pDraw->LoadFromFile(-1, 0, as.c_str());
		}
		else {
			Graphics::TBitmap *pBmp = MakeCurrentBitmap();
			pDraw->LoadFromBitmap(-1, 0, pBmp);
			if( !strcmpi(pExt, "JPG") ){
				pDraw->m_Trans = 0;
				pDraw->m_Stretch = 1;
			}
			delete pBmp;
		}
		Mmsstv->AddItem(pDraw, 0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KPEClick(TObject *Sender)
{
	if( (m_CurFile < 0) || (pCurPage->pList == NULL) ) return;

	char fname[256];

	sprintf(fname, "%s%s", pCurPage->m_Folder.c_str(), pCurPage->pList->Get(m_CurFile));
	g_ExecPB.Exec(fname, (HWND)Handle, CM_EDITEXIT, m_CurPage);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::OnEditExit(TMessage Message)
{
	if( g_ExecPB.IsChanged() ) SBUpdateClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::TabSChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	m_CurFile = -1;
	m_CurPage = pTabS->TabIndex;
	pCurPage = pFileV[m_CurPage];
	UD->Max = SHORT(pCurPage->m_MaxPage-1);
	UD->Position = SHORT(pCurPage->m_CurPage);
	if( (pCurPage->pList == NULL) || (pCurPage->pBitmapS == NULL) ){
		LoadImage();
	}
	UpdateBitmap();
	UpdateStat();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::GetCurFileName(AnsiString &as)
{
	if( m_CurFile >= 0 ){
		as = pCurPage->m_Folder.c_str();
		as += pCurPage->pList->Get(m_CurFile);
	}
}
//---------------------------------------------------------------------------
// 新規ページの作成
void __fastcall TFileViewDlg::KNPClick(TObject *Sender)
{
	if( m_MaxPage < FPAGEMAX ){
		m_CurFile = -1;
		CFILEV *cp = new CFILEV;
		pFileV[m_MaxPage] = cp;
		m_MaxPage++;
		if( m_MaxPage <= 2 ){
			Mmsstv->ReOpenFileView(this);
		}
		else {
			AnsiString as;
			GetPageName(as, m_MaxPage - 1);
			pTabS->Tabs->Add(as);
		}
	}
}
//---------------------------------------------------------------------------
// ページの名前を得る
void __fastcall TFileViewDlg::GetPageName(AnsiString &as, int n)
{
	CFILEV *cp = pFileV[n];
	if( cp->m_Name.IsEmpty() ){
		char bf[32];
		sprintf(bf, "P%u", n + 1);
		as = bf;
	}
	else {
		as = cp->m_Name;
	}
}
//---------------------------------------------------------------------------
// ページの削除
void __fastcall TFileViewDlg::KDPClick(TObject *Sender)
{
	if( m_MaxPage <= 1 ) return;

	AnsiString as;
	GetPageName(as, m_CurPage);
	if( YesNoMB( MsgEng ? "Delete '%s', are you sure?" : "'%s'を削除します. よろしいですか?", as.c_str()) != IDYES ) return;

	m_CurFile = -1;
	pFileV[m_CurPage]->Delete();
	if( m_CurPage < (m_MaxPage - 1) ){  // 途中のページ
		int i;
		for( i = m_CurPage; i < (m_MaxPage - 1); i++ ){
			pFileV[i] = pFileV[i+1];
		}
		pFileV[i] = NULL;
	}
	else {                              // 最後のページ
		pFileV[m_CurPage] = NULL;
		m_CurPage--;
	}
	m_MaxPage--;
	pCurPage = pFileV[m_CurPage];
	Mmsstv->ReOpenFileView(this);
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KRPClick(TObject *Sender)
{
	AnsiString as = pCurPage->m_Name;
	if( InputMB(NULL, MsgEng ? "Enter page name" : "名前を入力", as) == TRUE ){
		pCurPage->m_Name = as.c_str();
		GetPageName(as, m_CurPage);
		pTabS->Tabs->Strings[m_CurPage] = as;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KBPClick(TObject *Sender)
{
	InvMenu(KBP);
	if( pTabS != NULL ){
		pTabS->TabPosition = KBP->Checked ? tpBottom : tpTop;
		Mmsstv->ReOpenFileView(this);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::Reset(void)
{
	for( int i = 0; i < m_MaxPage; i++ ){
		if( pFileV[i]->m_Thumb.IsOpen() ){
			pFileV[i]->m_Thumb.Reset();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileViewDlg::KPIClick(TObject *Sender)
{
	pCurPage->m_UseIndex = pCurPage->m_UseIndex ? 0 : 1;
	if( pCurPage->m_UseIndex ){
		LoadFileList();
	}
	else if( pCurPage->m_Thumb.IsOpen() ){
		pCurPage->m_Thumb.CloseFolder();
	}
}
//
//
//***************************************************************************
//CFILEVクラス
//***************************************************************************
//
//
__fastcall CFILEV::CFILEV()
{
	pBitmapS = NULL;
	pList = NULL;
	m_CurPage = 0;
	m_MaxPage = 1;
	m_UseIndex = 1;
	m_Name = "";
	m_Type = 0;
}
//---------------------------------------------------------------------------
void __fastcall CFILEV::Delete(void)
{
	if( pBitmapS != NULL ) delete pBitmapS;
	delete pList;
	pList = NULL;
}
//
//
//***************************************************************************
//CThumbクラス
//***************************************************************************
//
//
__fastcall CThumb::CThumb()
{
	m_UpdateBmp = 0;
	pBitmap = NULL;
	pITBL = NULL;
	m_Top = -1;
}
//---------------------------------------------------------------------------
void __fastcall CThumb::SetSize(int sx, int sy)
{
	m_SizeX = sx;
	m_SizeY = sy;
	m_XW = m_SizeX * THUMBWIND;
	if( pBitmap != NULL ) delete pBitmap;
	pBitmap = new Graphics::TBitmap;
	AdjustBitmapFormat(pBitmap);
	pBitmap->Width = m_XW;
	pBitmap->Height = m_SizeY;
	m_Top = -1;
	m_UpdateBmp = 0;
}
//---------------------------------------------------------------------------
void __fastcall CThumb::OpenFolder(int index, int page, int size, USHORT crc)
{
	CloseFolder();
	if( !size ) return;
	m_FolderIndex = index * 10 + page;
	m_FileCount = size;
	size++;
	pITBL = new DIND[size];
	memset(pITBL, 0, sizeof(DIND)*size);
	char name[256];
	sprintf(name, "%sFindex\\I%02u.idx", BgnDir, m_FolderIndex);
	FILE *fp = fopen(name, "rb");
	MultProc();
	if( fp != NULL ){
		fread(pITBL, sizeof(DIND), size, fp);
		fclose(fp);
		if( (pITBL[0].crc != crc) || (pITBL[0].size != THUMBFID) ){
			memset(pITBL, 0, sizeof(DIND)*size);
		}
	}
	else {
		sprintf(name, "%sFindex", BgnDir);
		mkdir(name);
	}
	pITBL[0].crc = crc;
	pITBL[0].size = THUMBFID;
	m_UpdateIdx = 0;
	m_Top = -1;
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall CThumb::CloseFolder(void)
{
	if( pBitmap != NULL ){
		CloseBitmap();
		delete pBitmap;
		pBitmap = NULL;
	}
	if( pITBL != NULL ){
		MultProc();
		if( m_UpdateIdx ){
			char name[256];
			sprintf(name, "%sFindex\\I%02u.idx", BgnDir, m_FolderIndex);
			FILE *fp = fopen(name, "wb");
			if( fp != NULL ){
				fwrite(pITBL, sizeof(DIND), m_FileCount+1, fp);
				fclose(fp);
			}
			else {
				ErrorFWrite(name);
			}
			m_UpdateIdx = 0;
		}
		delete pITBL;
		pITBL = NULL;
		MultProc();
	}
}
//---------------------------------------------------------------------------
void __fastcall CThumb::CloseBitmap(void)
{
	if( m_UpdateBmp && (pBitmap != NULL) ){
		if( m_Top >= 0 ){
			MultProc();
			char name[256];
			sprintf(name, "%sFindex\\B%02u%u.bmp", BgnDir, m_FolderIndex, m_Top);
			::SaveBitmap(pBitmap, name);
			MultProc();
		}
		m_UpdateBmp = 0;
	}
}
//---------------------------------------------------------------------------
int __fastcall CThumb::UpdateBitmap(int n)
{
	if( pBitmap == NULL ) return FALSE;

	CloseBitmap();
	n /= THUMBWIND;
	if( n == m_Top ) return TRUE;
	m_Top = n;
	char name[256];
	sprintf(name, "%sFindex\\B%02u%u.bmp", BgnDir, m_FolderIndex, n);
	MultProc();
	int r = FALSE;
	m_UpdateBmp = 1;
	if( IsFile(name) ){
		::LoadBitmap(pBitmap, name);
		MultProc();
		if( (pBitmap->Width != m_XW) || (pBitmap->Height != m_SizeY) ){
			pBitmap->Width = m_XW;
			pBitmap->Height = m_SizeY;
		}
		else {
			m_UpdateBmp = 0;
			r = TRUE;
		}
	}
	if( r == FALSE ){
		n *= THUMBWIND;
		for( int i = n; i < (n + THUMBWIND); i++ ){
			if( i < m_FileCount ) pITBL[i+1].crc = 0;
		}
	}
	MultProc();
	return r;
}
//---------------------------------------------------------------------------
void __fastcall CThumb::SaveThumb(int n, Graphics::TBitmap *pbmp, TRect rc, USHORT crc, int size)
{
	if( pITBL == NULL ) return;
	if( pBitmap == NULL ) return;
	if( n < m_FileCount ){
		if( (n / THUMBWIND) != m_Top ) UpdateBitmap(n);
		int x = (n % THUMBWIND) * m_SizeX;
		TRect tc;
		tc.Left = x; tc.Top = 0;
		tc.Right = x + m_SizeX; tc.Bottom = m_SizeY;
		pBitmap->Canvas->CopyRect(tc, pbmp->Canvas, rc);
		n++;
		pITBL[n].crc = crc;
		pITBL[n].size = size;
		m_UpdateBmp = 1;
		m_UpdateIdx = 1;
	}
}
//---------------------------------------------------------------------------
int __fastcall CThumb::LoadThumb(int n, Graphics::TBitmap *pbmp, TRect rc, USHORT crc, int &size)
{
	if( pITBL == NULL ) return FALSE;
	if( pBitmap == NULL ) return FALSE;
//    if( rc.Bottom != m_SizeY ) return FALSE;
	if( (n < m_FileCount) && UpdateBitmap(n) ){
		if( crc == pITBL[n+1].crc ){
			int x = (n % THUMBWIND) * m_SizeX;
			TRect sc;
			sc.Left = x; sc.Top = 0;
			sc.Right = x + m_SizeX; sc.Bottom = m_SizeY;
			pbmp->Canvas->CopyRect(rc, pBitmap->Canvas, sc);
			size = pITBL[n+1].size;
			return TRUE;
		}
	}
	return FALSE;
}
//---------------------------------------------------------------------------
void __fastcall CThumb::Reset(void)
{
	if( pITBL != NULL ){
		memset(&pITBL[1], 0, sizeof(DIND)*m_FileCount);
		m_UpdateIdx = 1;
	}
}
//
//
//***************************************************************************
//CFILELクラス
//***************************************************************************
//
//
//---------------------------------------------------------------------------
void __fastcall CFILEL::Alloc(void)
{
	int am = AMax ? (AMax * 2) : 64;
	CFD *pN = new CFD[am];
	if( pBase != NULL ){
		memcpy(pN, pBase, sizeof(CFD)*Count);
		delete pBase;
	}
	pBase = pN;
	AMax = am;
}
//---------------------------------------------------------------------------
void __fastcall CFILEL::Delete(void)
{
	if( pBase != NULL ){
		CFD *cp = pBase;
		for( int i = 0; i < Count; i++, cp++ ) delete cp->pName;
		delete pBase;
	}
	pBase = NULL;
	AMax = 0;
	Count = 0;
}
//---------------------------------------------------------------------------
void __fastcall CFILEL::Add(LPCSTR pn, USHORT crc)
{
	if( Count >= AMax ) Alloc();
	CFD *cp = &pBase[Count];
	cp->pName = StrDupe(pn);
	cp->crc = crc;
	Count++;
}
//---------------------------------------------------------------------------
LPCSTR __fastcall CFILEL::Get(USHORT &crc, int n)
{
	if( n >= Count ) return NULL;
	CFD *cp = &pBase[n];
	crc = cp->crc;
	return cp->pName;
}
//---------------------------------------------------------------------------
LPCSTR __fastcall CFILEL::Get(int n)
{
	if( n >= Count ) return NULL;
	return pBase[n].pName;
}
//---------------------------------------------------------------------------
static int _cmp(const void *s, const void *t)
{
	CFD *sp = (CFD *)s;
	CFD *tp = (CFD *)t;
	return strcmp(sp->pName, tp->pName);
}
//---------------------------------------------------------------------------
void __fastcall CFILEL::Sort(void)
{
	if( Count < 2 ) return;

	qsort(pBase, Count, sizeof(CFD), _cmp);
}
//---------------------------------------------------------------------------

