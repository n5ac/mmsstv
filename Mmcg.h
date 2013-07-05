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



#ifndef MmcgH
#define	MmcgH
#include "LogFile.h"

#ifndef LPCUSTR
typedef const unsigned char *	LPCUSTR;
typedef unsigned char *	LPUSTR;
#endif

typedef struct {
	LPSTR	Code;
	LPSTR	QTH;
	LPSTR	Key;
}MMCG;

class CMMCG
{
private:
	int		m_Max;	// 現在確保しているスロット数
	int		m_Cnt;	// 現在記憶しているスロット数
	MMCG	*m_bp;	// MMCGの配列


	char	wbf[512];

public:
	int		m_mask;
	int		m_sinc;
	int		m_FindCnt;
	MMCG	**m_fp;

	AnsiString	m_QTH;
	char	m_Call[MLCALL+1];
private:
	void Free(void);
	void Alloc(int n);
	LPSTR adjspl(LPSTR p);
	int	chkspl(LPCSTR p);
	int LastMatch(LPSTR t, LPSTR s);
	int NormQth(LPSTR p);
	int isfind(LPCSTR p);
	int _ischrtyp(char *p, int x);
	LPCSTR getprf(LPCSTR pCode);
	void GetGun(LPSTR t, LPCSTR pCode);
	int isEnt(MMCG *mp);

public:
	CMMCG();
	~CMMCG();

	void LoadDef(LPCSTR pFileName);
	void FindAll(void);
	void FindQTH(LPSTR pKey);
	void FindNumb(LPSTR pKey);
	void Find(LPSTR p);
	void SetMask(void);

	LPCSTR GetQTH(MMCG *mp);
};

extern	CMMCG	mmcg;
#endif
