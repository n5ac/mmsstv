//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "ComLib.h"
SYS	sys;
//
//---------------------------------------------------------------------------
//void mcmFunc(DWORD index, FARPROC pFunc)
//
//MMSSTV calls this function at the first time, if the function was exported.
//You can use following functions which are in the MMSSTV.
//
//  index   : Index of the function.
//  pFunc   : Address of the function.
//
//indexes and functions are as follows.
//
//      0 : HBITMAP (__cdecl *mmLoadImageMenu)(int sw, int xw, int yw);
//              sw - 0 : Invoke image-clipper (xw and yw are the size of the bitmap)
//              sw - 1 : no image-clipper (xw and yw will be ignored)
//
//      1 : HBITMAP (__cdecl *mmLoadImage)(LPCSTR pName);
//              pName is a pathname of the image (BMP, JPG, WMF)
//
//      2 : HBITMAP (__cdecl *mmLoadHistImage)(void);
//
//      3 : int (__cdecl *mmSaveImageMenu)(HBITMAP hb, LPCSTR pName, LPCSTR pFolder);
//              pName is a filename of the image (BMP, JPG)
//              pFolder is a folder name (if it is NULL, MMSSTV uses a default)
//
//      4 : int (__cdecl *mmSaveImage)(HBITMAP hb, LPCSTR pName);
//              pName is a pathname of the image (BMP, JPG)
//
//      5 : HBITMAP (__cdecl *mmPasteImage)(int sw, int xw, int yw);
//              sw - 0 : Invoke image-clipper (xw and yw are the size of the bitmap)
//              sw - 1 : no image-clipper (xw and yw will be ignored)
//
//      6 : int (__cdecl *mmGetMacKey)(LPSTR pDest, int cbDest);
//              Invoke macro dialog box and can choose keyword
//
//      7 : int (__cdecl *mmConvMac)(LPSTR pDest, LPCSTR pSrc, int cbDest);
//              Convert macro keyword (pSrc) to the text (pDest)
//
//      8 : HBITMAP (__cdecl *mmCreateDIB)(int xw, int yw);
//              Create 24bit DIB
//
//      9 : void (__cdecl *mmMultProc)(void);
//      10 : void (__cdecl *mmMultProcA)(void);
//              Do decoding and encoding with the DSP in MMSSTV
//
//      11 : int (__cdecl *mmColorComp)(LPDWORD pCol);
//              Invoke color components dialog and can choose a color component
//
extern "C" __declspec(dllexport)
void mcmFunc(DWORD fID, FARPROC pFunc)
{
	switch(fID){
		case 0:
			sys.m_fLoadImageMenu = (mmLoadImageMenu)pFunc;
			break;
		case 2:
			sys.m_fLoadHistImage = (mmLoadHistImage)pFunc;
			break;
		case 9:
			sys.m_fMultProc = (mmMultProc)pFunc;
			break;
		case 13:
			sys.m_fView = (mmView)pFunc;
			break;
		default:
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall MultProc(void)
{
	if( sys.m_fMultProc ) sys.m_fMultProc();
}
//---------------------------------------------------------------------------
Graphics::TBitmap * __fastcall CreateBitmap(int xw, int yw)
{
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
	pBitmap->PixelFormat = pf24bit;
	pBitmap->Width = xw;
	pBitmap->Height = yw;
	return pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall CopyBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc)
{
	pDest->Width = pSrc->Width;
	pDest->Height = pSrc->Height;
	pDest->Palette = NULL;
	pDest->Canvas->Draw(0, 0, pSrc);
}
//---------------------------------------------------------------------------
//アスペクト比を保持する
void __fastcall KeepAspect(int &sx, int &sy, int bx, int by)
{
	double ass = double(sx) / double(sy);
	double asb = double(bx) / double(by);
	if( asb < ass ){    // ソースの方が横長
		sx = (double(sy) * asb) + 0.5;
	}
	else {
		sy = (double(sx) / asb) + 0.5;
	}
}
//---------------------------------------------------------------------------
//アスペクト比を保持する
void __fastcall KeepAspectCopy(Graphics::TBitmap *pDest, int xm, int ym, Graphics::TBitmap *pSrc, int mode)
{
	if( (xm < pSrc->Width) || (ym < pSrc->Height) ){
		KeepAspect(xm, ym, pSrc->Width, pSrc->Height);
		pDest->Width = xm;
		pDest->Height = ym;
		pDest->Palette = NULL;
		TRect tc;
		tc.Left = tc.Top = 0;
		tc.Right = xm; tc.Bottom = ym;
		int Sop = ::SetStretchBltMode(pDest->Canvas->Handle, mode);
		pDest->Canvas->StretchDraw(tc, pSrc);
		::SetStretchBltMode(pDest->Canvas->Handle, Sop);
	}
	else {
		CopyBitmap(pDest, pSrc);
	}
}
//---------------------------------------------------------------------------
void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap)
{
	if( pBitmap == NULL ) return;

	if( (pBitmap->Width != pBox->Width)||(pBitmap->Height != pBox->Height) ){
		int xm = pBox->Width;
		int ym = pBox->Height;
		KeepAspect(xm, ym, pBitmap->Width, pBitmap->Height);
		TRect rc;
		rc.Left = 0; rc.Right = xm;
		rc.Top = 0, rc.Bottom = ym;
		int Sop = ::SetStretchBltMode(pBox->Canvas->Handle, HALFTONE);
		pBox->Canvas->StretchDraw(rc, pBitmap);
		::SetStretchBltMode(pBox->Canvas->Handle, Sop);
	}
	else {
		pBox->Canvas->Draw(0, 0, pBitmap);
	}
}
//
//
//---------------------------------------------------------------------------
// Matrix
//
//  n[] = l[] * m[]
//
static void __fastcall Matrix(double l[4][4], double m[4][4], double n[4][4])
{
	int i, j, k;
	double  d;

	for( i = 0; i < 4; i++ ){
		for( j = 0; j < 4; j++ ){
			for( k = 0, d = 0.0; k < 4; k++ ) d += l[i][k] * m[k][j];
			n[i][j] = d;
		}
	}
}
static void __fastcall Matrix(double l[3][3], double m[3][3], double n[3][3])
{
	int i, j, k;
	double  d;

	for( i = 0; i < 3; i++ ){
		for( j = 0; j < 3; j++ ){
			for( k = 0, d = 0.0; k < 3; k++ ) d += l[i][k] * m[k][j];
			n[i][j] = d;
		}
	}
}
//---------------------------------------------------------------------------
// Perspect
//
void __fastcall SetMatrix(double k[3][3], LPCSPERSPECT pPar, int xs, int ys)
{
	double  l[4][4]; double m[4][4]; double n[4][4];
	double  k1, k2, k3, k4, k5, k6, k7, k8, k9;
	double  u, v, w;

	u = pPar->rx * PI / 180;
	v = pPar->ry * PI / 180;
	w = pPar->rz * PI / 180;
	memset(l, 0, sizeof(l)); memset(m, 0, sizeof(m));
	l[0][0] = 1.0/xs;   l[1][1] = -1.0/xs;  l[2][2] = 1.0;  l[3][3] = 1.0;
	m[0][0] = pPar->ax; m[1][1] = pPar->ay; m[2][2] = 1.0;  m[3][3] = 1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = 1.0;      l[1][1] = 1.0;      l[2][2] = 1.0;
	l[3][0] = pPar->px; l[3][1] = pPar->py; l[3][2] = pPar->pz; l[3][3] = 1.0;
	Matrix(n, l, m);
	memset(n, 0, sizeof(n));
	n[0][0] = cos(w);   n[0][1] = sin(w);
	n[1][0] = -n[0][1]; n[1][1] = n[0][0];
	n[2][2] = 1.0;      n[3][3] = 1.0;
	Matrix(m, n, l);
	memset(m, 0, sizeof(m));
	m[0][0] = 1.0;      m[1][1] = cos(u);   m[1][2] = sin(u);
	m[2][1] = -m[1][2]; m[2][2] = m[1][1];  m[3][3] = 1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = cos(v);   l[0][2] = sin(v);   l[1][1] = 1.0;
	l[2][0] = -l[0][2]; l[2][2] = l[0][0];  l[3][3] = 1.0;
	Matrix(n, l, m);
	memset(n, 0, sizeof(n));
	n[0][0] = 1.0;      n[1][1] = 1.0;      n[2][2] = -1.0; n[3][2] = -pPar->v; n[3][3] = 1.0;
	Matrix(m, n, l);
	memset(m, 0, sizeof(m));
	m[0][0] = 1.0;      m[1][1] = 1.0;      m[2][2] = 1/pPar->s;    m[2][3] = m[2][2];
	m[3][2] = -1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = xs;       l[1][1] = -xs;      l[2][2] = 1.0;      l[3][3] = 1.0;
	Matrix(n, l, m);

	k1 = m[0][3];   k2 = m[1][3];   k3 = m[3][3];
	k4 = m[0][0];   k5 = m[1][0];   k6 = m[3][0];
	k7 = m[0][1];   k8 = m[1][1];   k9 = m[3][1];

	k[0][0] = k7*k2 - k8*k1;   k[0][1] = k5*k1 - k4*k2;   k[0][2] = k4*k8 - k7*k5;
	k[1][0] = k8*k3 - k9*k2;   k[2][0] = k9*k1 - k7*k5;   k[1][1] = k6*k2 - k5*k3;
	k[2][1] = k4*k3 - k6*k1;   k[1][2] = k5*k9 - k8*k6;   k[2][2] = k7*k6 - k4*k9;

	if( pPar->r != 0.0 ){       // simple rotation
		double r = pPar->r * PI / 180.0;
		double  m[3][3];
		double  z[3][3];
		memset(m, 0, sizeof(m));
		m[0][0] = cos(r); m[0][1] = sin(r);
		m[1][0] = -m[0][1]; m[1][1] = m[0][0];
		m[2][2] = 1.0;
		Matrix(k, m, z);
		memcpy(k, z, sizeof(z));
	}
}
#define TMAX    400
//---------------------------------------------------------------------------
// Create mapping table
LPCDOUBLE __fastcall MapTable(int wave, int xs, int ys, double ks)
{
#define VMAX    32767.0
#define VMIN    (1.0/VMAX)
	LPDOUBLE  pTbl, dp;
	double d;
	switch(wave){
		case 6:
			{
				double  ky = 0.5 * PI / ys;
				dp = pTbl = new double[ys*2];
				for( int y = -ys; y < ys; y++, dp++ ){
					d = cos(y * ky);
					if( ABS(d) >= VMIN ){
						d = 1.0 * ks / d;
					}
					else {
						d = VMAX;
					}
					*dp = d * 0.8;
				}
			}
			break;
		case 9:
			{
				dp = pTbl = new double[xs*2];
				double kd = xs * xs;
				for( int x = -xs; x < xs; x++, dp++ ){
					d = kd - x * x;
					if( d >= 0 ){
						d = sqrt(d);
						*dp = (d > VMIN) ? xs * ks / d : VMAX;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 14:
			{
				dp = pTbl = new double[ys*2];
				double kd = ys * ys;
				for( int y = -ys; y < ys; y++, dp++ ){
					d = kd - y * y;
					if( d >= 0 ){
						d = sqrt(d);
						*dp = (d > VMIN) ? ys / d : VMAX;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 10:
		case 11:
			{
				dp = pTbl = new double[xs*2];
				double kd = xs * xs;
				double kc = xs * 0.25;
				for( int x = -xs; x < xs; x++, dp++ ){
					d = kd - x * x;
					if( d >= 0 ){
						*dp = sqrt(d) * 0.5 * ks - kc;
						if( wave == 10 ) *dp = -*dp;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 7:
		case 8:
		case 12:
		case 13:
		case 15:
        case 25:
			pTbl = NULL;
			break;
		case 16:
			{
				dp = pTbl = new double[ys*2];
				double kc = ks * PI /(4 * ys);
				double d;
				for( int y = -ys; y < ys; y++, dp++ ){
					d = fabs(sin((y + ys)*kc));
					*dp = d > VMIN ? 1.0 / d : VMAX;
				}
			}
			break;
		case 17:
			{
				dp = pTbl = new double[ys*2];
				double kc = ks * PI /(4 * ys);
				double d;
				for( int y = ys-1; y >= -ys; y--, dp++ ){
					d = fabs(sin((y + ys)*kc));
					*dp = d > VMIN ? 1.0 / d : VMAX;
				}
			}
			break;
		case 22:
			{
				dp = pTbl = new double[ys*2];
				double kc = PI * (ks-0.85)/ (ys * ys);
				double d;
				for( int y = -ys; y < ys; y++, dp++ ){
					d = y * y * kc + (PI/4);
					*dp = ABS(d) >= VMIN ? (1.0 / d) : VMAX;
				}
			}
			break;
        case 28:
        case 29:
			{
				dp = pTbl = new double[xs*2];
				double kd = xs * xs;
                double kc = ks*0.5 + 0.05;
				for( int x = -xs; x < xs; x++, dp++ ){
					d = kd - x * x;
					if( d >= 0 ){
						d = sqrt(d);
						*dp = (d > VMIN) ? xs * kc / d : VMAX;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 36:
			{
				dp = pTbl = new double[xs*2];
				double kx = PI * 0.5 / xs;
				double ky = ys * ks * 0.125;
                double ko = ks * ys * 0.1875;
				for( int x = -xs; x < xs; x++, dp++ ){
					*dp = ky * ((xs-x) * kx) - ko;
				}
			}
			break;
		case 38:
			{
				dp = pTbl = new double[ys*2];
				double ky = PI * 0.5 / ys;
				double kx = xs * ks * 0.25;
                double ko = ks * xs * 0.375;
				for( int y = -ys; y < ys; y++, dp++ ){
					*dp = kx * ((ys-y) * ky) - ko;
				}
			}
			break;
		default:
			{
				double  kx = PI / xs;
				double  ky = ys * ks/ 8;
				dp = pTbl = new double[xs*2];
				for( int x = -xs; x < xs; x++, dp++ ){
					switch(wave){
						case 1:
							*dp = ky * sin(x * kx);
							break;
						case 2:
							*dp = ky * sin(-x * kx);
							break;
						case 3:
							*dp = ky * sin(x * kx + PI/2);
							break;
						case 4:
							*dp = ky * sin(-x * kx - PI/2);
							break;
						case 5:
							d = cos(x * kx * 0.5);
							if( ABS(d) >= VMIN ){
								d = 1.0 * ks/ d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 18:
							d = ABS(x) * kx + (PI/2) * ks;
							if( ABS(d) >= VMIN ){
								d = PI / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 19:
                        case 34:
                        case 35:
							d = fabs(sin(x * kx)) + 0.25 * ks;
							if( ABS(d) >= VMIN ){
								d = 1.0 / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 20:
                        case 30:
                        case 32:
							d = (x+xs) * kx * 0.5 + (PI/2) * ks;
							if( ABS(d) >= VMIN ){
								d = PI / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 21:
                        case 31:
                        case 33:
							d = (xs-x) * kx * 0.5 + (PI/2) * ks;
							if( ABS(d) >= VMIN ){
								d = PI / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 23:
							d = x * x * kx * kx + (PI * (ks+1.0));
							*dp = ABS(d) >= VMIN ? (PI * 2.0 / d) : VMAX;
							break;
                        case 24:
                        case 26:
                        case 27:
							d = (xs-ABS(x)) * kx + (PI/2) * ks;
							*dp = ABS(d) >= VMIN ? (PI * 0.8 / d) : VMAX;
                        	break;
						case 37:
							*dp = ky * (ABS(x) * kx * 0.5);
							break;
						default:
							break;
					}
				}
			}
			break;
	}
	return pTbl;
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CItem::Perspect(Graphics::TBitmap* pDest, Graphics::TBitmap *pSrc)
{
	ASSERT(pSrc->PixelFormat == pf24bit);

	Graphics::TBitmap *pSrcBak = pSrc;
	TColor back = m_Back;
	LPCSPERSPECT pPar = &m_sperspect;
	if( (pDest == NULL) || (pDest->PixelFormat != pf24bit) ){
		if( pDest != NULL ) delete pDest;
		pDest = new Graphics::TBitmap;
		pDest->PixelFormat = pf24bit;
	}

	if( (pSrc->Width < 2) || (pSrc->Height < 2) ){
		pDest->Width = pSrc->Width;
		pDest->Height = pSrc->Width;
		CopyBitmap(pDest, pSrc);
	}
	else {
		int flag = pPar->flag & 0x0000ffff;
		int wave = pPar->flag >> 16;
		int bw = (pSrc->Width & 1) ? pSrc->Width - 1 : pSrc->Width;
		int bh = (pSrc->Height & 1) ? pSrc->Height - 1 : pSrc->Height;
		pDest->Width = bw;
		pDest->Height = bh;
		int i, j, m, n;
		double  x, y, w, p, q;
		double  k[3][3];
		int xs = bw / 2;
		int ys = bh / 2;
		SetMatrix(k, pPar, xs, ys);
		BYTE *sp1, *sp2;
		BYTE *tp;

		BYTE br, bg, bb;
		if( flag == 2 ){		// Overlay on source
			tp = (BYTE *)pSrc->ScanLine[0];
			bb = *tp++;
			bg = *tp++;
			br = *tp;
		}
		else {						// Overlay on template
			bb = (BYTE)(DWORD(back)>>16);
			bg = (BYTE)(DWORD(back)>> 8);
			br = (BYTE)(DWORD(back));
		}
		if( m_Frame ){
			pSrc = CreateBitmap(pSrcBak->Width, pSrcBak->Height);
			CopyBitmap(pSrc, pSrcBak);
			int ww = (m_Frame + 1) * 2;
			TRect rc;
			rc.Left = ww/2; rc.Top = ww/2;
			rc.Right = pSrc->Width - ww/2; rc.Bottom = pSrc->Height - ww/2;
			pSrc->Canvas->Pen->Color = m_FrameCol;
			pSrc->Canvas->Pen->Width = ww;
			pSrc->Canvas->MoveTo(rc.Left, rc.Top);
			pSrc->Canvas->LineTo(rc.Right, rc.Top);
			pSrc->Canvas->LineTo(rc.Right, rc.Bottom);
			pSrc->Canvas->LineTo(rc.Left, rc.Bottom);
			pSrc->Canvas->LineTo(rc.Left, rc.Top);
		}
		int r, g, b;
		LPCDOUBLE pTbl = NULL;
		double ks = 1.0 + (double(m_SC) * 0.05);
		if( wave ) pTbl = MapTable(wave, xs, ys, ks);
		double kc, kc3, kx, kc7;
		switch(wave){
			case 7:
			case 8:
				kc7 = ((wave == 7) ? 0.6 : 0.8)*double(ys)/double(xs);
				kx = xs * ((wave == 7) ? 4.0 * ks : 2.994) / PI;
				kc3 = ys/3;
				if( wave == 8 ){
					kc7 *= ks;
					kc3 *= ks;
				}
				break;
			case 13:
				if( m_SC >= (88-30) ) m_SC = 88-30;
				if( m_SC <= -30 ) m_SC = -29;
				kc = 30 + m_SC;
				kc3 = 1.0/tan(kc*PI/180);
				kx = 1.0/cos(kc*PI/180);
				kc7 = 1.0/sin(kc*PI/180);
				break;
			case 14:
				kc3 = ys * 7 /8;
				break;
			case 15:
				kc3 = ks + 0.75;
				break;
	        case 25:
				kc7 = 0.6*double(ys)/double(xs);
				kx = xs * (4.0 * ks) / PI;
				kc3 = ys/3;
				break;
    	    case 26:
    	    case 27:
    	    case 34:
    	    case 35:
				kc3 = (ABS(ks) > VMIN) ? (1.0 / ks) : VMAX;
    	       	break;
    	    case 28:
    	    case 29:
				kc3 = (ABS(ks) > VMIN) ? (ks) : VMAX;
    	        break;
    	    case 30:
    	    case 31:
    	    case 32:
    	    case 33:
				kc3 = (ABS(ks) > VMIN) ? (0.8 / ks) : VMAX;
    	        break;
        }
		kc = double(xs)/double(ys);
		for( i = -ys; i < ys; i++ ){
			tp = (BYTE *)pDest->ScanLine[i+ys];
			for( j = -xs; j < xs; j++ ){
				w = k[0][0] * j + k[0][1] * i + k[0][2];
				x = k[1][0] * j + k[1][1] * i + k[1][2];
				y = k[2][0] * j + k[2][1] * i + k[2][2];
				if( w ){
					x = x / w;
					y = y / w;
        	    }
        	    else {
					x = y = 10000.0;
        	    }
				if( wave ){
					switch(wave){
						case 1:
						case 2:
						case 3:
						case 4:
							ASSERT(pTbl);
							m = x + xs;
							if( m >= bw ) m = bw-1;
							if( m < 0 ) m = 0;
							y += pTbl[m];
							break;
						case 5:
							ASSERT(pTbl);
							x *= 0.8;
							m = x + xs;
							if( m >= bw ) m = bw-1;
							if( m < 0 ) m = 0;
							y *= pTbl[m];
							break;
						case 6:
							ASSERT(pTbl);
							y *= 0.8;
							m = y + ys;
							if( m >= bh ) m = bh-1;
							if( m < 0 ) m = 0;
							x *= pTbl[m];
							break;
						case 7:
						case 8:
							if( wave == 7 ){
								y -= ys;
							}
							x *= kc7;
							y *= kc7;
							q = -y * kc;
							w = (y||x) ? atan2(x, q) : 0.0;
							p = sqrt(q * q + x * x);
							x = w * kx;
							if( p >= kc3 ){
								if( wave == 8 ){
									x += xs;
									if( x < 0 ) x += 2*xs;
									x = fmod(x, 2*xs-1);
									x -= xs;
								}
								p = (p - kc3) * 3;
								y = (-(p) + ys);
							}
							else {
								y = ys;
							}
							break;
						case 9:
							ASSERT(pTbl);
							m = x + xs;
							if( m >= bw ) m = bw-1;
							if( m < 0 ) m = 0;
							y *= pTbl[m];
							break;
						case 10:
						case 11:
		                case 36:
        		        case 37:
							ASSERT(pTbl);
							m = x + xs;
							if( m >= bw ) m = bw-1;
							if( m < 0 ) m = 0;
							y += pTbl[m];
							break;
						case 12:
							x *= 0.8;
							p = (y+ys)*0.5;
							w = xs * xs - x * x;
							if( w >= 0 ){
								w = sqrt(w) * ks/2.0 - xs*0.25;
								y -= w;
							}
							else {
								y = ys;
							}

							w = ys * ys - p * p;
							if( w > 0 ){
								w = ys / sqrt(w);
								x *= w;
							}
							else {
								x = xs;
							}
							break;
						case 13:
							x += xs / 3;
							y += ys / 3;
							x *= 0.75;
							y *= 0.75;
							if( (y >= 0) && (x >= 0) ){   // C
								y = y * 2 - ys;
								x = x * 2 - xs;
							}
							else if( (-y*kc3 >= -x) ){     // A
								x = (x - y*kc3) * 2  - xs;
								y = y * kc7 * 3 + ys;
							}
							else {                        // B
								y = (y - x/kc3) * 2 - ys;
								x = x * kx * 3 * kc + xs;
							}
//                            m = int(y); n = int(x);
//                            if( (n ==(-xs+1))||(n==(xs-1))||(m==(-ys+1))||(m==(ys-1)) ){
//                                x = xs-1; y = ys-1;
//                            }
							break;
						case 14:
							y += kc3;
							w = (x||y) ? atan2(x, y*kc) : 0.0;
							p = fabs(ks + cos(w));
							p = (p > VMIN) ? 1.0 / p : VMAX;
							y *= p;
							ASSERT(pTbl);
							m = y;
							if( m >= ys ) m = ys-1;
							if( m < -ys ) m = -ys;
							x *= p * pTbl[m+ys];
							break;
						case 15:
							x *= 1.7; y *= 1.7;
							p = fabs(x); q = fabs(y);
							w = (x||y) ? atan2(p, q*kc) : 0.0;
							p = 1 + sin(w * kc3);
							p = (p > VMIN) ? 1.0 / p : VMAX;
							x *= p;
							y *= p;
							break;
						case 16:
						case 17:
                        case 22:
							ASSERT(pTbl);
							m = y + ys;
							if( m >= bh ) m = bh-1;
							if( m < 0 ) m = 0;
							x *= pTbl[m];
							break;
						case 18:
        		        case 20:
        		        case 21:
        		        case 24:
							ASSERT(pTbl);
                            y *= 1.50;
							m = x + xs;
							if( m >= bw ) m = bw-1;
							if( m < 0 ) m = 0;
							y *= pTbl[m];
							break;
        		        case 19:
							ASSERT(pTbl);
                            y *= 1.30;
							m = x + xs;
							if( m >= bw ) m = bw-1;
							if( m < 0 ) m = 0;
							y *= pTbl[m];
							break;
						case 23:
							ASSERT(pTbl);
                            y *= 2.00;
							m = x + xs;
							if( m >= bw ) m = bw-1;
							if( m < 0 ) m = 0;
							y *= pTbl[m];
							break;
        		        case 25:
							y += ys;
							x *= kc7;
							y *= kc7;
							q = y * kc;
							w = (y||x) ? atan2(x, q) : 0.0;
							p = sqrt(q * q + x * x);
							x = w * kx;
							if( p >= kc3 ){
								p = (p - kc3) * 3;
								y = p - ys;
							}
							else {
								y = -ys;
							}
							break;
        		        case 26:
        		        case 28:
        		        case 30:
        		        case 31:
        		        case 34:
							ASSERT(pTbl);
        		            if( y < 0 ){
								y *= 1.3;
								m = x + xs;
								if( m >= bw ) m = bw-1;
								if( m < 0 ) m = 0;
								y *= pTbl[m];
        		            }
        		            else {
								y *= kc3;
        		            }
							break;
        		        case 27:
        		        case 29:
        		        case 32:
        		        case 33:
        		        case 35:
							ASSERT(pTbl);
        		            if( y > 0 ){
								y *= 1.3;
								m = x + xs;
								if( m >= bw ) m = bw-1;
								if( m < 0 ) m = 0;
								y *= pTbl[m];
        		            }
        		            else {
								y *= kc3;
        		            }
							break;
        		        case 38:
							ASSERT(pTbl);
							m = y + ys;
							if( m >= bh ) m = bh-1;
							if( m < 0 ) m = 0;
							x += pTbl[m];
							break;
						default:
							break;
					}
				}
				if( y > 0 ){ m = int(y); } else { m = int(y-1); }
				if( x > 0 ){ n = int(x); } else { n = int(x-1); }
				q = y - m;
				p = x - n;
				double pp = 1.0 - p;
				double qq = 1.0 - q;
				if( (m >= -ys) && (m < ys-1) && (n >= -xs) && (n < xs-1) ){
					sp1 = (BYTE *)pSrc->ScanLine[m+ys] + (n+xs)*3;
					sp2 = (BYTE *)pSrc->ScanLine[m+ys+1] + (n+xs)*3;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						b = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
						if( b < 0 ) b = 0;
						if( b > 255 ) b = 255;
					}
					else {
						b = *sp1;
					}
					sp1++; sp2++;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						g = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
						if( g < 0 ) g = 0;
						if( g > 255 ) g = 255;
					}
					else {
						g = *sp1;
					}
					sp1++; sp2++;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						r = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
						if( r < 0 ) r = 0;
						if( r > 255 ) r = 255;
					}
					else {
						r = *sp1;
					}
					if( (br == r) && (bg == g) && (bb == b) && (flag != 2) ){
						if( r ){ r--; }
						else if( g ){ g--; }
						else if( b ){ b--; }
						else { r++; }
					}
					*tp++ = BYTE(b);
					*tp++ = BYTE(g);
					*tp++ = BYTE(r);
				}
				else {
					*tp++ = bb;
					*tp++ = bg;
					*tp++ = br;
				}
			}
		}
		if( pTbl != NULL ) delete pTbl;
	}
	if( pSrcBak != pSrc ) delete pSrc;
	return pDest;
}
//
//
//---------------------------------------------------------------------------
// CItem class
//
__fastcall CItem::CItem(void)
{
	m_pMemStream = NULL;
	m_pBitmap = NULL;

	// Init as default
	m_sperspect.ver = 3;
	m_sperspect.flag = 0;
	m_sperspect.ax = 3.0; m_sperspect.ay = 3.0;
	m_sperspect.px = m_sperspect.py = m_sperspect.pz = 0.0;
	m_sperspect.rz = -180; m_sperspect.rx = m_sperspect.ry = 0;
	m_sperspect.v = 10; m_sperspect.s = 2.3;
	m_sperspect.r = 0.0;
	m_SC = 0;

	m_Back = clBlack;
	m_OrgXW = m_OrgYW = 16;
	m_Draft = 0;

	m_Frame = 0;
	m_FrameCol = clWhite;
}
//---------------------------------------------------------------------------
__fastcall CItem::~CItem()
{
	if( m_pMemStream != NULL ) delete m_pMemStream;
	if( m_pBitmap != NULL ) delete m_pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall CItem::Create(LPCBYTE ps, DWORD size)
{
	if( (ps != NULL) && size ){
		LPSPERSPECT lps = (LPSPERSPECT)ps;
		if( lps->ver <= m_sperspect.ver ){
			int ver = m_sperspect.ver;
			if( !(lps->flag & 0x0000ffff) && (size <= sizeof(SPERSPECT)) ){
				memcpy(&m_sperspect, lps, size);
				m_sperspect.flag &= 0xffff0000;
			}
			else {
				TMemoryStream *pMem = new TMemoryStream;
				try{
					pMem->Clear();
					pMem->Write(ps, size);
					pMem->Seek(soFromBeginning, 0);
					SPERSPECT	sperspect;
					pMem->Read(&sperspect, sizeof(SPERSPECT));
					memcpy(&m_sperspect, &sperspect, sizeof(m_sperspect));
					m_SC = 0;
					if( m_sperspect.ver >= 2 ){
						pMem->Read(&m_SC, sizeof(m_SC));
					}
					if( m_sperspect.flag & 0x0000ffff ){
						if( m_pBitmap == NULL ) m_pBitmap = CreateBitmap(16, 16);
						m_pBitmap->LoadFromStream(pMem);
						m_OrgXW = m_pBitmap->Width;
						m_OrgYW = m_pBitmap->Height;
					}
					m_Frame = 0;
					m_FrameCol = clWhite;
					if( m_sperspect.ver >= 3 ){
						pMem->Read(&m_Frame, sizeof(m_Frame));
						pMem->Read(&m_FrameCol, sizeof(m_FrameCol));
					}
				}
				catch(...){
					ASSERT(0);
				}
				delete pMem;
			}
			m_sperspect.ver = ver;
		}
	}
}
//---------------------------------------------------------------------------
DWORD __fastcall CItem::GetItemType(void)
{
	return (m_sperspect.flag & 0x0000ffff) ? 0x00010000 : 0x00010001;
}
//---------------------------------------------------------------------------
LPCBYTE __fastcall CItem::CreateStorage(LPDWORD pSize)
{
	ASSERT(pSize);

	if( m_pMemStream != NULL ) delete m_pMemStream;
	m_pMemStream = new TMemoryStream;
	m_pMemStream->Clear();
	m_pMemStream->Write(&m_sperspect, sizeof(m_sperspect));
	m_pMemStream->Write(&m_SC, sizeof(m_SC));
	if( (m_sperspect.flag & 0x0000ffff) && (m_pBitmap != NULL) ){
		m_pBitmap->SaveToStream(m_pMemStream);
	}
	m_pMemStream->Write(&m_Frame, sizeof(m_Frame));
	m_pMemStream->Write(&m_FrameCol, sizeof(m_FrameCol));
	*pSize = m_pMemStream->Position;
	return (LPCBYTE)m_pMemStream->Memory;
}
//---------------------------------------------------------------------------
void __fastcall CItem::DeleteStorage(LPCBYTE pStorage)
{
	ASSERT(pStorage);

	if( m_pMemStream != NULL ){
		ASSERT(pStorage == (LPCBYTE)m_pMemStream->Memory);
		delete m_pMemStream;
		m_pMemStream = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall CItem::Copy(CItem *sp)
{
	ASSERT(sp);

	memcpy(&m_sperspect, &sp->m_sperspect, sizeof(m_sperspect));
	m_Frame = sp->m_Frame;
	m_FrameCol = sp->m_FrameCol;
	m_SC = sp->m_SC;
	if( sp->m_pBitmap != NULL ){
		CopySource(sp->m_pBitmap);
	}
	m_Back = sp->m_Back;
	m_OrgXW = sp->m_OrgXW;
	m_OrgYW = sp->m_OrgYW;
}
//---------------------------------------------------------------------------
void __fastcall CItem::CopySource(Graphics::TBitmap *pSrc)
{
	ASSERT(pSrc);

	int xm = pSrc->Width;
	int ym = pSrc->Height;
	if( xm > 400 ) xm = 400;
	if( ym > 320 ) ym = 320;
	if( m_pBitmap == NULL ) m_pBitmap = CreateBitmap(320, 256);
	KeepAspectCopy(m_pBitmap, xm, ym, pSrc, ((m_sperspect.flag & 3)!=2) ? HALFTONE : COLORONCOLOR);
	m_OrgXW = m_pBitmap->Width;
	m_OrgYW = m_pBitmap->Height;
}
//---------------------------------------------------------------------------
void __fastcall CItem::CopyFromHistory(void)
{
	ASSERT(sys.m_fLoadHistImage);

	if( m_pBitmap == NULL ) m_pBitmap = CreateBitmap(16, 16);
	m_pBitmap->Handle = sys.m_fLoadHistImage();
	m_OrgXW = m_pBitmap->Width;
	m_OrgYW = m_pBitmap->Height;
}
//---------------------------------------------------------------------------
void __fastcall CItem::DeleteSource(void)
{
	if( m_pBitmap != NULL ){
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
}
//---------------------------------------------------------------------------
DWORD __fastcall CItem::GetOrgSize(void)
{
	DWORD w = m_OrgXW;
	DWORD h = m_OrgYW;
	return (h << 16) + w;
}
//---------------------------------------------------------------------------
void __fastcall CItem::Draw(Graphics::TBitmap *pDest)
{
	if( m_pBitmap == NULL ) return;       // No source image

	m_Back = pDest->Canvas->Pixels[0][0];


	Graphics::TBitmap *pSrc = m_pBitmap;
	if( m_Draft ){
		int xm = pSrc->Width;
		int ym = pSrc->Height;
		if( xm > 80 ) xm = 80;
		if( ym > 64 ) ym = 64;
		pSrc = CreateBitmap(xm, ym);
		KeepAspectCopy(pSrc, xm, ym, m_pBitmap, COLORONCOLOR);
	}
	// Do perspect
	Graphics::TBitmap *pBitmap = Perspect(NULL, pSrc);
	if( pSrc != m_pBitmap ) delete pSrc;

	// pBitmap -> pDest
	TRect rc;
	rc.Left = rc.Top = 0;
	rc.Right = pDest->Width;
	rc.Bottom = pDest->Height;
	int Sop = ::SetStretchBltMode(pDest->Canvas->Handle, COLORONCOLOR);
	pDest->Canvas->StretchDraw(rc, pBitmap);
	::SetStretchBltMode(pDest->Canvas->Handle, Sop);

	delete pBitmap;

	// Delete source for saving resource
	if( !(m_sperspect.flag & 0x0000ffff) ) DeleteSource();

	// set transparent color to (0,0) for overlay
	if( (m_sperspect.flag & 0x0000ffff) != 2 ){
		LPBYTE p = (LPBYTE)pDest->ScanLine[0];
		ASSERT(p);
		*p++ = (BYTE)(DWORD(m_Back)>>16);
		*p++ = (BYTE)(DWORD(m_Back)>> 8);
		*p = (BYTE)(DWORD(m_Back));
	}
}
///------------------------------------------------------
///
///CWaitCursor クラス
///
CWaitCursor::CWaitCursor()
{
	sv = Screen->Cursor;
	Screen->Cursor = crHourGlass;
}

CWaitCursor::~CWaitCursor()
{
	Screen->Cursor = sv;
}

void CWaitCursor::Delete(void)
{
	Screen->Cursor = sv;
}

void CWaitCursor::Wait(void)
{
	Screen->Cursor = crHourGlass;
}

