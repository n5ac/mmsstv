//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "res.h"
#include "TextDlg.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TTextDlgBox *TextDlgBox;
//---------------------------------------------------------------------
__fastcall TTextDlgBox::TTextDlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
	MultProc();
	m_DisEvent = 1;
	m_pBitmap = NULL;
	m_pMaskBmp = NULL;
	m_Change = 0;
	m_DropProfile = 0;
	m_FText = "";
	m_MemoDown = 0;
	m_MouseHold = 0;
	if( sys.m_MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;

		Caption = VER"  "TTL;
		TabChar->Caption = "Set text and colors";
		TabMask->Caption = "Bitmap Mask";
        TabPer->Caption = "Deformations";
		CancelBtn->Caption = "Cancel";
		CancelBtn2->Caption = "Cancel";
		CancelBtn3->Caption = "Cancel";
		FontBtn->Caption = "Font";
		SBM->Caption = "Mac";
        SBL->Caption = "Code";
		GBG->Caption = "Gradation";
		GBS->Caption = "Shadow";
		GBA->Caption = "Misc";
		CBZero->Caption = "Slash zero";
		SBLD->Caption = "Del";
		SBLW->Caption = "Add";
		GBY->Caption = "Style profiles";
		L1->Caption = "Align";
		L2->Caption = "Style name";
        L33->Caption = "V.space";
        L34->Caption = "Rot.";
		SBYD->Caption = "Del";
		SBYW->Caption = "Add";
		CBY->Text = "Untitled";

		SBM->Hint = "Insert macro-keyword";
        SBL->Hint = "Character code list";
		SBC->Hint = "Choose color's component";
		SBLD->Hint = "Delete the text in the list";
		SBLW->Hint = "Add current text into the list";
		SBYD->Hint = "Delete the style in the profiles";
		SBYW->Hint = "Add current style into the profiles";
		SB3DD->Hint = "Make the shadow shorter";
		SB3DI->Hint = "Make the shadow longer";
		SBA1->Hint = "Left";
		SBA2->Hint = "Center";
		SBA3->Hint = "Right";
		SBA4->Hint = "Auto";
//		FontBtn->Hint = "Choose font";
		SBFD->Hint = "Make the font smaller";
		SBFI->Hint = "Make the font bigger";
        SBB->Hint = "Bold";
        SBI->Hint = "Italic";
		SBMSG->Hint = "Show messages list";
		Msg->Hint = "Copy the message to the text box with double click";
        SBGRev->Hint = "Reverses color's order";
        SBGRot->Hint = "Rotates color's order";
        SBGA->Hint = "Default combination";
		SBGRand->Hint = "Random combination";
		DefBtn->Caption = "Default";
		L51->Caption = "Rot.X";
		L52->Caption = "Rot.Y";
		L53->Caption = "Rot.Z";
		L54->Caption = "Move.Z";
		L55->Caption = "Move.Y";
		L56->Caption = "Move.X";
		L57->Caption = "ViewP.";
		L58->Caption = "Scale.X";
		L59->Caption = "Scale.Y";
		L60->Caption = "Rot";
		L61->Caption = "Deformation rate";
		DefBtn->Hint = "Return to the default";
		SBR->Hint = "Return to the default";

		L80->Caption = "Bitmap mask";
        CBBMask->Caption = "Enable bitmap mask";
        GB1->Caption = "Mask";
        GB2->Caption = "Built-in mask";
        GB3->Caption = "Edit mask";
        SBMLoad->Hint = "Load mask from file";
        SBMPaste->Hint = "Paste mask from the clipboard";
		SBMCopy->Hint = "Copy mask to the clipboard";
        SBMRot->Hint = "Rotates right";
        SBMMir->Hint = "Right and left turning over";
		SBAdjCol->Hint = "Adjusts mask image";
        SBClip->Hint = "Clipper";
		SBInvCol->Hint = "Inverts mask color";
        SBRotCol->Hint = "Rotates mask color";
		SBPtn->Hint = "Make random pattern";
		SBMEdit->Hint = "Edit mask";

        GB4->Caption = "User definition mask";
        L90->Caption = "Drag&&Drop for registration";
        SBMList->Hint = "User definition mask";

        GB5->Caption = "Filter";
	}
	else {
		Caption = VER"  "TTLJ;
		CBY->Text = "無題";
	}
	SBFD->Glyph->Assign(SB3DD->Glyph);
	SBFI->Glyph->Assign(SB3DI->Glyph);
	MultProc();
	if( sys.m_pTextList == NULL ){
		sys.m_pTextList = new TStringList;
		LoadStringsFromIniFile(sys.m_pTextList, "Strings", ININAME, TRUE);
		MultProc();
		if(!sys.m_pTextList->Count){
			sys.m_pTextList->Add("%c\\r\\nde %m");
			sys.m_pTextList->Add("CQSSTV");
			sys.m_pTextList->Add("Best 73");
		}
	}
	ASSERT(sys.m_pTextList);
	CBList->Items->Assign(sys.m_pTextList);
	if( sys.m_pStyleList == NULL ){
		sys.m_pStyleList = new TStringList;
		LoadStringsFromIniFile(sys.m_pStyleList, "Styles", ININAME, FALSE);
		MultProc();
	}
	CBY->Items->Assign(sys.m_pStyleList);
	CBY->Sorted = TRUE;
	int i;
	if( sys.m_pFontList == NULL ){
		sys.m_pFontList = new TStringList;
		char    fname[512];
		sprintf(fname, "%s%s", sys.m_BgnDir, ININAME);
		TMemIniFile *pIniFile = new TMemIniFile(fname);
		for( i = 0; i < 5; i++ ){
		    char bf[32];
			sprintf(bf, "Font.%u", i);
			AnsiString as = pIniFile->ReadString(bf, "FontName", "");
			if( !as.IsEmpty() ){
				BYTE charset = (BYTE)pIniFile->ReadInteger(bf, "FontSet", 0);
				DWORD style = pIniFile->ReadInteger(bf, "FontStyle", 0);
                AddStyle(as, charset, style);
            }
			sys.m_pFontList->Add(as);
        }
        delete pIniFile;
    }
    for( i = 0; i < 5; i++ ) SetSBF(i);
	ShowHint = sys.m_WinNT;
	MultProc();
	m_XC = -1;

    for( i = 0; i < AN(m_DirTB); i++ ){
		m_DirTB[i] = 1;
    }
	HBITMAP hb = ::LoadBitmap(sys.m_hInstance, MAKEINTRESOURCE(ID_BUILTIN));
	m_pBuiltInBmp = new Graphics::TBitmap;
	m_pBuiltInBmp->Handle = hb;

	m_pMaskBak = NULL;
	m_pCustomBmp = NULL;
	m_pMListBmp = NULL;
	m_pMLSrcBmp = NULL;
	m_MLIndex = -1;

	PC1->Color = sys.m_MaskC1.c;
    PC2->Color = sys.m_MaskC2.c;
	m_MaskMouseCapture = FALSE;
	m_MaskXW = m_MaskYW = 32;
    m_MaskX = m_MaskY = 0;
    m_pCodeView = NULL;
    m_hClipNext = ::SetClipboardViewer(Handle);
#if UseDraft
    m_Draft = 0;
#endif
	m_ExecPB = 0;
	srand(::GetTickCount());

#if 0		// 組み込みマスクビットマップの作成
#define	CUSTOMMAX	16
	Graphics::TBitmap *pBmp = CreateBitmap(32*CUSTOMMAX, 32);
    Graphics::TBitmap *pSrc;
	char name[256];
    for( i = 0; i < CUSTOMMAX; i++ ){
        wsprintf(name, "%sres\\P%u.bmp", sys.m_BgnDir, i+1);
		FILE *fp = fopen(name, "rb");
        if( fp != NULL ){
			fclose(fp);
	        pSrc = new Graphics::TBitmap;
	        pSrc->LoadFromFile(name);
	        pBmp->Canvas->Draw(i*32, 0, pSrc);
    	    delete pSrc;
        }
    }
	wsprintf(name, "%sres\\P.bmp", sys.m_BgnDir);
    pBmp->SaveToFile(name);
    delete pBmp;
#endif
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::FormDestroy(TObject *Sender)
{
	Timer->Enabled = FALSE;
	::ChangeClipboardChain(Handle, m_hClipNext);
	if( m_pBitmap != NULL ) delete m_pBitmap;
    if( m_pMaskBmp != NULL ) delete m_pMaskBmp;
	if( m_pMLSrcBmp != NULL ) delete m_pMLSrcBmp;
	if( m_pCustomBmp != NULL ) delete m_pCustomBmp;
    DeleteMaskSrc();
    if( m_pBuiltInBmp != NULL ) delete m_pBuiltInBmp;
	if( m_pMaskBak ) delete m_pMaskBak;
	MultProc();
	if( m_pMListBmp != NULL ){
		if( m_pMListBmp->Modified ){
			char name[256];
	        wsprintf(name, "%s"USERMASK, sys.m_BgnDir);
			try {
                m_pMListBmp->SaveToFile(name);
            }
            catch(...){
            }
        }
       	delete m_pMListBmp;
    }
    if( m_pCodeView ) delete m_pCodeView;

	ASSERT(sys.m_pTextList);
	sys.m_pTextList->Assign(CBList->Items);
	sys.m_MaskC1.c = PC1->Color;
    sys.m_MaskC2.c = PC2->Color;
    if( m_ExecPB ) QuitPB(FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::TopMost(void)
{
#if UseHWND
	::SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::NoTopMost(void)
{
#if UseHWND
	::SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateUI(void)
{
	m_DisEvent++;
	int f;
    if( CBBMask->Checked ){
		SBG1->Visible = FALSE;
        SBG2->Visible = FALSE;
        SBG3->Visible = FALSE;
        SBG4->Visible = FALSE;
        SBG5->Visible = FALSE;
        PG1->Visible = FALSE;
        PG2->Visible = FALSE;
        PG3->Visible = FALSE;
        PG4->Visible = FALSE;
        PG5->Visible = FALSE;
        PG6->Visible = FALSE;
        SBC->Visible = FALSE;
		SBGA->Visible = FALSE;
        SBGRand->Visible = FALSE;
		SBGRot->Visible = FALSE;
        SBGRev->Visible = FALSE;
        L80->Visible = TRUE;
    }
    else {
		SBG1->Visible = TRUE;
        SBG2->Visible = TRUE;
        SBG3->Visible = TRUE;
        SBG4->Visible = TRUE;
        SBG5->Visible = TRUE;
        PG1->Visible = TRUE;
        L80->Visible = FALSE;
		int f = SBG1->Down ? FALSE : TRUE;
        int f2 = SBG5->Down;
		PG2->Visible = f;
		PG3->Visible = f;
		PG4->Visible = f;
		SBC->Visible = f && !f2 && sys.m_fColorComp;
        PG5->Visible = f2;
        PG6->Visible = f2;
		SBGA->Visible = f2;
        SBGRand->Visible = f2;
		SBGRot->Visible = f;
        SBGRev->Visible = f;
    }
	f = !SBS1->Down;
	if( !f ) CB3D->Checked = FALSE;
	CB3D->Enabled = f;
	PB1->Visible = f && (!SBS2->Down || !CB3D->Checked);
	PB2->Visible = f && (SBS4->Down || CB3D->Checked);
	PB3->Visible = f && CB3D->Checked;
	UDL->Enabled = f && (SBS3->Down || !CB3D->Checked);
	SBLW->Enabled = (!m_FText.IsEmpty() && CBList->Items->IndexOf(m_FText) < 0) ? TRUE : FALSE;
	SBLD->Enabled = CBList->ItemIndex >= 0 ? TRUE : FALSE;
	SBYW->Enabled = !CBY->Text.IsEmpty();
	SBYD->Enabled = !CBY->Text.IsEmpty() && (sys.m_pStyleList->IndexOf(CBY->Text) >= 0);
	UDX->Enabled = CB3D->Checked;
	UDY->Enabled = CB3D->Checked;
	SB3DI->Enabled = CB3D->Checked;
	SB3DD->Enabled = CB3D->Checked;
	if( ShowHint ){
		char bf[32];
		sprintf(bf, "W = %d", UDL->Position);
		UDL->Hint = bf;
		sprintf(bf, "3D(Y) = %d", UDY->Position);
		UDY->Hint = bf;
		sprintf(bf, "3D(X) = %d", UDX->Position);
		UDX->Hint = bf;
	}
	Msg->Visible = SBMSG->Down;
    CBMSize->Enabled = CBBMask->Checked;
	SBAdjCol->Visible = sys.m_fAdjCol != NULL;
    SBClip->Visible = sys.m_fClip != NULL;
    f = m_pMaskBmp != NULL;
	SBMCopy->Enabled = f;
	SBMRot->Enabled = f;
    SBMMir->Enabled = f;
	SBInvCol->Enabled = f;
    SBRotCol->Enabled = f;
    SBAdjCol->Enabled = f;
    SBClip->Enabled = f;

	f = SBMList->Down;
    GB1->Visible = !f;
	GB4->Visible = f;

	f = !SB->Down;
	TBSC->Enabled = f;
    SBR->Enabled = f;

    TBF->Enabled = !SBFN->Down;
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateMemoFont(BYTE charset)
{
	switch(charset){
		case SHIFTJIS_CHARSET:
		case HANGEUL_CHARSET:
		case JOHAB_CHARSET:
		case CHINESEBIG5_CHARSET:   //
		case 134:       // 簡略
			break;
		default:
			charset = Font->Charset;
			break;
	}
	m_DisEvent++;
	if( Memo->Font->Charset != charset ){
		Memo->Font->Charset = charset;
	}
	m_DisEvent--;
	UpdateFontBtnHint();
}
//---------------------------------------------------------------------
int __fastcall TTextDlgBox::Execute(CItem *pItem)
{
	MultProc();
	m_Item.Copy(pItem);

	if( (sys.m_DefaultPage >= 0) && (sys.m_DefaultPage < Page->PageCount) ){
		Page->ActivePage = Page->Pages[sys.m_DefaultPage];
	}

	UpdateDialog(&m_Item);
	UpdateBitmap();
	UpdateSBSrc();
	m_DisEvent = 0;
	SBMList->Down = sys.m_SBMListState;
    if( SBMList->Down ) SBMListClick(NULL);
	MultProc();
	int r = ShowModal();
	sys.m_DefaultPage = GetActiveIndex(Page);
	sys.m_SBMListState = SBMList->Down;
	if( Msg->Modified ) sys.m_Msg = Msg->Text;
	if( r == IDOK ){
		MultProc();
#if UseDraft
		if( m_Draft ){
			m_Draft = 0;
            UpdateItem(&m_Item);
        }
#endif
		pItem->Copy(&m_Item);
		pItem->m_OrgXW = -1;
		MultProc();
		return TRUE;
	}
	else {
		MultProc();
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateFText(void)
{
	AnsiString as;
	if( Memo->Text.IsEmpty() ){
		m_FText = "";
	}
	else {
		::CrLf2Yen(as, Memo->Text);
		LPSTR p = as.c_str();
		if( *p ){
			*(p+strlen(p)-1) = 0;
			m_FText = p+1;
		}
	}
	m_DisEvent++;
	CBList->ItemIndex = CBList->Items->IndexOf(m_FText);
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateItem(CItem *pItem)
{
	UpdateUI();
	if( SBG1->Down ){
		pItem->m_Grade = 0;
	}
	else if( SBG2->Down ){
		pItem->m_Grade = 1;
	}
	else if( SBG3->Down ){
		pItem->m_Grade = 2;
	}
	else if( SBG4->Down ){
		pItem->m_Grade = 3;
	}
    else {
		pItem->m_Grade = 4;
    }
	if( SBS1->Down ){
		pItem->m_Shadow = 0;
	}
	else if( SBS2->Down ){
		pItem->m_Shadow = 1;
	}
	else if( SBS3->Down ){
		pItem->m_Shadow = 2;
	}
	else {
		pItem->m_Shadow = 3;
	}
	if( SBA1->Down ){
		pItem->m_Align = 0;
	}
	else if( SBA2->Down ){
		pItem->m_Align = 1;
	}
	else if( SBA3->Down ){
		pItem->m_Align = 2;
	}
	else {
		pItem->m_Align = 3;
	}
	pItem->m_ColB[0].c = PB1->Color;
	pItem->m_ColB[1].c = PB2->Color;
	pItem->m_ColB[2].c = PB3->Color;
	if( pItem->m_Grade == 4 ){
		pItem->m_ColF[0].c = PG1->Color;
		pItem->m_ColF[1].c = PG2->Color;
		pItem->m_ColF[2].c = PG3->Color;
		pItem->m_ColF[3].c = PG4->Color;
		pItem->m_ColF[4].c = PG5->Color;
		pItem->m_ColF[5].c = PG6->Color;
    }
    else {
		pItem->m_ColG[0].c = PG1->Color;
		pItem->m_ColG[1].c = PG2->Color;
		pItem->m_ColG[2].c = PG3->Color;
		pItem->m_ColG[3].c = PG4->Color;
    }
	pItem->m_Line = UDL->Position;
	pItem->m_Zero = CBZero->Checked;
	pItem->m_Text = Memo->Text.c_str();
	pItem->m_3d = (CB3D->Enabled && CB3D->Checked) ? 0x00010000 : 0;
	pItem->m_3d |= ((UDY->Position << 8)&0x0000ff00);
	pItem->m_3d |= (UDX->Position & 0x000000ff);
    pItem->m_OYW = UDOYW->Position & 0x0000ffff;
    pItem->m_OYWS = 0;
    pItem->m_Rot = UDRot->Position;

	LPSPERSPECT psp = &pItem->m_sperspect;
	psp->ax = double(TBAX->Position)/20.0;
	psp->ay = double(TBAY->Position)/20.0;
	psp->px = double(TBPX->Position)/20.0;
	psp->py = double(TBPY->Position)/20.0;
	psp->pz = double(TBPZ->Position)/20.0;
	psp->rx = TBRX->Position;
	psp->ry = TBRY->Position;
	psp->rz = TBRZ->Position;
	psp->s = double(TBS->Position)/10.0;
	psp->r = TBR->Position;
	UpdateWaveBtn(1);
	psp->flag = m_Wave;
	pItem->m_SC = TBSC->Position;

    pItem->m_Filter = 0;
    if( SBFA->Down ){
		pItem->m_Filter = 0x10000;
    }
    else if( SBFB->Down ){
		pItem->m_Filter = 0x20000;
    }
    pItem->m_Filter |= TBF->Position;

#if UseDraft
    if( m_Draft ){
    	psp->s *= 2;
		pItem->m_OYW = (UDOYW->Position/2) & 0x0000ffff;
    }
	pItem->m_pFont->Height = m_Draft ? m_FontHeight / 2 : m_FontHeight;
#else
	pItem->m_pFont->Height = m_FontHeight;
#endif
    pItem->UpdateFont();

#if 0
	static int count = 0;
	char bf[256];
    wsprintf(bf, "%u - %u", m_Draft, count++ );
    Caption = bf;
#endif
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateDialog(CItem *pItem)
{
	m_DisEvent++;
	switch(pItem->m_Grade){
		case 1:
			SBG2->Down = TRUE;
			break;
		case 2:
			SBG3->Down = TRUE;
			break;
		case 3:
			SBG4->Down = TRUE;
			break;
		case 4:
			SBG5->Down = TRUE;
			break;
		default:
			SBG1->Down = TRUE;
			break;
	}
	switch(pItem->m_Shadow){
		case 1:
			SBS2->Down = TRUE;
			break;
		case 2:
			SBS3->Down = TRUE;
			break;
		case 3:
			SBS4->Down = TRUE;
			break;
		default:
			SBS1->Down = TRUE;
			break;
	}
	switch(pItem->m_Align){
		case 1:
			SBA2->Down = TRUE;
			break;
		case 2:
			SBA3->Down = TRUE;
			break;
		case 3:
			SBA4->Down = TRUE;
			break;
		default:
			SBA1->Down = TRUE;
			break;
	}
	PB1->Color = pItem->m_ColB[0].c;
	PB2->Color = pItem->m_ColB[1].c;
	PB3->Color = pItem->m_ColB[2].c;
	if( SBG5->Down ){
		PG1->Color = pItem->m_ColF[0].c;
		PG2->Color = pItem->m_ColF[1].c;
		PG3->Color = pItem->m_ColF[2].c;
		PG4->Color = pItem->m_ColF[3].c;
		PG5->Color = pItem->m_ColF[4].c;
		PG6->Color = pItem->m_ColF[5].c;
    }
    else {
		PG1->Color = pItem->m_ColG[0].c;
		PG2->Color = pItem->m_ColG[1].c;
		PG3->Color = pItem->m_ColG[2].c;
		PG4->Color = pItem->m_ColG[3].c;
    }
	UDL->Position = SHORT(pItem->m_Line);
	CBZero->Checked = pItem->m_Zero;
	CB3D->Checked = pItem->m_3d & 0x00010000;
	int xm = pItem->m_3d & 0x000000ff;
	int ym = (pItem->m_3d >> 8) & 0x000000ff;
	if( xm & 0x0080 ) xm |= 0xffffff80;
	if( ym & 0x0080 ) ym |= 0xffffff80;
	UDY->Position = SHORT(ym);
	UDX->Position = SHORT(xm);
	if( pItem->m_pFont != NULL ){
		m_FontHeight = pItem->m_pFont->Height;
		UpdateMemoFont(pItem->m_pFont->Charset);
		UpdateFontBtnHint();
	}
	Memo->Text = pItem->m_Text.c_str();

    UDOYW->Position = (SHORT)pItem->GetTrueOYW();
    UDRot->Position = (SHORT)pItem->m_Rot;
	switch(pItem->m_Filter & 0x30000){
        case 0x10000:
        	SBFA->Down = TRUE;
            break;
		case 0x20000:
        	SBFB->Down = TRUE;
            break;
        default:
        	SBFN->Down = TRUE;
        	break;
    }
    TBF->Position = pItem->m_Filter & 0x00ff;
	UpdateFText();


	LPSPERSPECT psp = &pItem->m_sperspect;
	TBAX->Position = psp->ax * 20.0 + 0.5;
	TBAY->Position = psp->ay * 20.0 + 0.5;
	TBPX->Position = psp->px * 20.0;
	TBPY->Position = psp->py * 20.0;
	TBPZ->Position = psp->pz * 20.0;
	TBRX->Position = psp->rx;
	TBRY->Position = psp->ry;
	TBRZ->Position = psp->rz;
	TBS->Position = psp->s * 10.0 + 0.5;
	TBR->Position = psp->r;
	m_Wave = psp->flag;
	UpdateWaveBtn(0);
	TBSC->Position = (SHORT)pItem->m_SC;
	UpdateHint();
	UpdateTB();

    CBBMask->Checked = pItem->m_pMaskBmp != NULL;
    if( pItem->m_pMaskBmp ){
		if( m_pMaskBmp ) delete m_pMaskBmp;
        m_MaskXW = pItem->m_pMaskBmp->Width;
        m_MaskYW = pItem->m_pMaskBmp->Height;
        m_MaskX = m_MaskY = 0;
        m_pMaskBmp = CreateBitmap(m_MaskXW, m_MaskYW);
        m_pMaskBmp->Canvas->Draw(0, 0, pItem->m_pMaskBmp);
        UpdateCBMSize();
		UpdateSBSrc();
    }
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateHint(void)
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
		wsprintf(bf, "%d", TBSC->Position);
		TBSC->Hint = bf;
        wsprintf(bf, "Level:%d", TBF->Position + 1);
        TBF->Hint = bf;
	}
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateWaveBtn(int dir)
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
void __fastcall TTextDlgBox::UpdateBitmap(void)
{
	MultProc();
	m_Change = 0;
	UpdateItem(&m_Item);
	char bf[1024];
	sys.m_fConvMac(bf, m_Item.GetText(), sizeof(bf)-1);
	MultProc();
	if( m_pBitmap == NULL ){
		int xw = m_Item.m_OrgXW;
		int yw = m_Item.m_OrgYW;
		if( (xw < 0)||(yw < 0) ){
			xw = 160;
			yw = 128;
		}
		m_pBitmap = CreateBitmap(xw, yw);
	}
	FillBitmap(m_pBitmap, m_Item.m_Back);
	MultProc();
#if MeasureAccuracy
	LARGE_INTEGER	liFreq;

	QueryPerformanceFrequency(&liFreq);
	m_pBitmap = m_Item.Draw(m_pBitmap, bf);
    int t1 = int(m_Item.m_dlDiff1 * 100000 / liFreq.QuadPart);
    int t2 = int(m_Item.m_dlDiff2 * 100000 / liFreq.QuadPart);
    int t3 = int(m_Item.m_dlDiff3 * 100000 / liFreq.QuadPart);
    int t4 = t1 + t2 + t3;
    int t1p = int(m_Item.m_dlDiffPer1 * 100000 / liFreq.QuadPart);
    int t2p = int(m_Item.m_dlDiffPer2 * 100000 / liFreq.QuadPart);
	int xw = m_Item.Get1stXW();
    int yw = m_Item.Get1stYW();
	sprintf(bf, "%ux%u, %u.%02u + %u.%02u + %u.%02u = %u.%02u (%u.%02u + %u.%02u)",
    	xw, yw,
        t1 / 100, t1 % 100,
        t2 / 100, t2 % 100,
        t3 / 100, t3 % 100,
        t4 / 100, t4 % 100,
        t1p / 100, t1p % 100,
        t2p / 100, t2p % 100
    );
	Caption = bf;
#else
	m_pBitmap = m_Item.Draw(m_pBitmap, bf);
#endif
#if 1
	PBoxPaint(NULL);
#else
	if( Page->ActivePage == TabChar ){
		PBox->Invalidate();
    }
    else {
    	PBoxPer->Invalidate();
    }
#endif
	UpdateHint();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FillPBox(TPaintBox *pBox)
{
	TRect rc;
    rc.Left = 0;
    rc.Top = 0;
    rc.Right = pBox->Width;
    rc.Bottom = pBox->Height;
	pBox->Canvas->Brush->Color = clBtnFace;
	pBox->Canvas->FillRect(rc);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxPaint(TObject *Sender)
{
	if( !IsWindowVisible(Handle) ) return;
	if( m_pBitmap == NULL ) return;

	Graphics::TBitmap *pBitmap = m_pBitmap;
	if( sys.m_BitPixel == 16 ){
		pBitmap = new Graphics::TBitmap;
		pBitmap->Width = m_pBitmap->Width;
		pBitmap->Height = m_pBitmap->Height;
		pBitmap->Canvas->Draw(0, 0, m_pBitmap);
	}
	pBitmap->Transparent = TRUE;
	pBitmap->TransparentMode = tmFixed;
	pBitmap->TransparentColor = pBitmap->Canvas->Pixels[0][0];
	if( Page->ActivePage == TabChar ){
		FillPBox(PBox);
		PBox->Canvas->Draw(0, 0, pBitmap);
    }
	else if( Page->ActivePage == TabMask ){
		FillPBox(PBoxMask);
		PBoxMask->Canvas->Draw(0, 0, pBitmap);
    }
    else {
		FillPBox(PBoxPer);
	    PBoxPer->Canvas->Draw(0, 0, pBitmap);
    }
	pBitmap->Transparent = FALSE;
	if( pBitmap != m_pBitmap ) delete pBitmap;
	if( m_MouseHold ){
		PBox->Canvas->Pen->Color = clBlue;
		PBox->Canvas->MoveTo(0, m_MouseYC);
		PBox->Canvas->LineTo(PBox->Width, m_MouseYC);
		PBox->Canvas->MoveTo(m_MouseXC, 0);
		PBox->Canvas->LineTo(m_MouseXC, PBox->Height);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_Change = 2;
	Timer->Enabled = TRUE;
	UpdateFText();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::TimerTimer(TObject *Sender)
{
	if( m_Change ){
		m_Change--;
		if( !m_Change ){
			Timer->Enabled = m_ExecPB ? TRUE : FALSE;
#if UseDraft
			m_Draft = 0;
#endif
			UpdateBitmap();
		}
	}
#if UseDraft
    if( m_Draft ){
		Timer->Enabled = FALSE;
		m_Draft = 0;
        UpdateBitmap();
    }
#endif

	if( m_ExecPB ){
		if( !m_ExecPBCount || (m_ExecPBCount < 0) ){
			if( !m_ExecPBCount ) m_ExecPBCount = 4;		// 2[s] Interval
			CheckPBFinished();
        }
        if( m_ExecPBCount > 0 ){
			m_ExecPBCount--;
        }
        else {
			m_ExecPBCount++;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBGClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PB1Click(TObject *Sender)
{
	TPanel *pPanel = (TPanel *)Sender;
	InitCustomColor(ColorDialog);
	AddCustomColor(ColorDialog, PG1->Color);
	AddCustomColor(ColorDialog, PG2->Color);
	AddCustomColor(ColorDialog, PG3->Color);
	AddCustomColor(ColorDialog, PG4->Color);
	AddCustomColor(ColorDialog, PB1->Color);
	AddCustomColor(ColorDialog, PB2->Color);
	AddCustomColor(ColorDialog, m_Item.m_Back.c);
    if( SBG5->Down ){
		AddCustomColor(ColorDialog, PG5->Color);
		AddCustomColor(ColorDialog, PG6->Color);
    }

	ColorDialog->Color = pPanel->Color;
	NoTopMost();
	int r = ColorDialog->Execute();
	TopMost();
	if( r == TRUE ){
		pPanel->Color = ColorDialog->Color;
		if( (pPanel == PC1) || (pPanel == PC2) ){
			UpdateCustom(); PBoxCS->Invalidate();
        }
        else {
			UpdateBitmap();
        }
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FormActivate(TObject *Sender)
{
	TopMost();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FormDeactivate(TObject *Sender)
{
	NoTopMost();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FontBtnClick(TObject *Sender)
{
	FontDialog->Font->Assign(m_Item.m_pFont);
	int hOrg = m_FontHeight;
    int h = hOrg / 4;
	FontDialog->Font->Height = h;
	FontDialog->Font->Color = PG1->Color;
	NoTopMost();
	int r = FontDialog->Execute();
	TopMost();
	if( r == TRUE ){
		m_Item.m_pFont->Assign(FontDialog->Font);
		if( h != m_Item.m_pFont->Height ){
			hOrg = m_Item.m_pFont->Height * 4;
        }
		m_Item.m_pFont->Height = hOrg;
        m_FontHeight = hOrg;
		PG1->Color = FontDialog->Font->Color;
		m_Item.UpdateFont();
		if( m_Item.m_pFont != NULL ){
			UpdateMemoFont(m_Item.m_pFont->Charset);
		}
		UpdateBitmap();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMClick(TObject *Sender)
{
	if( sys.m_fGetMacKey ){
		char bf[256];
		NoTopMost();
		int r = sys.m_fGetMacKey(bf, sizeof(bf));
		TopMost();
		if( r ){
			HWND hWnd;
			if( SBMSG->Down ){
				Msg->SetFocus();
				hWnd = Msg->Handle;
			}
			else {
				Memo->SetFocus();
				hWnd = Memo->Handle;
			}
			for( LPCSTR p = bf; *p; p++ ){
				::PostMessage(hWnd, WM_CHAR, *p, 0);
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBCClick(TObject *Sender)
{
	if( sys.m_fColorComp ){
		UCOL    col[4];
		col[0].c = PG1->Color;
		col[1].c = PG2->Color;
		col[2].c = PG3->Color;
		col[3].c = PG4->Color;
		NoTopMost();
		int r = sys.m_fColorComp(&col[0].d);
		TopMost();
		if( r ){
			PG1->Color = col[0].c;
			PG2->Color = col[1].c;
			PG3->Color = col[2].c;
			PG4->Color = col[3].c;
			UpdateBitmap();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBLWClick(TObject *Sender)
{
	m_DisEvent++;
	CBList->Items->Add(m_FText);
	CBList->ItemIndex = CBList->Items->IndexOf(m_FText);
	UpdateUI();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBLDClick(TObject *Sender)
{
	m_DisEvent++;
	if( CBList->ItemIndex >= 0 ){
		int n = CBList->ItemIndex;
		CBList->ItemIndex = -1;
		CBList->Items->Delete(n);
		UpdateUI();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBListChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( CBList->ItemIndex < 0 ) return;

	m_DisEvent++;
	AnsiString cs;
	AnsiString as = CBList->Items->Strings[CBList->ItemIndex];
	m_FText = as;
	::Yen2CrLf(cs, as);
	Memo->Text = cs;
	UpdateBitmap();
	m_DisEvent--;
    Memo->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBYWClick(TObject *Sender)
{
	m_DisEvent++;
	int r = sys.m_pStyleList->IndexOf(CBY->Text);
	if( r < 0 ) r = sys.m_pStyleList->Count;

	CItem *pItem = new CItem;
	pItem->Copy(&m_Item);
	pItem->m_Text = CBY->Text;
	char bf[64];
	sprintf(bf, "Style.%u", r);
	pItem->SaveToInifile(bf, ININAME, TRUE);
	delete pItem;

	if( r == sys.m_pStyleList->Count ){
		CBY->Items->Add(CBY->Text);
		sys.m_pStyleList->Add(CBY->Text);
		UpdateUI();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBYDClick(TObject *Sender)
{
	m_DisEvent++;
	int r = sys.m_pStyleList->IndexOf(CBY->Text);
	if( r >= 0 ){
		char bf[64];
		CItem *pItem = new CItem;
		int i;
		for( i = r; i < sys.m_pStyleList->Count - 1; i++ ){
			sprintf(bf, "Style.%u", i+1);
			pItem->LoadFromInifile(bf, ININAME, TRUE);
			sprintf(bf, "Style.%u", i);
			pItem->SaveToInifile(bf, ININAME, TRUE);
			MultProc();
		}
		sprintf(bf, "Style.%u", i);
		pItem->DeleteInifile(bf, ININAME);
		delete pItem;
		sys.m_pStyleList->Delete(r);
		r = CBY->Items->IndexOf(CBY->Text);
		if( r >= 0 ) CBY->Items->Delete(r);
		UpdateUI();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBYChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( !m_DropProfile ){
		UpdateUI();
		return;
	}
	m_DropProfile = 0;

	m_DisEvent++;
	int r = sys.m_pStyleList->IndexOf(CBY->Text);
	if( r >= 0 ){
		AnsiString as = m_Item.m_Text;
        DWORD ex = m_Item.GetEx();
        int pos = m_Item.GetPos();
		CItem *pItem = new CItem;
		char bf[64];
		sprintf(bf, "Style.%u", r);
		pItem->LoadFromInifile(bf, ININAME, TRUE);
		m_Item.Copy(pItem);
		delete pItem;
		m_Item.m_Text = as;
        m_Item.SetEx(ex);
        m_Item.SetPos(pos);
		UpdateDialog(&m_Item);
		UpdateBitmap();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBYKeyPress(TObject *Sender, char &Key)
{
	m_DropProfile = 0;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBYDropDown(TObject *Sender)
{
	m_DropProfile = 1;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	if( Button == mbRight ){
		CB3D->Checked = CB3D->Checked ? FALSE : TRUE;
	}
	else {
		CB3D->Checked = TRUE;
		m_MouseHold = TRUE;
		int xc = m_pBitmap->Width / 2;
		int yc = m_pBitmap->Height / 2;
		m_MouseXC = xc;
		m_MouseYC = yc;
		int x = X - xc;
		int y = Y - yc;
		x = x * 48 / xc;
		y = y * 48 / yc;
		double s = -UDRot->Position * PI / 180.0;
        double cosk = cos(s);
        double sink = sin(s);
        short xx = x * cosk + y * sink;
        short yy = -x * sink + y * cosk;
		UDX->Position = SHORT(xx);
		UDY->Position = SHORT(yy);
		Screen->Cursor = crSizeAll;
	}
	m_DisEvent--;
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_MouseHold ){
		X = X - m_MouseXC;
		Y = Y - m_MouseYC;
		double s = -UDRot->Position * PI / 180.0;
        double cosk = cos(s);
        double sink = sin(s);
        short x = X * cosk + Y * sink;
        short y = -X * sink + Y * cosk;
		short UX = UDX->Position;
		short UY = UDY->Position;
		m_DisEvent++;
		UDX->Position = short(x);
		UDY->Position = short(y);
		m_DisEvent--;
		if( (UX != UDX->Position) || (UY != UDY->Position) ){
#if UseDraft
			m_Draft = TRUE;
			Timer->Enabled = TRUE;
#endif
			UpdateBitmap();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_MouseHold ){
		m_MouseHold = 0;
		PBox->Invalidate();
	}
	Screen->Cursor = crDefault;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UDLClick(TObject *Sender, TUDBtnType Button)
{
	SBGClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UD3D(int a)
{
	m_DisEvent++;
	int x = UDX->Position;
	int y = UDY->Position;
	int xs = (x >= 0) ? 1 : -1;
	int ys = (y >= 0) ? 1 : -1;
	x = ABS(x);
	y = ABS(y);
	if( x == y ){
		int ad;
		if( x < 4 ){
			ad = 1;
		}
		else if( x < 12 ){
			ad = 2;
		}
		else {
			ad = 4;
		}
		ad *= a;
		x += ad;
		y += ad;
		if( x < 2 ) x = 2;
		if( y < 2 ) y = 2;
		if( x > 48 ) x = 48;
		if( y > 48 ) y = 48;
	}
	else {
		if( x < y ) x = y;
		x &= 0xfffffffc;
		y = x;
	}
	x *= xs; y *= ys;
	UDX->Position = SHORT(x);
	UDY->Position = SHORT(y);
	m_DisEvent--;
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SB3DIClick(TObject *Sender)
{
	UD3D((Sender == SB3DI) ? 1 : -1);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMSGClick(TObject *Sender)
{
	Msg->Visible = SBMSG->Down;
	if( SBMSG->Down ){
		Msg->Text = sys.m_Msg;
		Msg->Modified = FALSE;
		Msg->SelLength = 0;
		Msg->SetFocus();
	}
	else {
		if( Msg->Modified ){
			sys.m_Msg = Msg->Text;
			Msg->Modified = FALSE;
		}
		Memo->SetFocus();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MsgDblClick(TObject *Sender)
{
	AnsiString  as;
	AnsiString  cs;

	m_DisEvent++;
	if( Msg->Lines->Count ){
		int e = Msg->SelStart;
		LPCSTR p = Msg->Text.c_str();
		int n = 0;
		for( int i = 0; i < e; i++, p++ ){
			if( *p == '\n' ) n++;
		}
		if( n > Msg->Lines->Count ) n = Msg->Lines->Count - 1;
		as = Msg->Lines->Strings[n];
	}
	m_FText = as;
	::Yen2CrLf(cs, as);
	Memo->Text = cs;
	m_DisEvent--;
	SBMSG->Down = FALSE;
	SBMSGClick(NULL);
	MultProc();
	UpdateFText();
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoStartDrag(TObject *Sender,
	  TDragObject *&DragObject)
{
	if( !SBMSG->Down ){
		SBMSG->Down = TRUE;
		SBMSGClick(NULL);
	}
}
//---------------------------------------------------------------------------

void __fastcall TTextDlgBox::MsgDragDrop(TObject *Sender, TObject *Source,
	  int X, int Y)
{
	Msg->Lines->Add(m_FText);
}
//---------------------------------------------------------------------------

void __fastcall TTextDlgBox::MsgDragOver(TObject *Sender, TObject *Source,
	  int X, int Y, TDragState State, bool &Accept)
{
	if( Source == Memo ) Accept = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_MemoX = X;
	m_MemoY = Y;
	m_MemoDown = 1;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_MemoDown == 1 ){
		X -= m_MemoX;
		Y -= m_MemoY;
		if( (X*X)+(Y*Y) > 64 ){
			m_MemoDown = 2;
			Memo->BeginDrag(TRUE);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_MemoDown = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FontBtnMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		BYTE cset;
		UINT cp = ::GetACP();
		switch(cp){
			case 932:
	        	cset = SHIFTJIS_CHARSET;
				break;
			case 949:
            	cset = HANGEUL_CHARSET;
				break;
			case 950:
            	cset = CHINESEBIG5_CHARSET;
				break;
			case 936:
            	cset = 134;       // 簡略
				break;
			default:
				cset = DEFAULT_CHARSET;
				break;
		}
		int charset = m_Item.m_pFont->Charset;
		m_Item.m_pFont->Charset = BYTE((charset != ANSI_CHARSET) ? ANSI_CHARSET : cset);
		m_Item.UpdateFont();
		if( m_Item.m_pFont != NULL ){
			UpdateMemoFont(m_Item.m_pFont->Charset);
		}
		UpdateBitmap();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateFontBtnHint(void)
{
	AnsiString as = sys.m_MsgEng ? "Choose font" : "フォントの選択";
	if( m_Item.m_pFont ){
		as += " [";
		as += m_Item.m_pFont->Name;
		TFontStyles ts = m_Item.m_pFont->Style;
		DWORD d = FontStyle2Code(ts);
        AddStyle(as, m_Item.m_pFont->Charset, d);
        as += "]";
        SBB->Down = d & FSBOLD;
        SBI->Down = d & FSITALIC;
		if( m_pCodeView ) m_pCodeView->Execute(Handle, m_Item.m_pFont);
    }
    FontBtn->Hint = as;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SetSBF(int n)
{
	if( !sys.m_pFontList ) return;

	TSpeedButton* _tt[]={SBF1, SBF2, SBF3, SBF4, SBF5, NULL};

	TSpeedButton* pButton = _tt[n];
	AnsiString es = sys.m_MsgEng ? "Registration = Right button" : "登録=右ﾎﾞﾀﾝ";
	AnsiString fs = sys.m_pFontList->Strings[n];
    int undef = fs.IsEmpty();
    if( undef ) fs = sys.m_MsgEng ? "Load registered font" : "登録ﾌｫﾝﾄの読み出し";
	char bf[256];
    sprintf(bf, "%s (%s)", fs.c_str(), es.c_str());
	pButton->Hint = bf;
	pButton->Font->Color = undef ? clGrayText : clBlack;
}
//---------------------------------------------------------------------------
int __fastcall TTextDlgBox::GetSBFIndex(TObject *Sender)
{
	TSpeedButton* _tt[]={SBF1, SBF2, SBF3, SBF4, SBF5, NULL};

    for( int i = 0; _tt[i]; i++ ){
		if( Sender == _tt[i] ) return i;
    }
    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBFClick(TObject *Sender)
{
	if( !sys.m_pFontList ) return;
	int N = GetSBFIndex(Sender);
	if( sys.m_pFontList->Strings[N].IsEmpty() ) return;

	FontDialog->Font->Assign(m_Item.m_pFont);

	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, ININAME);
	TMemIniFile *pIniFile = new TMemIniFile(fname);
    {
	    char bf[32];
		sprintf(bf, "Font.%u", N);
		int ht = m_Item.m_pFont->Height;
		LoadFontFromInifile(m_Item.m_pFont, bf, pIniFile);
        int hs = m_Item.m_pFont->Height;
		m_Item.m_pFont->Height = ht;
        m_FontHeight = ht;
        int oyw = (short)pIniFile->ReadInteger(bf, "OYW", UDOYW->Position);
		if( hs ) oyw = ht * oyw / hs;
        UDOYW->Position = (short)oyw;
    }
    delete pIniFile;

	m_Item.UpdateFont();
	if( m_Item.m_pFont != NULL ){
		UpdateMemoFont(m_Item.m_pFont->Charset);
	}
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBFMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		FontDialog->Font->Assign(m_Item.m_pFont);
		FontDialog->Font->Color = clBlack;
		NoTopMost();
		int r = FontDialog->Execute();
		TopMost();
		if( r == TRUE ){
			char    fname[512];
			sprintf(fname, "%s%s", sys.m_BgnDir, ININAME);
			TMemIniFile *pIniFile = new TMemIniFile(fname);
            {
				int N = GetSBFIndex(Sender);
			    char bf[32];
				sprintf(bf, "Font.%u", N);
				SaveFontToInifile(FontDialog->Font, bf, pIniFile);
				pIniFile->WriteInteger(bf, "OYW", UDOYW->Position);
				pIniFile->UpdateFile();
				if( sys.m_pFontList ){
					AnsiString as = FontDialog->Font->Name;
					TFontStyles ts = FontDialog->Font->Style;
					DWORD d = FontStyle2Code(ts);
                    AddStyle(as, FontDialog->Font->Charset, d);
					sys.m_pFontList->Strings[N] = as;
					SetSBF(N);
                }
            }
	        delete pIniFile;
		}
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBBClick(TObject *Sender)
{
	if( !m_Item.m_pFont ) return;

	TFontStyles ts = m_Item.m_pFont->Style;
	DWORD d = FontStyle2Code(ts);
	d ^= FSBOLD;
    ts = Code2FontStyle(d);
    m_Item.m_pFont->Style = ts;

	m_Item.UpdateFont();
	if( m_Item.m_pFont != NULL ){
		UpdateMemoFont(m_Item.m_pFont->Charset);
	}
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBIClick(TObject *Sender)
{
	if( !m_Item.m_pFont ) return;

	TFontStyles ts = m_Item.m_pFont->Style;
	DWORD d = FontStyle2Code(ts);
	d ^= FSITALIC;
    ts = Code2FontStyle(d);
    m_Item.m_pFont->Style = ts;

	m_Item.UpdateFont();
	if( m_Item.m_pFont != NULL ){
		UpdateMemoFont(m_Item.m_pFont->Charset);
	}
	UpdateBitmap();
}
//---------------------------------------------------------------------------

void __fastcall TTextDlgBox::SBFDMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_Item.m_pFont == NULL ) return;

	int ad = 0;
	int h = m_FontHeight;
	if( Sender == SBFD ){
		if( Button == mbLeft ){
			if( h < -2 ) ad = 1;
    	}
    	else if( Button == mbRight ){
			if( h < -6 ) ad = 4;
    	}
	}
	else {
		if( Button == mbLeft ){
			ad = -1;
    	}
    	else if( Button == mbRight ){
			ad = -4;
    	}
	}
    if( ad ){
		m_FontHeight = h + ad;
		UpdateBitmap();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::DefBtnClick(TObject *Sender)
{
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
	TBS->Position = 15;
	TBSC->Position = 0;
	m_DisEvent--;
	UpdateTB();
	SBGClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBRClick(TObject *Sender)
{
	if( TBSC->Position ){
		m_DisEvent++;
		TBSC->Position = 0;
		m_DisEvent--;
		SBGClick(NULL);
    }
    TBSC->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxPerMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_pBitmap == NULL ) return;

	m_SCX = double(m_pBitmap->Width) / double(PBoxPer->Width);
	m_SCY = double(m_pBitmap->Height) / double(PBoxPer->Height);
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
	Screen->Cursor = crSizeAll;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxPerMouseMove(TObject *Sender,
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

		UpdateTB();
#if UseDraft
		m_Draft = TRUE;
		Timer->Enabled = TRUE;
#endif
		UpdateBitmap();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxPerMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_XC = -1;
	Screen->Cursor = crDefault;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::TBSCChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( m_Item.m_SC != TBSC->Position ){
		UpdateBitmap();
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
void __fastcall TTextDlgBox::LDefMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( (Button != mbLeft) && (Button != mbRight) ) return;
	BOOL f = (Button != mbLeft);

	TLabel *_tt[]={L57, L58, L59, L56, L55, L54, L51, L52, L53, L60};

    int i;
    for( i = 0; i < AN(_tt); i++ ){
		if( _tt[i] == (TLabel *)Sender ) break;
    }
	m_DisEvent++;
    switch(i){
		case 0:
			UpdateDefTB(TBS, f, 15, m_DirTB[0]);
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
	UpdateTB();
	SBGClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateTB(void)
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
void __fastcall TTextDlgBox::TBSChange(TObject *Sender)
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
		UpdateBitmap();
    }
}
//---------------------------------------------------------------------
void __fastcall MakeBitmapPtn(Graphics::TBitmap *pBmp, int hw, int sw, UCOL col1, UCOL col2)
{
	hw /= 3;
	CWaitCursor wait;
	TCanvas *pCanvas = pBmp->Canvas;
	int x, y, f, v;
	for( y = 0; y < pBmp->Height; y++ ){
		for( x = 0; x < pBmp->Width; x++ ){
			f = x / hw;
			switch(sw){
			case 0:
				if( (y/2) & 1 ){
					v = (x/2) & 1;
				}
				else {
					v = !((x/2) & 1);
				}
				break;
			case 1:
				if( (y/4) & 1 ){
					v = (x/2) & 1;
				}
				else {
					v = !((x/2) & 1);
				}
				break;
			case 2:
				if( (y/2) & 1 ){
					v = (x/4) & 1;
				}
				else {
					v = !((x/4) & 1);
				}
				break;
			case 3:
				if( (y/4) & 1 ){
					v = (x/4) & 1;
				}
				else {
					v = !((x/4) & 1);
				}
				break;
			case 4:
				switch(f){
					case 0:
						v = (y/2) & 1;
                    	break;
                    case 1:
						v = (y/4) & 1;
                    	break;
                    default:
						v = (y/8) & 1;
                    	break;
                }
				break;
			case 5:
				switch(f){
					case 0:
						v = (x/2) & 1;
                    	break;
                    case 1:
						v = (x/4) & 1;
                    	break;
                    default:
						v = (x/8) & 1;
                    	break;
                }
				break;
			case 6:
				switch(f){
					case 0:
						v = ((y+x)/2) & 1;
                    	break;
                    case 1:
						v = ((y+x)/4) & 1;
                    	break;
                    default:
						v = ((y+x)/8) & 1;
                    	break;
                }
				break;
			case 7:
				switch(f){
					case 0:
						v = ((x-y+1024)/2) & 1;
                    	break;
                    case 1:
						v = ((x-y+1024)/4) & 1;
                    	break;
                    default:
						v = ((x-y+1024)/8) & 1;
                    	break;
                }
				break;
			}
			pCanvas->Pixels[x][y] = v ? col1.c : col2.c;
    	}
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBBMaskClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( CBBMask->Checked ){
		if( !m_pMaskBmp ){
			if( m_pMaskBak ){
				m_pMaskBmp = m_pMaskBak;
                m_pMaskBak = NULL;
                UpdateMaskSrc(TRUE);
            }
            else {
				m_pMaskBmp = CreateBitmap(PBoxSrc->Width, PBoxSrc->Height);
				UCOL c1, c2;
        	    c1.c = PC1->Color; c2.c = PC2->Color;
				MakeBitmapPtn(m_pMaskBmp, PBoxSrc->Width, 0, c1, c2);
				m_MaskX = m_MaskY = 0; m_MaskXW = m_MaskYW = 8;
            }
			UpdateItemMask();
        }
    }
    else {
		if( m_pMaskBmp ){
			delete m_pMaskBmp;
            m_pMaskBmp = NULL;
        }
        if( m_Item.m_pMaskBmp ){
			if( m_pMaskBak ) delete m_pMaskBak;
			m_pMaskBak = m_Item.m_pMaskBmp;
            m_Item.m_pMaskBmp = NULL;
        }
    }
	PBoxSrcPaint(NULL);
    UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::L80Click(TObject *Sender)
{
	Page->ActivePage = TabMask;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::DrawMaskCursor(int x1, int y1, int x2, int y2)
{
	x1 -= SBH->Position;
    x2 -= SBH->Position;
    y1 -= SBV->Position;
    y2 -= SBV->Position;
	TCanvas *tp = PBoxSrc->Canvas;
	tp->Pen->Width = 1;
	tp->Pen->Style = psSolid;
	tp->MoveTo(x1, y1);
	int rop = ::SetROP2(tp->Handle, R2_NOT);
	tp->LineTo(x2, y1);
	tp->LineTo(x2, y2);
	tp->LineTo(x1, y2);
	tp->LineTo(x1, y1);
	::SetROP2(tp->Handle, rop);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::DrawMaskCursor(void)
{
	DrawMaskCursor(m_MaskX, m_MaskY, m_MaskX+m_MaskXW, m_MaskY+m_MaskYW);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxSrcPaint(TObject *Sender)
{
	if( (Sender != SBV) && (Sender != SBH) ) FillPBox(PBoxSrc);
	if( m_pMaskBmp ){
		PBoxSrc->Canvas->Draw(-SBH->Position, -SBV->Position, m_pMaskBmp);
        DrawMaskCursor();
    }
}
//---------------------------------------------------------------------------
static void __fastcall DrawBmpPBoxBtn(TCanvas *pCanvas, int x, int y, int w, BOOL sw)
{
	pCanvas->Pen->Style = psSolid;
    pCanvas->Pen->Width = 1;
    pCanvas->Pen->Color = sw ? clBlack : clWhite;
	w++;
    pCanvas->MoveTo(x+w, y);
    pCanvas->LineTo(x, y);
    pCanvas->LineTo(x, y+w);
    pCanvas->LineTo(x+1, y+w);
    pCanvas->Pen->Color = sw ? clWhite : clBlack;
    pCanvas->LineTo(x+w, y+w);
	pCanvas->LineTo(x+w, y);
}
//---------------------------------------------------------------------------
static void __fastcall DrawBmpPBox(TCanvas *pCanvas, int x, int y, Graphics::TBitmap *pBmp)
{
   	pCanvas->Draw(x+1, y+1, pBmp);
    DrawBmpPBoxBtn(pCanvas, x, y, pBmp->Width, FALSE);
}
//---------------------------------------------------------------------------
static int __fastcall GetIndex(int w, int m, int x, int y)
{
	if( ((x % w) < 1) || ((x % w) >= (w-1)) ) return -1;
	if( ((y % w) < 1) || ((y % w) >= (w-1)) ) return -1;
	x /= w;
    y /= w;
	int n = (y * 8) + x;
    if( n >= m ) n = m-1;
    return n;
}
//---------------------------------------------------------------------------
static void __fastcall DrawBmpPBox(TCanvas *pCanvas, int s, int w, int ws, int x, int y, Graphics::TBitmap *pBmp)
{
	TRect src, drc;

	src.Left = ((y*8)+x)*ws;
    src.Right = src.Left + w;
    src.Top = 0; src.Bottom = w;
    drc.Left = 1 + x*s;
	drc.Right = drc.Left + w;
    drc.Top = 1 + y*s;
    drc.Bottom = drc.Top + w;
    pCanvas->CopyRect(drc, pBmp->Canvas, src);
	DrawBmpPBoxBtn(pCanvas, x*s, y*s, w, FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxBMPaint(TObject *Sender)
{
	int x, y;
	for( y = 0; y < 2; y++ ){
		for( x = 0; x < 8; x++ ){
			DrawBmpPBox(PBoxBM->Canvas, 33, 30, 32, x, y, m_pBuiltInBmp);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxBMMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		m_MIndex = GetIndex(33, 16, X, Y);
        if( m_MIndex >= 0 ){
		    DrawBmpPBoxBtn(PBoxBM->Canvas, 33*(m_MIndex%8), 33*(m_MIndex/8), 30, TRUE);
			Graphics::TBitmap *pBmp = CreateBitmap(32, 32);
		    pBmp->Canvas->Draw(-m_MIndex*32, 0, m_pBuiltInBmp);
		    if( !m_pMaskBmp ) m_pMaskBmp = CreateBitmap(32, 32);
			if( !IsSameBitmap(pBmp, m_pMaskBmp) ){
				delete m_pMaskBmp;
		        m_pMaskBmp = pBmp;
		       	UpdateMaskSrc(TRUE);
		    }
		    else {
		    	delete pBmp;
		    }
		    CBMSize->SetFocus();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxBMMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( (Button == mbLeft) && (m_MIndex >= 0) ){
	    DrawBmpPBoxBtn(PBoxBM->Canvas, 33*(m_MIndex%8), 33*(m_MIndex/8), 30, FALSE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateCustom(void)
{
	if( !m_pCustomBmp ) m_pCustomBmp = CreateBitmap(16*8, 16);
	Graphics::TBitmap *pBmp = CreateBitmap(16, 16);
    int i;
	for( i = 0; i < 8; i++ ){
		UCOL c1, c2;
    	c1.c = PC1->Color; c2.c = PC2->Color;
		MakeBitmapPtn(pBmp, PBoxSrc->Width, i, c1, c2);
		m_pCustomBmp->Canvas->Draw(i*16, 0, pBmp);
    }
    delete pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxCSPaint(TObject *Sender)
{
	if( !m_pCustomBmp ){
		UpdateCustom();
		if( !m_pCustomBmp ) return;
    }
    MultProc();
	int x;
	for( x = 0; x < 8; x++ ){
		DrawBmpPBox(PBoxCS->Canvas, 18, 16, 16, x, 0, m_pCustomBmp);
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxCSMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		m_MIndex = GetIndex(18, 8, X, Y);
        if( m_MIndex >= 0 ){
		    DrawBmpPBoxBtn(PBoxCS->Canvas, 18*(m_MIndex%8), 0, 16, TRUE);

			Graphics::TBitmap *pBmp = CreateBitmap(PBoxSrc->Width, PBoxSrc->Height);
			UCOL c1, c2;
		    c1.c = PC1->Color; c2.c = PC2->Color;
			MakeBitmapPtn(pBmp, PBoxSrc->Width, m_MIndex, c1, c2);
		    if( m_pMaskBmp ) delete m_pMaskBmp;
			DeleteMaskSrc();
		    m_pMaskBmp = pBmp;
			m_MaskX = m_MaskY = 0; m_MaskXW = m_MaskYW = 8;
		    UpdateMaskSrc(FALSE);
		    CBMSize->SetFocus();
		}
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxCSMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( (Button == mbLeft) && (m_MIndex >= 0) ){
	    DrawBmpPBoxBtn(PBoxCS->Canvas, 18*(m_MIndex%8), 0, 16, FALSE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateItemMask(void)
{
	m_DisEvent++;
	CBBMask->Checked = TRUE;
	UpdateCBMSize();

	if( m_pMaskBak ){
    	delete m_pMaskBak;
        m_pMaskBak = NULL;
    }
	if( m_Item.m_pMaskBmp ) delete m_Item.m_pMaskBmp;
	m_Item.m_pMaskBmp = CreateBitmap(m_MaskXW, m_MaskYW);
    m_Item.m_pMaskBmp->Canvas->Draw(-m_MaskX, -m_MaskY, m_pMaskBmp);
    m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxSrcMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( !m_pMaskBmp ) return;

	if( Button == mbLeft ){
		X += SBH->Position; Y += SBV->Position;
		DrawMaskCursor();
		m_MaskAX = m_MaskX = X;
	    m_MaskAY = m_MaskY = Y;
        m_MaskAXW = m_MaskXW; m_MaskAYW = m_MaskYW;
	    DrawMaskCursor();
		UpdateItemMask();
	    UpdateBitmap();
	    m_MaskMouseCapture = 1;
	}
    else if( Button == mbRight ){
	    if( (m_MaskXW > 32) || (m_MaskYW > 32) ){
			Graphics::TBitmap *pSrc = CreateBitmap(m_MaskXW, m_MaskYW);
            pSrc->Canvas->Draw(-m_MaskX, -m_MaskY, m_pMaskBmp);

			delete m_pMaskBmp;
            m_pMaskBmp = CreateBitmap(32, 32);
            StretchCopy(m_pMaskBmp, pSrc, HALFTONE);
            delete pSrc;

			m_MaskX = m_MaskY = 0;
            m_MaskXW = m_MaskYW = 32;
			DeleteMaskSrc();
	    }
		else {
			m_MaskX = m_MaskY = 0;
        }
		SBH->Position = 0; SBV->Position = 0;
		UpdateItemMask();
		PBoxSrcPaint(NULL);
	   	UpdateBitmap();
    }
    CBMSize->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxSrcMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	if( m_MaskMouseCapture ){
		X += SBH->Position; Y += SBV->Position;
		if( m_MaskMouseCapture == 1 ){
			m_MaskMouseCapture++;
		    DrawMaskCursor();
        }
        else {
			DrawMaskCursor(m_MaskX, m_MaskY, m_MaskAX, m_MaskAY);
        }
        m_MaskAX = X; m_MaskAY = Y;
		DrawMaskCursor(m_MaskX, m_MaskY, m_MaskAX, m_MaskAY);
        X = ABS(m_MaskX-m_MaskAX); Y = ABS(m_MaskY-m_MaskAY);
        if( (X != m_MaskAXW)||(Y != m_MaskAYW) ){
			m_MaskAXW = X; m_MaskAYW = Y;
	        UpdateCBMSize(X, Y);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxSrcMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_MaskMouseCapture ){
		X += SBH->Position; Y += SBV->Position;
		if( m_MaskMouseCapture != 1 ){
			DrawMaskCursor(m_MaskX, m_MaskY, m_MaskAX, m_MaskAY);
			if( (ABS(X-m_MaskX) >= 2) && (ABS(Y-m_MaskY) >= 2) ){
				m_MaskXW = ABS(X-m_MaskX);
                m_MaskYW = ABS(Y-m_MaskY);
                if( X < m_MaskX ) m_MaskX = X;
                if( Y < m_MaskY ) m_MaskY = Y;
				UpdateItemMask();
			    UpdateBitmap();
            }
    	    DrawMaskCursor();
        }
		m_MaskMouseCapture = 0;
        UpdateCBMSize();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateSBSrc(void)
{
	SBH->Position = 0; SBV->Position = 0;
	int hmax, vmax;
	if( m_pMaskBmp ){
		int xw = m_pMaskBmp->Width;
	    int yw = m_pMaskBmp->Height;
	    hmax = xw - PBoxSrc->ClientWidth;
	    vmax = yw - PBoxSrc->ClientHeight;
    }
    else {
		hmax = vmax = -1;
    }
    SBH->Max = (hmax > 0) ? hmax : 0;
    SBV->Max = (vmax > 0) ? vmax : 0;
	SBH->Enabled = (hmax > 0) ? TRUE : FALSE;
    SBV->Enabled = (vmax > 0) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateMaskSrc(BOOL sw)
{
	DeleteMaskSrc();
	int xw = m_pMaskBmp->Width;
    int yw = m_pMaskBmp->Height;
	if( sw ){
		m_MaskX = m_MaskY = 0;
    	if( ((xw <= 32) && (yw <= 32)) ){
			m_MaskXW = xw; m_MaskYW = yw;
        }
    }
    else {
		if( m_MaskX >= xw ) m_MaskX = 0;
        if( m_MaskY >= yw ) m_MaskY = 0;
    }
	UpdateSBSrc();
	UpdateItemMask();
	PBoxSrcPaint(NULL);
   	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateMaskSrc(HBITMAP hb)
{
    if( hb != NULL ){
		Graphics::TBitmap *pBmp = new Graphics::TBitmap;
		pBmp->Handle = hb;
		if( m_pMaskBmp ) delete m_pMaskBmp;
		m_pMaskBmp = CreateBitmap(pBmp->Width, pBmp->Height);
		m_pMaskBmp->Canvas->Draw(0, 0, pBmp);
        delete pBmp;
		UpdateMaskSrc(FALSE);
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMLoadClick(TObject *Sender)
{
	if( !sys.m_fLoadImageMenu ) return;
    UpdateMaskSrc(sys.m_fLoadImageMenu(1, PBoxSrc->Width, PBoxSrc->Height));
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMPasteClick(TObject *Sender)
{
	if( !sys.m_fPasteImage ) return;
   	UpdateMaskSrc(sys.m_fPasteImage(1, PBoxSrc->Width, PBoxSrc->Height));
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMCopyClick(TObject *Sender)
{
	if( !m_pMaskBmp ) return;
    CopyBitmap(m_pMaskBmp);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBClipClick(TObject *Sender)
{
	if( !sys.m_fClip ) return;

	Graphics::TBitmap *pBmp = CreateBitmap(PBoxSrc->Width, PBoxSrc->Height);
	if( sys.m_fClip(pBmp->Handle, m_pMaskBmp->Handle) ){
		delete m_pMaskBmp;
        m_pMaskBmp = pBmp;
		UpdateMaskSrc(FALSE);
    }
    else {
		delete pBmp;
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateCBMSize(void)
{
	UpdateCBMSize(m_MaskXW, m_MaskYW);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateCBMSize(int xw, int yw)
{
	m_DisEvent++;
    int f = TRUE;
	if( xw == yw ){
		switch(xw){
			case 8:
				CBMSize->ItemIndex = 0;
            	break;
            case 16:
				CBMSize->ItemIndex = 1;
            	break;
            case 24:
				CBMSize->ItemIndex = 2;
            	break;
            case 32:
				CBMSize->ItemIndex = 3;
            	break;
            default:
            	f = FALSE;
                break;
        }
    }
    else {
		f = FALSE;
    }
	if( f ){
    	if( CBMSize->Items->Count > 4 ) CBMSize->Items->Delete(4);
    }
    else {
		char bf[256];
        wsprintf(bf, "%ux%u", xw, yw);
    	if( CBMSize->Items->Count > 4 ){
	        CBMSize->Items->Strings[4] = bf;
        }
        else {
	        CBMSize->Items->Add(bf);
        }
        CBMSize->ItemIndex = 4;
    }
    m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBMSizeChange(TObject *Sender)
{
	if( m_DisEvent ) return;

    switch(CBMSize->ItemIndex){
		case 0:
			m_MaskXW = m_MaskYW = 8;
			break;
        case 1:
			m_MaskXW = m_MaskYW = 16;
			break;
        case 2:
			m_MaskXW = m_MaskYW = 24;
			break;
        case 3:
			m_MaskXW = m_MaskYW = 32;
			break;
    }
	if( m_pMaskBmp ){
	    if( (m_MaskXW > m_pMaskBmp->Width) || (m_MaskYW > m_pMaskBmp->Height) ){
			Graphics::TBitmap *pBmp = CreateBitmap(m_MaskXW, m_MaskYW);
			int x, y;
			y = 0;
			while(y < m_MaskYW){
				x = 0;
				while(x < m_MaskXW){
					pBmp->Canvas->Draw(x, y, m_pMaskBmp);
					x += m_pMaskBmp->Width;
                }
                y += m_pMaskBmp->Height;
            }
            delete m_pMaskBmp;
            m_pMaskBmp = pBmp;
    	}
    }
	UpdateItemMask();
	PBoxSrcPaint(NULL);
   	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
	if( Page->ActivePage == TabMask ){
		TShiftState sf1, sf2;
		sf1 << ssShift;
		sf2 << ssShift;
		sf1 *= Shift;
		int sft = (sf1 == sf2);
        int MaskXW = m_MaskXW;
        int MaskYW = m_MaskYW;
        int MaskX = m_MaskX;
        int MaskY = m_MaskY;
		switch(Key){
			case VK_LEFT:
				if( sft ){
					if( m_MaskXW > 4 ) m_MaskXW--;
                }
                else {
					if( m_MaskX ) m_MaskX--;
                }
				Key = 0;
            	break;
            case VK_RIGHT:
				if( sft ){
					m_MaskXW++;
                }
                else {
					m_MaskX++;
                }
				Key = 0;
            	break;
            case VK_UP:
				if( sft ){
					if( m_MaskYW > 4 ) m_MaskYW--;
                }
                else {
					if( m_MaskY ) m_MaskY--;
                }
				Key = 0;
            	break;
            case VK_DOWN:
				if( sft ){
					m_MaskYW++;
                }
                else {
					m_MaskY++;
                }
				Key = 0;
            	break;
        }
        if( (MaskX!=m_MaskX)||(MaskY!=m_MaskY)||(MaskXW!=m_MaskXW)||(MaskYW!=m_MaskYW) ){
			UpdateItemMask();
			PBoxSrcPaint(NULL);
		   	UpdateBitmap();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::OnDrawClipboard(TMessage Message)
{
	SBMPaste->Enabled = ::IsClipboardFormatAvailable(CF_BITMAP);
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_DRAWCLIPBOARD, Message.WParam, Message.LParam);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::OnChangeCbChain(TMessage Message)
{
	if( (HWND)Message.WParam == m_hClipNext ){
		m_hClipNext = (HWND)Message.LParam;
	}
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_CHANGECBCHAIN, Message.WParam, Message.LParam);
    Message.Result = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMRotMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( !m_pMaskBmp ) return;
	if( (Button != mbLeft) && (Button != mbRight) ) return;

	BOOL f = (Button == mbLeft);
	CWaitCursor w;
	int xw = m_pMaskBmp->Width;
    int yw = m_pMaskBmp->Height;
	Graphics::TBitmap *pBmp = CreateBitmap(yw, xw);
	int x, y;
	BYTE *sp, *tp;
	for( y = 0; y < yw; y++ ){
		sp = (BYTE *)m_pMaskBmp->ScanLine[y];
		for( x = 0; x < xw; x++ ){
			if( f ){
				tp = (BYTE *)pBmp->ScanLine[x];
				tp += ((yw - y - 1) * 3);
        	}
            else {
				tp = (BYTE *)pBmp->ScanLine[xw - x - 1];
				tp += (y * 3);
            }
			*tp++ = *sp++;
			*tp++ = *sp++;
			*tp = *sp++;
		}
	}
	delete m_pMaskBmp;
    m_pMaskBmp = pBmp;
	UpdateMaskSrc(FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMMirMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( !m_pMaskBmp ) return;
	if( (Button != mbLeft) && (Button != mbRight) ) return;

	CWaitCursor w;
	int xw = m_pMaskBmp->Width;
   	int yw = m_pMaskBmp->Height;
	Graphics::TBitmap *pBmp = CreateBitmap(xw, yw);
	int x, y;
	BYTE *sp, *tp;
	for( y = 0; y < yw; y++ ){
		if( Button == mbLeft ){
			sp = (BYTE *)m_pMaskBmp->ScanLine[y];
			tp = (BYTE *)pBmp->ScanLine[y];
			tp += ((xw - 1) * 3);
			for( x = 0; x < xw; x++ ){
				*tp++ = *sp++;
				*tp++ = *sp++;
				*tp = *sp++;
				tp -= 5;
			}
		}
        else {
			sp = (BYTE *)m_pMaskBmp->ScanLine[y];
			tp = (BYTE *)pBmp->ScanLine[yw - y - 1];
			for( x = 0; x < xw; x++ ){
				*tp++ = *sp++;
				*tp++ = *sp++;
				*tp++ = *sp++;
			}
        }
	}
	delete m_pMaskBmp;
    m_pMaskBmp = pBmp;
	UpdateMaskSrc(FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBAdjColClick(TObject *Sender)
{
	if( !m_pMaskBmp ) return;
	if( !sys.m_fAdjCol ) return;

    if( sys.m_fAdjCol(m_pMaskBmp->Handle) ){
		DeleteMaskSrc();
		UpdateItemMask();
		PBoxSrcPaint(NULL);
   		UpdateBitmap();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::DeleteMaskSrc(void)
{
	if( m_pMaskBmpSrc ){
		delete m_pMaskBmpSrc;
        m_pMaskBmpSrc = NULL;
    }
    m_RotCol = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBInvColClick(TObject *Sender)
{
	if( !m_pMaskBmp ) return;

	int xw = m_pMaskBmp->Width;
    int yw = m_pMaskBmp->Height;
    int x, y;
    for( y = 0; y < yw; y++ ){
		LPBYTE p = (LPBYTE)m_pMaskBmp->ScanLine[y];
		for( x = 0; x < xw; x++ ){
			*p = BYTE(~*p); p++;
			*p = BYTE(~*p); p++;
			*p = BYTE(~*p); p++;
        }
    }
	DeleteMaskSrc();
	UpdateItemMask();
	PBoxSrcPaint(NULL);
	UpdateBitmap();

}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBRotColMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( !m_pMaskBmp ) return;

	int f = 0;
	if( Button == mbLeft ){
		f = 1;
    }
    else if( Button == mbRight ){
    	f = -1;
    }
	if( f ){
		CWaitCursor w;
		if( !m_pMaskBmpSrc ){
			m_pMaskBmpSrc = CreateBitmap(m_pMaskBmp->Width, m_pMaskBmp->Height);
			m_pMaskBmpSrc->Canvas->Draw(0, 0, m_pMaskBmp);
            m_RotCol = 0;
        }
		m_RotCol += f;
        if( m_RotCol >= 16 ) m_RotCol = 0;
        if( m_RotCol < 0 ) m_RotCol = 15;
        if( !m_RotCol ){
			m_pMaskBmp->Canvas->Draw(0, 0, m_pMaskBmpSrc);
        }
        else {
			double ad = m_RotCol * 6.283185307179586476925286766559 / 16.0;
			int x, y;
	        int xw = m_pMaskBmp->Width;
	        int yw = m_pMaskBmp->Height;
			int R, G, B;
	        double Y, RY, BY, S, H;
	        UCOL c;
			for( y = 0; y < yw; y++ ){
				LPBYTE sp = (LPBYTE)m_pMaskBmpSrc->ScanLine[y];
	            LPBYTE wp = (LPBYTE)m_pMaskBmp->ScanLine[y];
				for( x = 0; x < xw; x++ ){
					c.b.b = *sp++;
	                c.b.g = *sp++;
	                c.b.r = *sp++;

					Y = 0.3 * c.b.r + 0.59 * c.b.g + 0.11 * c.b.b;
					RY = 0.7 * c.b.r - 0.59 * c.b.g - 0.11 * c.b.b;
					BY = -0.3 * c.b.r - 0.59 * c.b.g + 0.89 * c.b.b;

					S = sqrt(RY*RY + BY*BY);
					if( BY ){
						H = atan2(RY, BY);
						H += ad;
						RY = S * sin(H);
						BY = S * cos(H);

						R = Y + RY;
						G = Y - 0.3 * RY / 0.59 - 0.11 * BY / 0.59;
						B = Y + BY;
						if( R > 255 ) R = 255;
						if( R < 0 ) R = 0;
						if( G > 255 ) G = 255;
						if( G < 0 ) G = 0;
						if( B > 255 ) B = 255;
						if( B < 0 ) B = 0;
	                }
	                else {
						R = c.b.r; G = c.b.g; B = c.b.b;
	                }

	                *wp++ = (BYTE)B;
	                *wp++ = (BYTE)G;
	                *wp++ = (BYTE)R;
	            }
	        }
		}
		UpdateItemMask();
		PBoxSrcPaint(NULL);
	   	UpdateBitmap();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMListClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( SBMList->Down ){
		if( m_pMLSrcBmp ) delete m_pMLSrcBmp;
        m_pMLSrcBmp = CreateBitmap(32, 32);
        FillBitmap(m_pMLSrcBmp, clBlack);
        if( m_Item.m_pMaskBmp ){
			int x, y;
            int xw = m_Item.m_pMaskBmp->Width;
            int yw = m_Item.m_pMaskBmp->Height;
            for( y = 0; y < 32; y += yw){
				for( x = 0; x < 32; x += xw){
		        	m_pMLSrcBmp->Canvas->Draw(x, y, m_Item.m_pMaskBmp);
                }
            }
        }
    }
    else {
		if( m_pMLSrcBmp ) delete m_pMLSrcBmp;
        m_pMLSrcBmp = NULL;
    }
	UpdateUI();
}
//---------------------------------------------------------------------------
int __fastcall TTextDlgBox::GetMLIndex(int x, int y)
{
	return GetIndex(35, 9*8, x, y);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMListPaint(TObject *Sender)
{
	Graphics::TBitmap *pBmp;
	if( !m_pMListBmp ){
		char name[256];
        wsprintf(name, "%s"USERMASK, sys.m_BgnDir);
		FILE *fp = fopen(name, "rb");
        if( fp != NULL ){
			fclose(fp);
			pBmp = new Graphics::TBitmap;
			try {
	            pBmp->LoadFromFile(name);
            }
            catch(...){
            }
            if( pBmp->Width < (32*72) ) pBmp->Width = 32*72;
            if( pBmp->Height < 32 ) pBmp->Height = 32;

            if( pBmp->PixelFormat != pf24bit ){
				m_pMListBmp = CreateBitmap(pBmp->Width, pBmp->Height);
				m_pMListBmp->Canvas->Draw(0, 0, pBmp);
            	delete pBmp;
            }
            else {
                m_pMListBmp = pBmp;
            }
        }
        else {
			m_pMListBmp = CreateBitmap(32*72, 32);
            FillBitmap(m_pMListBmp, clBlack);
        }
        m_pMListBmp->Modified = FALSE;
    }
    MultProc();
	int x, y;
	for( y = 0; y < 9; y++ ){
		for( x = 0; x < 8; x++ ){
			DrawBmpPBox(PBoxMList->Canvas, 35, 32, 32, x, y, m_pMListBmp);
        }
    }
}
//---------------------------------------------------------------------------
int __fastcall TTextDlgBox::IsMList(int n)
{
	int yw = m_pMListBmp->Height;
	int x, y;
    for( y = 0; y < yw; y++ ){
		LPBYTE p = (LPBYTE)m_pMListBmp->ScanLine[y];
        p += (n * 32) * 3;
		for( x = 0; x < 32; x++ ){
			if( *p++ ) return TRUE;
            if( *p++ ) return TRUE;
            if( *p++ ) return TRUE;
        }
    }
    return FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMListMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( !m_pMListBmp ) return;
	m_MLIndex = GetMLIndex(X, Y);
	if( (m_MLIndex >= 0) && IsMList(m_MLIndex) ){
		if( Button == mbLeft ){
			DrawBmpPBoxBtn(PBoxMList->Canvas, (m_MLIndex%8)*35, (m_MLIndex/8)*35, 32, TRUE);
			Graphics::TBitmap *pBmp = CreateBitmap(32, 32);
			pBmp->Canvas->Draw(-m_MLIndex*32, 0, m_pMListBmp);
		    if( !m_pMaskBmp ) m_pMaskBmp = CreateBitmap(32, 32);
			if( !IsSameBitmap(pBmp, m_pMaskBmp) ){
				delete m_pMaskBmp;
                m_pMaskBmp = pBmp;
	           	UpdateMaskSrc(TRUE);
            }
            else {
				delete pBmp;
            }
            ::Sleep(100);
			DrawBmpPBoxBtn(PBoxMList->Canvas, (m_MLIndex%8)*35, (m_MLIndex/8)*35, 32, FALSE);
            PBoxMList->BeginDrag(FALSE);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMLSrcPaint(TObject *Sender)
{
	FillPBox(PBoxMLSrc);
    if( m_pMLSrcBmp ){
		DrawBmpPBox(PBoxMLSrc->Canvas, 0, 0, m_pMLSrcBmp);
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMListDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
	if( Source == PBoxMLSrc ){
		int n = GetMLIndex(X, Y);
		if( n >= 0 ){
			m_pMListBmp->Canvas->Draw(n*32, 0, m_pMLSrcBmp);
	        m_pMListBmp->Modified = TRUE;
            PBoxMListPaint(NULL);
        }
    }
    else if( Source == PBoxMList ){
		X = GetMLIndex(X, Y);
        if( (X >= 0) && (X != m_MLIndex) ){
			Graphics::TBitmap *pBmp = CreateBitmap(32, 32);
			Graphics::TBitmap *pBmp2 = CreateBitmap(32, 32);
			pBmp->Canvas->Draw(-m_MLIndex*32, 0, m_pMListBmp);
			pBmp2->Canvas->Draw(-X*32, 0, m_pMListBmp);
            m_pMListBmp->Canvas->Draw(X*32, 0, pBmp);
            m_pMListBmp->Canvas->Draw(m_MLIndex*32, 0, pBmp2);
            delete pBmp;
            delete pBmp2;
	        m_pMListBmp->Modified = TRUE;
            PBoxMListPaint(NULL);
    	}
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMListDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = FALSE;
	if( Source == PBoxMLSrc ){
		if( GetMLIndex(X, Y) >= 0 ){
			Accept = TRUE;
        }
    }
    else if( Source == PBoxMList ){
		X = GetMLIndex(X, Y);
        if( (X >= 0) && (X != m_MLIndex) ){
			Accept = TRUE;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMLSrcMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( !m_pMLSrcBmp ) return;

	if( Button == mbLeft ){
		DrawBmpPBoxBtn(PBoxMLSrc->Canvas, 0, 0, 32, TRUE);
        if( m_Item.m_pMaskBmp ){
			if( !IsSameBitmap(m_pMLSrcBmp, m_Item.m_pMaskBmp) ){
				CopyBitmap(m_pMaskBmp, m_pMLSrcBmp);
	           	UpdateMaskSrc(TRUE);
            }
        }
		::Sleep(100);
		DrawBmpPBoxBtn(PBoxMLSrc->Canvas, 0, 0, 32, FALSE);
		PBoxMLSrc->BeginDrag(FALSE);
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMLSrcDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = (Source == PBoxMList) && (m_MLIndex >= 0);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMLSrcDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
	if( (Source == PBoxMList) && (m_MLIndex >= 0) ){
		m_pMLSrcBmp->Canvas->Draw(-m_MLIndex*32, 0, m_pMListBmp);
		Graphics::TBitmap *pBmp = CreateBitmap(32, 32);
        FillBitmap(pBmp, clBlack);
        m_pMListBmp->Canvas->Draw(m_MLIndex*32, 0, pBmp);
		delete pBmp;
		m_pMListBmp->Modified = TRUE;
        PBoxMListPaint(NULL);
        PBoxMLSrcPaint(NULL);
    }
}
#if 0
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::DrawCrack(TCanvas *pCanvas, int xw, int yw, double &x, double &y, double &dir)
{
#if 0
	if( !(rand() % 32) ){
		double xs = x;
        double ys = y;
        double ds = dir + 90;
		for( int i = 0; i < 8; i++ ){
			DrawCrack(pCanvas, xw, yw, xs, ys, ds);
        }
    }
#endif

	double d = dir * 3.1415926535 / 180.0;

    UCOL c;
    double d1 = d + 0.78539816339744830961566084581988;
    double d2 = d - 0.78539816339744830961566084581988;
    int xx, yy;
    xx = cos(d1); yy = sin(d1);
    c.c = pCanvas->Pixels[x+xx][y+yy];
    c.b.b /= 2; c.b.r /= 2; c.b.g /= 2;
	pCanvas->Pixels[x+xx][y+yy] = c.c;

    xx = cos(d2); yy = sin(d2);
    c.c = pCanvas->Pixels[x+xx][y+yy];
    c.b.b /= 2; c.b.r /= 2; c.b.g /= 2;
	pCanvas->Pixels[x+xx][y+yy] = c.c;

	pCanvas->Pixels[x][y] = clBlack;

    x += cos(d);
    y += sin(d);
    if( !(rand() % 2) ) dir += (15 - (rand() % 30));
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CreateCrack(void)
{
	TCanvas *pCanvas = m_pMaskBmp->Canvas;
    int xw = m_pMaskBmp->Width;
    int yw = m_pMaskBmp->Height;
    int max = sqrt(xw*xw + yw*yw);
	for( int i = 0; i < max/16; i++ ){
		double x = rand() % xw;
        double y = rand() % yw;
        double deg = rand() % 360;
		for( int j = 0; j < max; j++ ){
			DrawCrack(pCanvas, xw, yw, x, y, deg);
        }
    }
   	UpdateMaskSrc(FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMEditClick(TObject *Sender)
{
	char name[256];
	wsprintf(name, "%s$TextArt.bmp", sys.m_BgnDir);
//	m_MaskTempName = name;
   	Graphics::TBitmap *pBmp;
	if( !m_pMaskBmp ){
    	pBmp = CreateBitmap(32, 32);
        FillBitmap(pBmp, clBlack);
        pBmp->SaveToFile(name);
        delete pBmp;
    }
    else {
        m_pMaskBmp->SaveToFile(name);
    }
    ExecPB(name);
}
#endif
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMEditClick(TObject *Sender)
{
	CWaitCursor w;

	m_ExecPBCount = 0;
	m_ExecPB = FALSE;
    Timer->Enabled = FALSE;
    QuitPB(TRUE);

	char name[256];
	wsprintf(name, "%sTextArt$.bmp", sys.m_BgnDir);
	m_MaskTempName = name;
   	Graphics::TBitmap *pBmp;
	if( !m_pMaskBmp ){
    	pBmp = CreateBitmap(32, 32);
        FillBitmap(pBmp, clBlack);
        pBmp->SaveToFile(name);
        delete pBmp;
    }
    else {
        m_pMaskBmp->SaveToFile(name);
    }
    ExecPB(name);
    for( int i = 0; i < 100; i++ ){
        ::Sleep(100);
		if( IsPB() ){
			m_ExecPB = 1;
            Timer->Enabled = TRUE;
            break;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::OnActivateApp(TMessage Message)
{
	if( m_ExecPB ){
		m_ExecPBCount = -6;		// 3[s] continued check
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::QuitPB(BOOL fWait)
{
	HWND hWndA = NULL;
	while(1){
		HWND hWnd = IsPB();
		if( (hWnd != NULL) && (hWnd != hWndA) ){
			hWndA = hWnd;
			::PostMessage(hWnd, WM_QUIT, 0, 0);
			if( fWait ){
				for( int i = 0; i < 30; i++ ){
					::Sleep(100);
					MultProc();
                    if( IsPB() != hWnd ) break;
                }
        	}
            else {
				break;
            }
        }
        else {
			break;
        }
	}
}
//---------------------------------------------------------------------------
BOOL __fastcall TTextDlgBox::CheckPBFinished(void)
{
	if( !m_ExecPB ) return FALSE;

	if( !IsPB() ){		// ペイントブラシの終了を検出
		m_ExecPB = FALSE;
        Timer->Enabled = FALSE;
		LoadEditedMask();
        return TRUE;
    }
    else {
		return FALSE;
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::LoadEditedMask(void)
{
   	Graphics::TBitmap *pBmp = new Graphics::TBitmap;
	try {
	    pBmp->LoadFromFile(m_MaskTempName);
    }
    catch(...){
    }
	BOOL f = FALSE;
	if( !m_pMaskBmp ){
        f = TRUE;
    	m_pMaskBmp = CreateBitmap(32, 32);
        FillBitmap(m_pMaskBmp, clBlack);
    }
	if( !IsSameBitmap(pBmp, m_pMaskBmp) ){
    	CopyBitmap(m_pMaskBmp, pBmp);
	    UpdateMaskSrc(FALSE);
    }
    else if( f ){
		delete m_pMaskBmp;
        m_pMaskBmp = NULL;
    }
	delete pBmp;
	::DeleteFile(m_MaskTempName.c_str());
}
//---------------------------------------------------------------------------
static void __fastcall DrawPtn(TCanvas *pCanvas, int X, int Y, int xw, int yw, int w, int max, BOOL sw)
{
	const COLORREF tt[]={
		RGB(0,0,255), RGB(0,255,0), RGB(0,255,255), RGB(255,0,0),
        RGB(255,0,255), RGB(255,255,0), RGB(255,0,0), RGB(255,255,0)
    };
	int x, y, i;
	UCOL c;
    POINT P[10];
	if( sw == 3 ) w+=4;
    for( i = 0; i < max; i++ ){
		x = X + (rand() % xw);
        y = Y + (rand() % yw);
		c.d = tt[rand()%AN(tt)];
		pCanvas->Pen->Color = (w < 3) ? c.c : clBlack;
        pCanvas->Brush->Color = c.c;
		switch(sw){
			case 0:
				pCanvas->Ellipse(x-w, y-w, x+w, y+w);
            	break;
            case 1:
				pCanvas->Rectangle(x-w, y-w, x+w, y+w);
            	break;
            case 2:
				P[0].x = x-w/3;
                P[0].y = y-w;
                P[1].x = x-w;
                P[1].y = y;
                P[2].x = x-w/3;
                P[2].y = y+w;
                P[3].x = x+w/3;
                P[3].y = y+w;
                P[4].x = x+w;
                P[4].y = y;
                P[5].x = x+w/3;
                P[5].y = y-w;
            	pCanvas->Polygon(P, 5);
                break;
            case 3:
				P[0].x = x;
                P[0].y = y-w;
                P[1].x = x-w/3;
                P[1].y = y-w/3;
                P[2].x = x-w;
                P[2].y = y-w/3;
                P[3].x = x-w/2;
                P[3].y = y+w/4;
                P[4].x = x-w*2/3;
                P[4].y = y+w;
                P[5].x = x;
                P[5].y = y+w/2;
                P[6].x = x+w*2/3;
                P[6].y = y+w;
                P[7].x = x+w/2;
                P[7].y = y+w/4;
                P[8].x = x+w;
                P[8].y = y-w/3;
                P[9].x = x+w/3;
                P[9].y = y-w/3;
            	pCanvas->Polygon(P, 9);
            	break;
        }
    }
}
//---------------------------------------------------------------------------
static void __fastcall SetBrushColor(COLORREF ttm[4], int sw)
{
	if( sw ){
		const COLORREF ttc[][4]={
			{RGB(0,0,255), RGB(255,255,0), RGB(255,0,0), RGB(0,255,0),},
			{RGB(0,0,255), RGB(255,255,0), RGB(255,0,0), RGB(0,255,255),},
			{RGB(0,0,0), RGB(255,255,0), RGB(255,0,0), RGB(0,255,0),},
			{RGB(0,0,0), RGB(255,255,0), RGB(255,0,0), RGB(0,255,255),},
			{RGB(255,0,0), RGB(0,255,255), RGB(0,0,0), RGB(0,255,0),},
			{RGB(255,255,0), RGB(0,255,255), RGB(255,0,0), RGB(0,0,255),},
    	};

    	static int N = 0;
    	memcpy(ttm, ttc[N], sizeof(COLORREF)*4);
    	N++;
    	if( N >= AN(ttc) ) N = 0;
    }
	else {
		const COLORREF tt[]={
			RGB(0,255,0), RGB(0,255,255), RGB(255,0,0),
        	RGB(255,0,255), RGB(255,255,0), RGB(255,0,0), RGB(255,255,0),
            RGB(255,255,255)
    	};
		COLORREF c = 0;
    	for( int i = 0; i < 4; i++ ){
			while(1){
				ttm[i] = tt[rand() % AN(tt)];
                if( c != ttm[i] ) break;
            }
            c = ttm[i];
    	}
    }
}
//---------------------------------------------------------------------------
static UCOL __fastcall GetGradeColor(UCOL c, int sw)
{
	switch(sw%4){
		case 1:
		case 2:
        	c.b.r = (BYTE)(c.b.r * 2 / 3);
        	c.b.g = (BYTE)(c.b.g * 2 / 3);
        	c.b.b = (BYTE)(c.b.b * 2 / 3);
        	break;
        case 3:
        	c.b.r = (BYTE)(c.b.r / 2);
        	c.b.g = (BYTE)(c.b.g / 2);
        	c.b.b = (BYTE)(c.b.b / 2);
        	break;
    }
    return c;
}
//---------------------------------------------------------------------------
static void __fastcall DrawBrush(TCanvas *pCanvas, COLORREF ttm[4], int X, int Y, int xw, int yw, int sw)
{
	int x, y, v, w;
	UCOL c;
	for( y = Y; y < Y+yw; y++ ){
		for( x = X; x < X+xw; x++ ){
			switch(sw){
				case 0:
                case 4:
                	v = (y/4);
                    w = y;
                    break;
                case 1:
                case 5:
                	v = (x/4);
                    w = x;
                    break;
                case 2:
                case 6:
                	v = (x+y)/4;
                    w = x + y;
                    break;
                case 3:
                case 7:
					v = (x-y+1024)/4;
                    w = x-y+1024;
                    break;
            }
           	c.d = ttm[v % 4];
			if( sw < 4 ) c = GetGradeColor(c, w);
            pCanvas->Pixels[x][y] = c.c;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBPtnMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( (Button != mbLeft) && (Button != mbRight) ) return;

	CWaitCursor w;

	int xw = PBoxSrc->Width;
    int yw = PBoxSrc->Height;

	Graphics::TBitmap *pBmp = CreateBitmap(xw, yw);
    if( m_pMaskBmp ){
		int x, y;
		for( y = 0; y < yw; y += m_pMaskBmp->Height){
			for( x = 0;  x < xw; x += m_pMaskBmp->Width ){
				pBmp->Canvas->Draw(x, y, m_pMaskBmp);
            }
        }
    }
    else {
	    FillBitmap(pBmp, clBlack);
    }

	xw = (xw*10+5)/30;
    yw = (yw*10+5)/20;
    int sw = (Button != mbLeft) ? 1 : 0;
	TCanvas *pCanvas = pBmp->Canvas;
    int max = xw * yw;
    int sww = sw * 2;
	DrawPtn(pCanvas, 0, 0, xw, yw, 2, max/16, sww);
	DrawPtn(pCanvas, 0, 0, xw, yw, 3, max/16, sww);
	DrawPtn(pCanvas, 0, 0, xw, yw, 4, max/256, sww);
	DrawPtn(pCanvas, 0, 0, xw, yw, 5, max/1024, sww);

	DrawPtn(pCanvas, 0, yw, xw, yw, 2, max/16, 1+sww);
	DrawPtn(pCanvas, 0, yw, xw, yw, 3, max/16, 1+sww);
	DrawPtn(pCanvas, 0, yw, xw, yw, 4, max/256, 1+sww);
	DrawPtn(pCanvas, 0, yw, xw, yw, 5, max/1024, 1+sww);

	COLORREF ttm[4];
    SetBrushColor(ttm, sw);
    sww *= 2;
	DrawBrush(pCanvas, ttm, xw, 0, xw, yw, 0 + sww);
	DrawBrush(pCanvas, ttm, xw, yw, xw, yw, 1 + sww);
	DrawBrush(pCanvas, ttm, xw+xw, 0, xw, yw, 2 + sww);
	DrawBrush(pCanvas, ttm, xw+xw, yw, xw, yw, 3 + sww);

	if( m_pMaskBmp ) delete m_pMaskBmp;
    m_pMaskBmp = pBmp;
    m_MaskXW = m_MaskYW = 32;
    UpdateMaskSrc(FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBG1Click(TObject *Sender)
{
	CItem *pItem = &m_Item;
	if( SBG1->Down ){
		pItem->m_Grade = 0;
	}
	else if( SBG2->Down ){
		pItem->m_Grade = 1;
	}
	else if( SBG3->Down ){
		pItem->m_Grade = 2;
	}
	else if( SBG4->Down ){
		pItem->m_Grade = 3;
	}
    else {
		pItem->m_Grade = 4;
    }
	UpdateDialog(&m_Item);
	SBGClick(NULL);
}
//---------------------------------------------------------------------------
TPanel* __fastcall TTextDlgBox::GetPG(int index)
{
	TPanel *_tt[]={PG1, PG2, PG3, PG4, PG5, PG6};
    return _tt[index % 6];
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBGRevClick(TObject *Sender)
{
	TColor c[6];
	int m = SBG5->Down ? 6 : 4;
    int i;
    for( i = 0; i < m; i++ ) c[i] = GetPG(i)->Color;
    for( i = 0; i < m; i++ ) GetPG(m-i-1)->Color = c[i];
    SBGClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBGRotClick(TObject *Sender)
{
	int i;
	int m = SBG5->Down ? 5 : 3;
    TColor c = GetPG(0)->Color;
    for( i = 0; i < m; i++ ) GetPG(i)->Color = GetPG(i+1)->Color;
	GetPG(i)->Color = c;
    SBGClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBGRandClick(TObject *Sender)
{
	UCOL c[6];
	UCOL cc;
	int i, j, f, n;

	cc.d = 0;
    for( i = 0; i < 6; i++ ){
		do {
			n = rand() % 7;
            n++;
			if( (n==7) && (rand()&7) ) n = (rand()%6)+1;
    	    cc.b.r = BYTE(n & 1 ? 255 : 0);
        	cc.b.g = BYTE(n & 2 ? 255 : 0);
        	cc.b.b = BYTE(n & 4 ? 255 : 0);
        	f = FALSE;
        	for( j = 0; j < i; j++ ){
   	        	if( cc.d == c[j].d ){
					f = TRUE;
           	        break;
               	}
            }
        } while (f);
        c[i] = cc;
        GetPG(i)->Color = cc.c;
    }
    SBGClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBGAClick(TObject *Sender)
{
	const COLORREF tt[]={
		RGB(255,0,0), RGB(255,255,0), RGB(0,255,0), RGB(0,255,255), RGB(0,0,255), RGB(255,0,255),
    };

	int i;
	const COLORREF *p = tt;
    for( i = 0; i < 6; i++, p++ ){
		GetPG(i)->Color = TColor(*p);
    }
    SBGClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBLClick(TObject *Sender)
{
	if( !sys.m_CodeLeft ) sys.m_CodeLeft = Left - 120;
    if( !sys.m_CodeTop ) sys.m_CodeTop = Top + 110;
	if( !m_pCodeView ){
		m_pCodeView = new TCodeView(this);
    }
    m_pCodeView->Execute((HWND)Handle, m_Item.m_pFont);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::OnCodeViewClose(TMessage Message)
{
	if( Message.LParam == int(m_pCodeView) ){
		TCodeView *pView = m_pCodeView;
        m_pCodeView = NULL;
		delete pView;
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::OnCodeInsert(TMessage Message)
{
	if( Message.LParam != int(m_pCodeView) ) return;

	switch(Message.WParam){
		case 0xfffffff0:
		case 0xfffffff1:
		case 0xfffffff2:
		case 0xfffffff3:
		case 0xfffffff4:
		case 0xfffffff5:
		case 0xfffffff6:
		case 0xfffffff7:
			if( m_Item.m_pFont != NULL ){
				TFont *pFont = m_Item.m_pFont;
                int dfs = 0;
				switch(Message.WParam & 7){
					case 0:
			        	pFont->Name = "Webdings";
						pFont->Charset = 2;
                    	break;
					case 1:
			        	pFont->Name = "Wingdings";
						pFont->Charset = 2;
                    	break;
					case 2:
			        	pFont->Name = "Symbol";
						pFont->Charset = 2;
                    	break;
					case 3:
			        	pFont->Name = "Arial";
						pFont->Charset = ANSI_CHARSET;
                        dfs = FSBOLD;
                    	break;
					case 4:
			        	pFont->Name = sys.m_MsgEng ? "MS UI Gothic" : "ＭＳ Ｐゴシック";
						pFont->Charset = SHIFTJIS_CHARSET;
                        dfs = FSBOLD;
                    	break;
					case 5:
			        	pFont->Name = "DotumChe";
						pFont->Charset = HANGEUL_CHARSET;
                        if( !sys.m_tFontFam[fmHL] && sys.m_tFontFam[fmJOHAB] ){
							pFont->Charset = JOHAB_CHARSET;
                        }
                        dfs = FSBOLD;
                    	break;
					case 6:
			        	pFont->Name = "SimHei";
						pFont->Charset = 134;
                        dfs = FSBOLD;
                    	break;
					case 7:
			        	pFont->Name = "PMingLiU";
						pFont->Charset = CHINESEBIG5_CHARSET;
                        dfs = FSBOLD;
                    	break;
                }
			   	TFontStyles fs;
			   	fs = Code2FontStyle(dfs);
                pFont->Style = fs;
				m_Item.UpdateFont();
				UpdateMemoFont(m_Item.m_pFont->Charset);
				UpdateBitmap();
			}
        	break;
		case 0xfffffffe:
			::SetWindowPos(m_pCodeView->Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
//			m_pCodeView->Visible = FALSE;
			FontBtnClick(NULL);
			::SetWindowPos(m_pCodeView->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
//			m_pCodeView->Visible = TRUE;
        	break;
        case 0xffffffff:
			Memo->Clear();
    	    SBGClick(NULL);
        	break;
        default:
			if( Message.WParam ){
				if( Page->ActivePage != TabChar ) Page->ActivePage = TabChar;
				HWND hWnd;
				Memo->SetFocus();
				hWnd = Memo->Handle;
				char bf[16];
				if( Message.WParam & 0x00ff0000 ){
					wsprintf(bf, "\\$%04x", Message.WParam & 0x00ffff);
                }
                else if( Message.WParam & 0x00ff00 ){
					bf[0] = BYTE(Message.WParam >> 8);
   	                bf[1] = BYTE(Message.WParam);
       	            bf[2] = 0;
                }
				else if( (Message.WParam == 0x0d) || (Message.WParam == 0x0a) ){
					bf[0] = 0x0d; bf[1] = 0;
                }
				else if( (Message.WParam != '%') && ((Message.WParam >= 0x20) && (Message.WParam <= 0x7f)) ){
					bf[0] = (char)Message.WParam; bf[1] = 0;
				}
        		else {
	    		    wsprintf(bf, "\\x%02x", Message.WParam);
        		}
				for( LPCSTR p = bf; *p; p++ ){
					::PostMessage(hWnd, WM_CHAR, *p & 0x00ff, 0);
				}
            }
        	break;
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PageChange(TObject *Sender)
{
	if( !m_pCodeView ) return;

	m_pCodeView->Visible = (Page->ActivePage == TabChar) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------

 