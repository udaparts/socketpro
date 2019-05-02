#pragma once

#include <stdio.h>

class CQIndexFile {
public:
	CQIndexFile(const char *fileName);


public:
	unsigned int GetChanged();
	unsigned int Save();


private:
	CQIndexFile(const CQIndexFile &qif);
	CQIndexFile& operator=(const CQIndexFile &qif);

	void EnsureAvailable();


private:
	unsigned int m_changed;
	FILE *m_hFile;
};

