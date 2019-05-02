// Users.H : Declaration of the CUsers class

#ifndef __USERS_H_
#define __USERS_H_

class CUsersAccessor
{
public:
	WCHAR m_Password[256];

BEGIN_COLUMN_MAP(CUsersAccessor)
	COLUMN_ENTRY(1, m_Password)
END_COLUMN_MAP()

/*
DEFINE_COMMAND(CUsersAccessor, _T(" \
	SELECT \
		UserID, \
		Password  \
		FROM Users"))*/

	// You may wish to call this function if you are inserting a record and wish to
	// initialize all the fields, if you are not going to explicitly set all of them.
	void ClearRecord()
	{
		memset(this, 0, sizeof(*this));
	}
};

#endif // __USERS_H_
