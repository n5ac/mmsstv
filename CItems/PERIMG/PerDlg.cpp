//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "PerDlg.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
#if UseHWND
__fastcall TPerDlgBox::TPerDlgBox(HWND hWnd)
	: TForm(hWnd)
#else
__fastcall TPerDlgBox::TPerDlgBox(TComponent* AOwner)
	: TForm(AOwner)
#endif
{
	m_Loaded = 0;
	m_DisEvent = 1;
	if( sys.m_MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;
		Caption = VER"  -  "TTL;
		CancelBtn->Caption = "Cancel";
		DefBtn->Caption = "Default";
		L1->Caption = "Rot.X";
		L2->Caption = "Rot.Y";
		L3->Caption = "Rot.Z";
		L4->Caption = "Move.Z";
		L5->Caption = "Move.Y";
		L6->Caption = "Move.X";
		L7->Caption = "ViewP.";
		L8->Caption = "Scale.X";
		L9->Caption = "Scale.Y";
		L10->Caption = "Rot";
		L11->Caption = "Defo.";
		RGSRC->Caption = "Source";
		RGSRC->Items->Strings[0] = "History";
		RGSRC->Items->Strings[1] = "Loaded image";
		RGSRC->Items->Strings[2] = "Overlay";
		ViewBtn->Caption = "Show";
		PasteBtn->Caption = "Paste";
		DefBtn->Hint = "Return to the default";
		LoadBtn->Hint = "Load image file";
		PasteBtn->Hint = "Load from the clipboard";
		ViewBtn->Hint = "Show source image";
		SBR->Hint = "Return to the default";
		L12->Caption = "Frame W.";
		CBF->Items->Strings[0] = "None";
		PC->Hint = "Set frame color";
	}
	else {
		Caption = VER"  -  "TTLJ;
	}
	m_pBitmap = NULL;
	m_pItem = NULL;
	m_XC = -1;
	m_Wave = 0;
    for( int i = 0; i < AN(m_DirTB); i++ ){
		m_DirTB[i] = 1;
    }
    m_hClipNext = ::SetClipboardViewer(Handle);
	ShowHint = sys.m_WinNT;
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::FormDestroy(TObject *Sender)
{
	::ChangeClipboardChain(Handle, m_hClipNext);
	if( m_pBitmap != NULL ) delete m_pBitmap;
}
//---------------------------------------------------------------------------
int __fastcall TPerDlgBox::Execute(CItem *pItem)
{
	m_pItem = pItem;
	m_Item.Copy(pItem);
	if( m_Item.m_pBitmap == NULL ) m_Item.CopyFromHistory();
	m_Back = m_Item.m_Back;
	LPSPERSPECT psp = &m_Item.m_sperspect;

	MultProc();

	TBAX->Position = psp->ax * 20.0 + 0.5;
	TBAY->Position = psp->ay * 20.0 + 0.5;
	TBPX->Position = psp->px * 40.0;
	TBPY->Position = psp->py * 40.0;
	TBPZ->Position = psp->pz * 40.0;
	TBRX->Position = psp->rx;
	TBRY->Position = psp->ry;
	TBRZ->Position = psp->rz;
	TBS->Position = psp->s * 10.0 + 0.5;
	TBR->Position = psp->r;
	RGSRC->ItemIndex = psp->flag & 0x0000ffff;
	SBSC->Position = SHORT(m_Item.m_SC);
	m_Wave = psp->flag >> 16;
	CBF->ItemIndex = pItem->m_Frame;
	PC->Color = pItem->m_FrameCol;

	UpdateWaveBtn(0);
	UpdateBitmap();
	UpdateBtn();
	m_DisEvent = 0;
	MultProc();
	if( ShowModal() == IDOK ){
		MultProc();
		psp = &pItem->m_sperspect;
		psp->ax = double(TBAX->Position)/20.0;
		psp->ay = double(TBAY->Position)/20.0;
		psp->px = double(TBPX->Position)/40.0;
		psp->py = double(TBPY->Position)/40.0;
		psp->pz = double(TBPZ->Position)/40.0;
		psp->rx = TBRX->Position;
		psp->ry = TBRY->Position;
		psp->rz = TBRZ->Position;
		psp->s = double(TBS->Position)/10.0;
		psp->r = TBR->Position;
		pItem->m_SC = SBSC->Position;
		pItem->m_Frame = CBF->ItemIndex;
		pItem->m_FrameCol = PC->Color;

		UpdateWaveBtn(1);
		psp->flag = RGSRC->ItemIndex | (m_Wave << 16);
		if( (psp->flag & 0x0000ffff) && m_Loaded && (m_Item.m_pBitmap != NULL) ){
			pItem->CopySource(m_Item.m_pBitmap);
		}
		return TRUE;
	}
	MultProc();
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TPerDlgBox::UpdateWaveBtn(int dir)
{
	TSpeedButton *tbl[]={
		SB, SBZ1, SBZ2, SBZ3, SBZ4, SBZ5, SBZ6, SBZ7,
		SBZ8, SBZ9, SBZ10, SBZ11, SBZ12, SBZ13, SBZ14,
        SBZ15, SBZ16, SBZ17, SBZ18, SBZ19, SBZ20, SBZ21,
        SBZ22, SBZ23, SBZ24, SBZ25, SBZ26, SBZ27, SBZ28,
        SBZ29, SBZ30, SBZ31, SBZ32, SBZ33, SBZ34, SBZ35,
        SBZ36, SBZ37, SBZ38,
		NULL,
	};
	if( dir ){
		for( int i = 0; tbl[i] != NULL; i++ ){
			if( tbl[i]->Down ){
				m_Wave = i;
				break;
			}
		}
	}
	else {
		if( m_Wave >= (AN(tbl)-1) ) m_Wave = 0;
		tbl[m_Wave]->Down = TRUE;
	}
}
//---------------------------------------------------------------------
void __fastcall TPerDlgBox::UpdateBitmap(void)
{
	if( m_Item.m_pBitmap == NULL ) return;

	m_Item.m_sperspect.ax = double(TBAX->Position)/20.0;
	m_Item.m_sperspect.ay = double(TBAY->Position)/20.0;

	m_Item.m_sperspect.px = double(TBPX->Position)/40.0;
	m_Item.m_sperspect.py = double(TBPY->Position)/40.0;
	m_Item.m_sperspect.pz = double(TBPZ->Position)/40.0;

	m_Item.m_sperspect.rx = TBRX->Position;
	m_Item.m_sperspect.ry = TBRY->Position;
	m_Item.m_sperspect.rz = TBRZ->Position;
	m_Item.m_sperspect.s = double(TBS->Position)/10.0;
	m_Item.m_sperspect.r = TBR->Position;
	m_Item.m_SC = SBSC->Position;
	m_Item.m_Frame = CBF->ItemIndex;
	m_Item.m_FrameCol = PC->Color;
	m_pBitmap = m_Item.Perspect(m_pBitmap, m_Item.m_pBitmap);
	UpdateHint();
	UpdateTB();
	if( ShowHint ) Application->CancelHint();
}
//---------------------------------------------------------------------
void __fastcall TPerDlgBox::PBoxPaint(TObject *Sender)
{
	DrawBitmap(PBox, m_pBitmap);
}
//---------------------------------------------------------------------
void __fastcall TPerDlgBox::UpdateHint(void)
{
	if( sys.m_WinNT ){
		char bf[128];
		wsprintf(bf, "VP: %d", TBS->Position);
		TBS->Hint = bf;
		wsprintf(bf, "SX: %d", TBAX->Position);
		TBAX->Hint = bf;
		wsprintf(bf, "SY: %d", TBAY->Position);
		TBAY->Hint = bf;
		wsprintf(bf, "PX: %d", TBPX->Position);
		TBPX->Hint = bf;
		wsprintf(bf, "PY: %d", TBPY->Position);
		TBPY->Hint = bf;
		wsprintf(bf, "PZ: %d", TBPZ->Position);
		TBPZ->Hint = bf;
		wsprintf(bf, "RX: %d", TBRX->Position);
		TBRX->Hint = bf;
		wsprintf(bf, "RY: %d", TBRY->Position);
		TBRY->Hint = bf;
		wsprintf(bf, "RZ: %d", TBRZ->Position);
		TBRZ->Hint = bf;
		wsprintf(bf, "R: %d", TBR->Position);
		TBR->Hint = bf;
		wsprintf(bf, "%d", SBSC->Position);
		SBSC->Hint = bf;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::DefBtnClick(TObject *Sender)
{
#if 0   // for to get default
	FILE *fp = fopen("TEST.TXT", "wt");
	fprintf(fp, "%d\n", TBAX->Position);
	fprintf(fp, "%d\n", TBAY->Position);
	fprintf(fp, "%d\n", TBPX->Position);
	fprintf(fp, "%d\n", TBPY->Position);
	fprintf(fp, "%d\n", TBPZ->Position);
	fprintf(fp, "%d\n", TBRX->Position);
	fprintf(fp, "%d\n", TBRY->Position);
	fprintf(fp, "%d\n", TBRZ->Position);
	fprintf(fp, "%d\n\n", TBS->Position);

	SPERSPECT *psp = &m_Item.m_sperspect;
	fprintf(fp, "%f\n", psp->ax);
	fprintf(fp, "%f\n", psp->ay);
	fprintf(fp, "%f\n", psp->px);
	fprintf(fp, "%f\n", psp->py);
	fprintf(fp, "%f\n", psp->pz);
	fprintf(fp, "%f\n", psp->rx);
	fprintf(fp, "%f\n", psp->ry);
	fprintf(fp, "%f\n", psp->rz);
	fprintf(fp, "%f\n", psp->s);
	fclose(fp);
#else
	m_DisEvent++;
	TBAX->Position = 60;
	TBAY->Position = 60;
	TBPX->Position = 0;
	TBPY->Position = 0;
	TBPZ->Position = 0;
	TBRX->Position = 0;
	TBRY->Position = 0;
	TBRZ->Position = -180;
	TBR->Position = 0;
	TBS->Position = 23;
	SBSC->Position = 0;
	m_DisEvent--;
    UpdateItem();
	UpdateBtn();
#endif
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::UpdateBtn(void)
{
	LoadBtn->Enabled = RGSRC->ItemIndex && sys.m_fLoadImageMenu;
	PasteBtn->Enabled = RGSRC->ItemIndex && Clipboard()->HasFormat(CF_BITMAP);
	ViewBtn->Visible = sys.m_fView && (m_Item.m_pBitmap != NULL);
	SBSC->Enabled = !SB->Down;
	SBR->Enabled = !SB->Down && SBSC->Position;
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::RGSRCClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( !RGSRC->ItemIndex ){
		if( m_Item.m_sperspect.flag & 0x0000ffff ) m_Item.CopyFromHistory();
	}
	else if( !(m_Item.m_sperspect.flag & 0x0000ffff) ){
		if( m_pItem->m_pBitmap != NULL ){
			m_Item.CopySource(m_pItem->m_pBitmap);
			m_Loaded = 0;
		}
		else {
			m_Loaded = 1;
		}
	}
	UpdateWaveBtn(1);
	m_Item.m_sperspect.flag = RGSRC->ItemIndex | (m_Wave << 16);
	UpdateBitmap();
//	PBoxPaint(NULL);
	PBox->Invalidate();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_pBitmap == NULL ) return;

	m_SCX = double(m_pBitmap->Width) / double(PBox->Width);
	m_SCY = double(m_pBitmap->Height) / double(PBox->Height);
	X = X * m_SCX;
	Y = Y * m_SCY;

	m_XC = m_pBitmap->Width / 2;
	m_YC = m_pBitmap->Height / 2;
	int x = X - m_XC;
	int y = m_YC - Y;
	if( x || y ){
		m_Deg = atan2(y, x);
	}
	else {
		m_Deg = 0;
	}
	m_Dist = sqrt(x * x + y * y);
	m_SDeg = TBR->Position;
	m_SDistX = TBAX->Position;
	m_SDistY = TBAY->Position;
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_XC > 0 ){
		X = X * m_SCX;
		Y = Y * m_SCY;

		int x = X - m_XC;
		int y = m_YC - Y;
		double Deg;
		if( x || y ){
			Deg = atan2(y, x);
		}
		else {
			Deg = 0;
		}
		Deg = (m_Deg - Deg) * 180.0 / PI;
		Deg += m_SDeg;
		if( Deg < -180 ) Deg += 360.0;
		if( Deg > 180 ) Deg -= 360.0;
		TBR->Position = SHORT(Deg);

		double Dist = sqrt(x * x + y * y);
		Dist = (Dist - m_Dist)/2;
		int d = Dist + m_SDistX;
		if( d > 201 ) d = 201;
		if( d < 2 ) d = 2;
		TBAX->Position = d;
		d = Dist + m_SDistY;
		if( d > 201 ) d = 201;
		if( d < 2 ) d = 2;
		TBAY->Position = d;

	    UpdateItem();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_XC = -1;
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::LoadBtnClick(TObject *Sender)
{
	if( sys.m_fLoadImageMenu ){
		NoTopMost();
		CWaitCursor w;
		HBITMAP hb = sys.m_fLoadImageMenu(1, 16, 16);
		if( hb != NULL ){
			Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
			pBitmap->Handle = hb;
			m_Item.CopySource(pBitmap);
			m_Loaded = 1;
			UpdateBitmap();
			PBox->Invalidate();
			delete pBitmap;
		}
		TopMost();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::PasteBtnClick(TObject *Sender)
{
	NoTopMost();
	TClipboard *pCB = Clipboard();
	if (pCB->HasFormat(CF_BITMAP)){
		CWaitCursor w;
		Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
		try
		{
			pBitmap->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
			m_Item.CopySource(pBitmap);
			m_Loaded = 1;
			UpdateBitmap();
			PBox->Invalidate();
		}
		catch(...){
			ASSERT(0);
		}
		delete pBitmap;
	}
	TopMost();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::TopMost(void)
{
#if UseHWND
	::SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::NoTopMost(void)
{
#if UseHWND
	::SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::FormActivate(TObject *Sender)
{
	TopMost();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::FormDeactivate(TObject *Sender)
{
	NoTopMost();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::ViewBtnClick(TObject *Sender)
{
	if( sys.m_fView && (m_Item.m_pBitmap != NULL) ){
		NoTopMost();
		sys.m_fView(m_Item.m_pBitmap->Handle);
		TopMost();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::SBRClick(TObject *Sender)
{
	m_DisEvent++;
	SBSC->Position = 0;
	m_DisEvent--;
	UpdateItem();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::SBClick(TObject *Sender)
{
	UpdateWaveBtn(1);
	m_Item.m_sperspect.flag = RGSRC->ItemIndex | (m_Wave << 16);
	UpdateBitmap();
	PBoxPaint(NULL);
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::CBFChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateItem();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::PCClick(TObject *Sender)
{
	ColorDialog->Color = PC->Color;
	if( ColorDialog->Execute() == TRUE ){
		PC->Color = ColorDialog->Color;
		CBFChange(NULL);
	}
}
//---------------------------------------------------------------------------
static void __fastcall UpdateDefTB(TTrackBar *pBar, int f, int def, int &dir)
{
	if( f ){
		pBar->Position = pBar->Position + (pBar->PageSize * dir);
        if( pBar->Position == pBar->Max ){
			dir = -1;
        }
        else if( pBar->Position == pBar->Min ){
			dir = 1;
        }
    }
    else {
       	pBar->Position = def;
    }
    pBar->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::LDefMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( (Button != mbLeft) && (Button != mbRight) ) return;
	BOOL f = (Button != mbLeft);

	TLabel *_tt[]={L7, L8, L9, L6, L5, L4, L1, L2, L3, L10};

    int i;
    for( i = 0; i < AN(_tt); i++ ){
		if( _tt[i] == (TLabel *)Sender ) break;
    }
	m_DisEvent++;
    switch(i){
		case 0:
			UpdateDefTB(TBS, f, 23, m_DirTB[0]);
            break;
		case 1:
			UpdateDefTB(TBAX, f, 60, m_DirTB[1]);
            break;
        case 2:
			UpdateDefTB(TBAY, f, 60, m_DirTB[2]);
            break;
        case 3:
			UpdateDefTB(TBPX, f, 0, m_DirTB[3]);
            break;
        case 4:
			UpdateDefTB(TBPY, f, 0, m_DirTB[4]);
            break;
		case 5:
			UpdateDefTB(TBPZ, f, 0, m_DirTB[5]);
            break;
		case 6:
			UpdateDefTB(TBRX, f, 0, m_DirTB[6]);
            break;
		case 7:
			UpdateDefTB(TBRY, f, 0, m_DirTB[7]);
            break;
		case 8:
			UpdateDefTB(TBRZ, f, -180, m_DirTB[8]);
            break;
		case 9:
			UpdateDefTB(TBR, f, 0, m_DirTB[9]);
            break;
    }
    m_DisEvent--;
	UpdateItem();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::UpdateTB(void)
{
	m_SBar[0] = TBS->Position;
	m_SBar[1] = TBAX->Position;
	m_SBar[2] = TBAY->Position;
	m_SBar[3] = TBPX->Position;
	m_SBar[4] = TBPY->Position;
	m_SBar[5] = TBPZ->Position;
	m_SBar[6] = TBRX->Position;
	m_SBar[7] = TBRY->Position;
	m_SBar[8] = TBRZ->Position;
	m_SBar[9] = TBR->Position;
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::UpdateItem(void)
{
	if( m_DisEvent ) return;

	UpdateBitmap();
	PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::TBSChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	TTrackBar *_tt[]={TBS, TBAX, TBAY, TBPX, TBPY, TBPZ, TBRX, TBRY, TBRZ, TBR};
    TTrackBar *pBar = (TTrackBar *)Sender;

    int i;
    for( i = 0; i < AN(_tt); i++ ){
		if( _tt[i] == pBar ) break;
    }
    if( m_SBar[i] != pBar->Position ){
		m_SBar[i] = pBar->Position;
		UpdateItem();
    }
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::SBSCChange(TObject *Sender)
{
	UpdateItem();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::OnDrawClipboard(TMessage Message)
{
	PasteBtn->Enabled = RGSRC->ItemIndex && ::IsClipboardFormatAvailable(CF_BITMAP);
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_DRAWCLIPBOARD, Message.WParam, Message.LParam);
}
//---------------------------------------------------------------------------
void __fastcall TPerDlgBox::OnChangeCbChain(TMessage Message)
{
	if( (HWND)Message.WParam == m_hClipNext ){
		m_hClipNext = (HWND)Message.LParam;
	}
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_CHANGECBCHAIN, Message.WParam, Message.LParam);
    Message.Result = 0;
}
//---------------------------------------------------------------------------

