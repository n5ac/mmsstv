#ifndef CountryH
#define	CountryH

extern LPSTR StrDupe(LPCSTR s);
//----------------------------------------------------------------------------

#define	CTMAX	512
typedef struct {
	LPSTR	Name;
	LPSTR	Code;
	LPSTR	QTH;
	LPSTR	Cont;
	LPSTR	TD;
}CTL;

class CCountry
{
private:
	int		cmax;
	CTL		ctl[CTMAX];

	char	wbf[512];
public:
	CCountry();
	~CCountry(void);

	inline CTL *GetCTL(int n){ return &ctl[n];};
	void 	Init(void);
	void 	Free(void);
	int 	GetNo(LPCSTR s);
	int 	GetNoP(LPCSTR p);
	int     GetRefP(LPCSTR p);
	LPCSTR  GetCountry(int n);
	LPCSTR  GetCont(int n);

	LPCSTR	GetCountry(LPCSTR p);
	LPCSTR	GetCont(LPCSTR p);
	void	Load(LPCSTR fm);
	inline	int IsData(void){
		return cmax;
	};
};

extern	CCountry	Cty;
#endif

