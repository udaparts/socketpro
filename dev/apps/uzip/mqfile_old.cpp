
//#include "stdafx.h"
#include "../../pinc/mqfile.h"
#include <string>
#include <assert.h>
#include "../../pinc/sha1.h"
#include <algorithm>
#include "../../include/membuffer.h"

using namespace std;

namespace MQ_FILE {

    CMqFile::CMqFile(const char *fileName, unsigned short port)
    : m_CurrentReadPos(0),
    m_fileName(fileName),
    m_hFile(NULL),
    m_qOut(*m_suOut),
    m_nInternalIndex(0),
    m_UQueue(*m_su) {
        char str[128 + 1] = {0};

#ifdef WIN32_64
		sprintf_s(str, "_%d.umq", sizeof(str), port);
#else
        sprintf(str, "_%d.umq", port);
#endif
        m_fileName += str;

        SetFileHandlers();
        if (IsAvailable())
            SetInitialEnqueuePosition();
    }

    CMqFile::~CMqFile() {
        CloseFile();
    }

    void CMqFile::CloseFile() {
        if (m_hFile != NULL) {
            ::fclose(m_hFile);
            m_hFile = NULL;
        }
    }

    void CMqFile::SetFileHandlers() {
        m_hFile = ::fopen(m_fileName.c_str(), "r+b");
        if (m_hFile == NULL) {
            m_hFile = ::fopen(m_fileName.c_str(), "w+b");
        }
    }

    MB::U_INT64 CMqFile::GetFileSize() {
        if (m_hFile != NULL) {
#ifdef WIN32_64
            ::_fseeki64(m_hFile, 0, SEEK_END);
            return _ftelli64(m_hFile);
#else
            ::fseeko64(m_hFile, 0, SEEK_END);
            return ftello64(m_hFile);
#endif
        }
        return 0;
    }

	/*
    CMqFile& CMqFile::operator=(CMqFile && mq) {
        MB::CAutoLock al(m_cs);
        std::swap(m_CurrentReadPos, mq.m_CurrentReadPos);
        std::swap(m_fileName, mq.m_fileName);
        std::swap(m_hFile, mq.m_hFile);
        std::swap(m_qOut, mq.m_qOut);
        std::swap(m_nInternalIndex, mq.m_nInternalIndex);
        return *this;
    }
	*/

    bool CMqFile::IsInDequeuing(MB::U_UINT64 nMsgIndex) {
        unsigned int n, Count = m_qOut.GetSize() / sizeof (MB::U_UINT64);
        const MB::U_UINT64 *pMsgIndex = (const MB::U_UINT64 *)m_qOut.GetBuffer();
        for (n = 0; n < Count; ++n) {
            if (pMsgIndex[n] == nMsgIndex)
                return true;
        }
        return false;
    }

    bool CMqFile::RemoveMsgIndex(MB::U_UINT64 nMsgIndex) {
        unsigned int n, Count = m_qOut.GetSize() / sizeof (MB::U_UINT64);
        const MB::U_UINT64 *pMsgIndex = (const MB::U_UINT64 *)m_qOut.GetBuffer();
        for (n = 0; n < Count; ++n) {
            if (pMsgIndex[n] == nMsgIndex) {
                m_qOut.Pop((unsigned int) sizeof (MB::U_UINT64), n * sizeof (MB::U_UINT64));
                return true;
            }
        }
        return false;
    }

    unsigned int CMqFile::GetMessagesInDequeuing() {
        MB::CAutoLock al(m_cs);
        return m_qOut.GetSize() / sizeof (MB::U_UINT64);
    }

    const char* CMqFile::GetMQFileNmae() const {
        return m_fileName.c_str();
    }

    MB::U_UINT64 CMqFile::GetMQSize() {
        MB::CAutoLock al(m_cs);
        return (MB::U_UINT64)GetFileSize();
    }

    bool CMqFile::SetFilePointer(FILE *stream, MB::U_INT64 offset, int origin) {
#ifdef WIN32_64
        return (::_fseeki64(stream, offset, origin) == 0);
#else
        return (::fseeko64(stream, offset, origin) == 0);
#endif
    }

    void CMqFile::SetInitialEnqueuePosition() {
        size_t read;
        MB::U_UINT64 pos = 0;
        MB::U_UINT64 fileSize = (MB::U_UINT64)GetFileSize();
        SetFilePointer(m_hFile, 0, SEEK_SET);
        MessageDecriptionHeader mdh;
        while ((pos + sizeof (mdh)) <= fileSize) {
            read = ::fread(&mdh, sizeof (mdh), 1, m_hFile);
            assert(read == 1);
            pos += sizeof (mdh);
            if (mdh.Size > 0) {
                read = SetFilePointer(m_hFile, mdh.Size, SEEK_CUR);
                assert(read == 1);
                pos += mdh.Size;
            }
            if (mdh.MessageIndex)
                m_nInternalIndex = mdh.MessageIndex;
            else
                ++m_nInternalIndex;
        }
    }

    MB::U_UINT64 CMqFile::GetMessageCount() {
        bool ok;
        MB::U_INT64 fileSize;
        MessageDecriptionHeader mdh;
        size_t read;
        MB::U_UINT64 mc = 0;

        MB::CAutoLock al(m_cs);
        EnsureAvailable();
        MB::U_INT64 pos = m_CurrentReadPos;
        fileSize = GetFileSize();
        ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
        assert(ok);
        while (pos < fileSize) {
            read = ::fread(&mdh, sizeof (mdh), 1, m_hFile);
            assert(read == 1);
            if (mdh.MessageIndex != 0 && !IsInDequeuing(mdh.MessageIndex))
                ++mc;
            if (mdh.Size) {
                ok = SetFilePointer(m_hFile, mdh.Size, SEEK_CUR);
                assert(ok);
            }
            pos += (sizeof (mdh) + mdh.Size);
        }
        ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
        assert(ok);
        return (mc + m_qOut.GetSize() / sizeof (MB::U_UINT64));
    }

    void CMqFile::ConfirmDequeue(MB::U_UINT64 pos, bool fail) {
        bool ok;
        size_t read;
        MessageDecriptionHeader mdh;
        MB::U_INT64 absPos = (MB::U_INT64) pos;

        MB::CAutoLock al(m_cs);
        EnsureAvailable();
        ok = SetFilePointer(m_hFile, absPos, SEEK_SET);
        assert(ok);
        read = ::fread(&mdh, sizeof (mdh), 1, m_hFile);
        assert(read == 1);
        bool found = RemoveMsgIndex(mdh.MessageIndex);
        assert(found);
        if (fail) {
            if (pos < (MB::U_UINT64)m_CurrentReadPos)
                m_CurrentReadPos = (MB::U_INT64) pos;
        } else {
            mdh.MessageIndex = 0;
            ok = SetFilePointer(m_hFile, absPos, SEEK_SET);
            assert(ok);
            read = ::fwrite(&mdh, sizeof (mdh), 1, m_hFile);
            fflush(m_hFile);
            assert(read == 1);
        }
        ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
        assert(ok);
    }

    MB::U_UINT64 CMqFile::Dequeue(MB::CUQueue &q, MB::U_UINT64 &mqIndex) {
        MB::U_INT64 fileSize;
        MessageDecriptionHeader mdh;
        size_t read = 0;
        MB::U_UINT64 pos = INVALID_MSG_INDEX;
        mqIndex = 0;
        q.SetSize(0);

        MB::CAutoLock al(m_cs);
        EnsureAvailable();
        fileSize = GetFileSize();
        bool ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
        assert(ok);
        while (m_CurrentReadPos < fileSize) {
            read = ::fread(&mdh, sizeof (mdh), 1, m_hFile);
            if (!read)
                break;
            if (mdh.MessageIndex == 0 || IsInDequeuing(mdh.MessageIndex)) {
                if (mdh.Size)
                    SetFilePointer(m_hFile, mdh.Size, SEEK_CUR);
                m_CurrentReadPos += (sizeof (mdh) + mdh.Size);
            } else {
                mqIndex = mdh.MessageIndex;
                if (mdh.Size > q.GetMaxSize())
                    q.ReallocBuffer(mdh.Size);
                read = ::fread((unsigned char*) q.GetBuffer(), mdh.Size, 1, m_hFile);
                assert(read == 1);
                q.SetSize(mdh.Size);
                pos = (MB::U_UINT64)m_CurrentReadPos;
                m_CurrentReadPos += (sizeof (mdh) + mdh.Size);
                m_qOut.Push((const unsigned char*) &mqIndex, sizeof (mqIndex));
                break;
            }
        }
        if (pos == INVALID_MSG_INDEX && m_qOut.GetSize() == 0 && GetFileSize()) {
            m_CurrentReadPos = 0;
            CloseFile();
            m_hFile = ::fopen(m_fileName.c_str(), "w+b");
        }
        return pos;
    }

    void CMqFile::EnsureAvailable() {
        if (m_hFile == NULL)
            throw CMBExCode("Can not create a file to store message queue!", MB_CAN_NOT_CREATE_FILE);
    }

    bool CMqFile::IsAvailable() {
        MB::CAutoLock al(m_cs);
        return (m_hFile != NULL);
    }

    MB::U_UINT64 CMqFile::Enqueue(const unsigned char *buffer, unsigned int size) {
        size_t written;
        if (buffer == NULL)
            size = 0;
        MB::CAutoLock al(m_cs);
        EnsureAvailable();
        ++m_nInternalIndex;
        m_UQueue.SetSize(0);
        m_UQueue.Push((const unsigned char*) &m_nInternalIndex, sizeof (m_nInternalIndex));
        m_UQueue.Push((const unsigned char*) &size, sizeof (size));
        m_UQueue.Push(buffer, size);
        SetFilePointer(m_hFile, 0, SEEK_END);
        written = ::fwrite(m_UQueue.GetBuffer(), m_UQueue.GetSize(), 1, m_hFile);
        assert(written == 1);
        fflush(m_hFile);
        return m_nInternalIndex;
    }

    MB::U_UINT64 CMqFile::Enqueue(const MB::CUQueue &buffer) {
        return Enqueue(buffer.GetBuffer(), buffer.GetSize());
    }

    vector<MB::U_UINT64> CMqFile::Enqueue(const MB::CUQueue *pBuffer, unsigned int Count) {
        unsigned int n;
        unsigned int size;
        size_t written;
        vector<MB::U_UINT64> vIndex;
        if (pBuffer == NULL)
            Count = 0;
        if (Count == 0)
            return vIndex;
        MB::CAutoLock al(m_cs);
        EnsureAvailable();
        m_UQueue.SetSize(0);
        for (n = 0; n < Count; ++n) {
            ++m_nInternalIndex;
            m_UQueue.Push((const unsigned char*) &m_nInternalIndex, sizeof (m_nInternalIndex));
            size = pBuffer[n].GetSize();
            m_UQueue.Push((const unsigned char*) &size, sizeof (size));
            m_UQueue.Push(pBuffer[n].GetBuffer(), size);
            vIndex.push_back(m_nInternalIndex);
        }
        SetFilePointer(m_hFile, 0, SEEK_END);
        written = ::fwrite(m_UQueue.GetBuffer(), m_UQueue.GetSize(), 1, m_hFile);
        assert(written == 1);
        fflush(m_hFile);
        return vIndex;
    }


	CMqFileEx::CMqFileEx(const char *fileName, unsigned short port, const wchar_t *userId, const wchar_t *password, const char *machineId)
		: CMqFile(fileName, port)
	{
		std::wstring s(userId);

		std::transform(s.begin(), s.end(), s.begin(), ::tolower);

		MB::CScopeUQueue su;
		su->Push(s.c_str());
		su->Push(password);
		su->Push(machineId);
		unsigned char bytes[32] = {0};
		MB::CSHA1 sha1;
		sha1.Update(su->GetBuffer(), su->GetSize());
        sha1.Final();
        bool ok = sha1.GetHash(bytes);
		m_bf.reset(new MB::CBF(bytes, 20));
		memset(bytes, 0, sizeof(bytes));
		su->CleanTrack();
	}
}