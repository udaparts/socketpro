#ifndef	___UID_METHOD_H__
#define ___UID_METHOD_H__

#define cTab		((TCHAR)9)
#define cLine		((TCHAR)10)
#define	cReturn		((TCHAR)13)
#define cBlank		_T(' ')

enum tagDataType
{
	dtUnknown = 0,
	dtVoid,
	dtBool,
	dtByte,
	dtAChar,
	dtUShort,
	dtShort,
	dtWChar,
	dtUInt,
	dtInt,
	dtULong,
	dtLong,
	dtFloat,
	dtDouble,
	dtDecimal,
	dtString,
	dtAString,
	dtGuid,
	dtObject,
	dtUDT,
};

enum tagParameterType
{
	ptUnknown = 0,
	ptIn = 0x1,
	ptOut = 0x2,
	ptInOut = (ptIn | ptOut),
};

enum tagLanguage
{
	lCSharp = 0,
	lCpp,
	lVBNet,
	lCLI,
};

struct CParameter
{
public:
	tagDataType			m_dt;
	tagParameterType	m_pt;
	TCHAR				m_strName[256];
	TCHAR				m_strType[256];
};

class CMethod
{
public:
	CMethod(short sRequestID = 0);
	CMethod(const CMethod &Method);
	virtual ~CMethod();

public:
	CMethod& operator=(const CMethod &Method);
	bool operator==(const CMethod &Method);
	bool operator!=(const CMethod &Method);

public:
	bool SetDeclaration(LPCTSTR strDeclaration);
	tagLanguage GetLanguage(){return m_lang;}
	short GetRequestID(){return m_sReqestID;}
	void SetRequestID(short sRequestID){m_sReqestID = sRequestID;}
	bool IsSlow() {return m_bSlow;}
	const CString& GetName(){return m_strName;}
	tagDataType GetReturn() {return m_dtReturn;}
	LPCTSTR GetReturnType(){return m_strReturnType;}
	LPCTSTR GetDeclaration(){return LPCTSTR(m_strDeclaration);}
	LPCTSTR GetErrorMessage(){return m_strErrorMessage;}
	LPCTSTR	GetServiceName(){return m_strServiceName;}
	void SetServiceName(LPCTSTR strServiceName){m_strServiceName = strServiceName;}
	const CString& GetRtnProcessing(){return m_strRtnProcessing;}
	bool	AlreadyExist(const CString strParameterName);
	void SetLang(tagLanguage lang){m_lang = lang;}
	void SetDefClass(LPCTSTR strDefClass){m_strDefClass = strDefClass;}
	LPCTSTR GetDefClass(){return m_strDefClass;}
	static bool IsValid(const CString &strName);
	static tagDataType GetDataType(const CString &strType);
	int GetOutputCount(){return m_nOutput;}

public:
	LPCTSTR GetSynFunCpp();
	LPCTSTR GetSynFunCppNew();
	CString GetMethodForPeerCpp();
	CString	GetPeerCallCpp();
	CString	GetPeerCallCppEx(int &nInput, int &nOutput);
	CString GetClientAsynInputMethodCpp();

public:
	CString GetClientAsynInputMethodCSharp();
	LPCTSTR GetSynFunCSharp();
	LPCTSTR GetSynFunCSharpNew();
	CString	GetPeerCallCSharp();
	CString	GetPeerCallCSharpEx(int &nInput, int &nOutput);
	CString GetMethodForPeerCSharp();

public:
	CString GetClientAsynInputMethodVBNet();
	LPCTSTR GetSynFunVBNet();
	LPCTSTR GetSynFunVBNetNew();
	CString GetMethodForPeerVBNet();
	CString	GetPeerCallVBNet();
	CString	GetPeerCallVBNetEx(int &nInput, int &nOutput);

private:
	CString GetInputString(tagDataType dt);

private:
	void GetInputParameters(CSimpleArray<CParameter> &aParameter);
	void GetOutputParameters(CSimpleArray<CParameter> &aParameter);

private:
	tagLanguage				m_lang;
	CString					m_strRtnProcessing;
	CString					m_strSynFun;
	CSimpleArray<CParameter> m_aParameter;
	CString					m_strReturnType;
	short					m_sReqestID;
	bool					m_bSlow;
	tagDataType				m_dtReturn;
	CString					m_strName;
	CString					m_strServiceName;
	CString					m_strDeclaration;
	CString					m_strErrorMessage;
	CString					m_strDefClass;
	int						m_nOutput;
};

#endif
