//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "PerSpect.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TPerSpectDlg::TPerSpectDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_Loaded = 0;
	m_DisEvent = 1;
	if( MsgEng ){
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
		RGSRC->Caption = "Source";
		RGSRC->Items->Strings[0] = "History";
		RGSRC->Items->Strings[1] = "Loaded image";
		RGSRC->Items->Strings[2] = "Overlay";
		PasteBtn->Caption = "Paste";
	}
	else {
		Caption = VER"  -  "TTLJ;
	}
	pBitmap = NULL;
	m_pObj = NULL;
	m_XC = -1;
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::FormDestroy(TObject *Sender)
{
	if( pBitmap != NULL ) delete pBitmap;
}
//---------------------------------------------------------------------------
int __fastcall TPerSpectDlg::Execute(CObj *pObj)
{
	m_pObj = pObj;
	m_Obj.Copy(pObj);
	m_Back = m_Obj.m_Back;
	LPSPERSPECT psp = &m_Obj.m_sperspect;

	TBAX->Position = psp->ax * 20.0;
	TBAY->Position = psp->ay * 20.0;
	TBPX->Position = psp->px * 40.0;
	TBPY->Position = psp->py * 40.0;
	TBPZ->Position = psp->pz * 40.0;
	TBRX->Position = psp->rx;
	TBRY->Position = psp->ry;
	TBRZ->Position = psp->rz;
	TBS->Position = psp->s * 10.0;
	TBR->Position = psp->r;
	RGSRC->ItemIndex = psp->flag;

	UpdateBitmap();
	UpdateBtn();
	m_DisEvent = 0;
	if( ShowModal() == IDOK ){
		psp = &pObj->m_sperspect;
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
		psp->flag = RGSRC->ItemIndex;
		if( psp->flag && m_Loaded && (m_Obj.pBitmap != NULL) ){
			pObj->CopySource(m_Obj.pBitmap);
		}
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TPerSpectDlg::UpdateBitmap(void)
{
	if( m_Obj.pBitmap == NULL ) return;

	m_Obj.m_sperspect.ax = double(TBAX->Position)/20.0;
	m_Obj.m_sperspect.ay = double(TBAY->Position)/20.0;

	m_Obj.m_sperspect.px = double(TBPX->Position)/40.0;
	m_Obj.m_sperspect.py = double(TBPY->Position)/40.0;
	m_Obj.m_sperspect.pz = double(TBPZ->Position)/40.0;

	m_Obj.m_sperspect.rx = TBRX->Position;
	m_Obj.m_sperspect.ry = TBRY->Position;
	m_Obj.m_sperspect.rz = TBRZ->Position;
	m_Obj.m_sperspect.s = double(TBS->Position)/10.0;
	m_Obj.m_sperspect.r = TBR->Position;

	pBitmap = Perspect(pBitmap, m_Obj.pBitmap, &m_Obj.m_sperspect, m_Back);
}
//---------------------------------------------------------------------
void __fastcall TPerSpectDlg::PBoxPaint(TObject *Sender)
{
	DrawBitmap(PBox, pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::TBAXChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateBitmap();
	PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::DefBtnClick(TObject *Sender)
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

	SPERSPECT *psp = &m_Obj.m_sperspect;
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
	TBAX->Position = 59;
	TBAY->Position = 59;
	TBPX->Position = 0;
	TBPY->Position = 0;
	TBPZ->Position = 0;
	TBRX->Position = 0;
	TBRY->Position = 0;
	TBRZ->Position = -180;
	TBR->Position = 0;
	TBS->Position = 23;
	m_DisEvent--;
	TBAXChange(NULL);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::UpdateBtn(void)
{
	LoadBtn->Enabled = RGSRC->ItemIndex && fLoadImageMenu;
	PasteBtn->Enabled = LoadBtn->Enabled && Clipboard()->HasFormat(CF_BITMAP);
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::RGSRCClick(TObject *Sender)
{
	if( !RGSRC->ItemIndex ){
		if( m_Obj.m_sperspect.flag && fLoadHistImage ){
			if( m_Obj.pBitmap == NULL ) m_Obj.pBitmap = CreateBitmap(16, 16);
			m_Obj.pBitmap->Handle = fLoadHistImage();
		}
	}
	else if( !m_Obj.m_sperspect.flag ){
		if( m_pObj->pBitmap != NULL ) m_Obj.CopySource(m_pObj->pBitmap);
	}
	m_Obj.m_sperspect.flag = RGSRC->ItemIndex;
	UpdateBitmap();
	PBox->Invalidate();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_XC = PBox->Width / 2;
	m_YC = PBox->Height / 2;
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
void __fastcall TPerSpectDlg::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_XC > 0 ){
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

		TBAXChange(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_XC = -1;
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::LoadBtnClick(TObject *Sender)
{
	if( fLoadImageMenu ){
		HBITMAP hb = fLoadImageMenu(1, 320, 256);
		if( hb != NULL ){
			if( m_Obj.pBitmap == NULL ) m_Obj.pBitmap = CreateBitmap(16, 16);
			m_Obj.pBitmap->Handle = hb;
			m_Loaded = 1;
			TBAXChange(NULL);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::PasteBtnClick(TObject *Sender)
{
	TClipboard *pCB = Clipboard();
	if (pCB->HasFormat(CF_BITMAP)){
		Graphics::TBitmap *pbmp = new Graphics::TBitmap;
		try
		{
			pbmp->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
			m_Obj.CopySource(pbmp);
			m_Loaded = 1;
			TBAXChange(NULL);
		}
		catch(...){
		}
		delete pbmp;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPerSpectDlg::TimerTimer(TObject *Sender)
{
	if( LoadBtn->Enabled ){
		PasteBtn->Enabled = Clipboard()->HasFormat(CF_BITMAP);
	}
}
//---------------------------------------------------------------------------

