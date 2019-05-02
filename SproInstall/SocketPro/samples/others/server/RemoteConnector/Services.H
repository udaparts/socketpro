// Services.H : Declaration of the CServices class

#ifndef __SERVICES_H_
#define __SERVICES_H_

class CServicesAccessor
{
public:
	unsigned long m_SvsID;

BEGIN_COLUMN_MAP(CServicesAccessor)
	COLUMN_ENTRY(1, m_SvsID)
END_COLUMN_MAP()

DEFINE_COMMAND(CServicesAccessor, _T("SELECT SvsID FROM Services"))

	// You may wish to call this function if you are inserting a record and wish to
	// initialize all the fields, if you are not going to explicitly set all of them.
	void ClearRecord()
	{
		memset(this, 0, sizeof(*this));
	}
};


#endif // __SERVICES_H_
