#include "stdafx.h"
#include "qindexfile.h"

CQIndexFile::CQIndexFile(const char *fileName)
: m_changed(0), m_hFile(NULL) {
	m_hFile = ::fopen(fileName, "r+b");
	if (m_hFile == NULL) {
		m_hFile = ::fopen(fileName, "w+b");
	}
	EnsureAvailable();
}

void CQIndexFile::EnsureAvailable() {
	if (m_hFile == NULL)
		throw CMBExCode("Can not create a file to store message queue!", MB_CAN_NOT_CREATE_FILE);
}

unsigned int CQIndexFile::GetChanged() {
	return m_changed;
}

unsigned int CQIndexFile::Save() {
	return 0;
}