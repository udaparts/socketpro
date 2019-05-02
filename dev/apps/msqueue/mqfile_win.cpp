
#include "stdafx.h"
#include "mqfile_win.h"
#include <string>
#include <assert.h>

using namespace std;

namespace MQ_FILE
{
  CMqFile::CMqFile(const char *fileName, unsigned int nMQId, unsigned short port)
    : m_fileName(fileName),
    m_nMQId(nMQId),
    m_fileEnqueue(INVALID_HANDLE_VALUE),
    m_fileDequeue(INVALID_HANDLE_VALUE),
    m_qOut(*m_suOut),
    m_UQueue(*m_su),
	m_nInternalIndex(0)
  {
	m_CurrentReadPoistion.QuadPart = 0;
    char str[128 + 1] = {0};
    sprintf(str, "_%d_%d.umq", port, nMQId);
    m_fileName += str;

	SetFileHandlers();
	if(IsAvailable())
		SetInitialEnqueuePosition();
  }

  CMqFile::CMqFile(CMqFile &&mq)
    : m_CurrentReadPoistion(std::move(mq.m_CurrentReadPoistion)),
	m_fileName(std::move(mq.m_fileName)),
    m_nMQId(std::move(mq.m_nMQId)),
    m_fileEnqueue(std::move(mq.m_fileEnqueue)),
    m_fileDequeue(std::move(mq.m_fileDequeue)),
    m_qOut(std::move(mq.m_qOut)),
    m_UQueue(*m_su),
    m_nInternalIndex(std::move(mq.m_nInternalIndex))
  {

  }

  CMqFile::~CMqFile()
  {
    if(m_fileEnqueue != INVALID_HANDLE_VALUE)
      ::CloseHandle(m_fileEnqueue);

    if(m_fileDequeue != INVALID_HANDLE_VALUE)
      ::CloseHandle(m_fileDequeue);
  }

  void CMqFile::SetFileHandlers()
  {
	m_fileEnqueue = ::CreateFileA(m_fileName.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_fileEnqueue == INVALID_HANDLE_VALUE)
		return;

	m_fileDequeue = ::CreateFileA(m_fileName.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_fileDequeue == INVALID_HANDLE_VALUE)
	{  
		::CloseHandle(m_fileEnqueue);
		m_fileEnqueue = INVALID_HANDLE_VALUE;
	}
  }

  CMqFile& CMqFile::operator=(CMqFile &&mq)
  {
    MB::CAutoLock al(m_cs);
	std::swap(m_CurrentReadPoistion, mq.m_CurrentReadPoistion);
    std::swap(m_fileName, mq.m_fileName);
    std::swap(m_nMQId, mq.m_nMQId);
    std::swap(m_fileEnqueue, mq.m_fileEnqueue);
    std::swap(m_fileDequeue, mq.m_fileDequeue);
    std::swap(m_qOut, mq.m_qOut);
    std::swap(m_nInternalIndex, mq.m_nInternalIndex);
    return *this;
  }

   bool CMqFile::IsInDequeuing(MB::U_UINT64 nMsgIndex)
   {
     unsigned int n, Count = m_qOut.GetSize()/sizeof(MB::U_UINT64);
     const MB::U_UINT64 *pMsgIndex = (const MB::U_UINT64 *)m_qOut.GetBuffer();
     for(n=0; n<Count; ++n)
     {
       if(pMsgIndex[n] == nMsgIndex)
         return true;
     }
     return false;
   }

   bool CMqFile::RemoveMsgIndex(MB::U_UINT64 nMsgIndex)
   {
     unsigned int n, Count = m_qOut.GetSize()/sizeof(MB::U_UINT64);
     const MB::U_UINT64 *pMsgIndex = (const MB::U_UINT64 *)m_qOut.GetBuffer();
     for(n=0; n<Count; ++n)
     {
       if(pMsgIndex[n] == nMsgIndex)
       {
         m_qOut.Pop((unsigned int)sizeof(MB::U_UINT64), n*sizeof(MB::U_UINT64));
         return true;
       }
     }
     return false;
   }

  unsigned int CMqFile::GetMessagesInDequeuing()
  {
    MB::CAutoLock al(m_cs);
    return m_qOut.GetSize()/sizeof(MB::U_UINT64);
  }

  const char* CMqFile::GetMQFileNmae() const
  {
    return m_fileName.c_str();
  }

  unsigned int CMqFile::GetMQId() const
  {
    return m_nMQId;
  }

  MB::U_UINT64 CMqFile::GetMQSize()
  {
    LARGE_INTEGER NewPosition;
    MB::CAutoLock al(m_cs);
    BOOL ok = GetFileSizeEx(m_fileEnqueue, &NewPosition);
    assert(ok == TRUE);
    return (MB::U_UINT64)(NewPosition.QuadPart);
  }

 void CMqFile::SetInitialEnqueuePosition()
 {
   DWORD dwRead;
   LARGE_INTEGER NewPosition;
   MB::U_UINT64 pos = 0;
   BOOL ok = GetFileSizeEx(m_fileDequeue, &NewPosition);
   assert(ok == TRUE);
   MB::U_UINT64 fileSize = (MB::U_UINT64)(NewPosition.QuadPart);
   MessageDecriptionHeader mdh;
   while((pos + sizeof(mdh)) <= fileSize)
   {
     ok = ::ReadFile(m_fileEnqueue, &mdh, sizeof(mdh), &dwRead, NULL);
     assert(ok == TRUE);
     pos += sizeof(mdh);
     if(mdh.Size > 0)
     {
       dwRead = ::SetFilePointer(m_fileEnqueue, mdh.Size, NULL, FILE_CURRENT);
       assert(dwRead != INVALID_SET_FILE_POINTER);
       pos += mdh.Size;
     }
     if(mdh.MessageIndex)
       m_nInternalIndex = mdh.MessageIndex;
     else
       ++m_nInternalIndex;
   }
 }

 MB::U_UINT64 CMqFile::GetMessageCount()
 {
   BOOL ok;
   LARGE_INTEGER NewPosition;
   MessageDecriptionHeader mdh;
   DWORD dwRead = 0;
   MB::U_UINT64 mc = 0;
   EnsureAvailable();
   MB::CAutoLock al(m_cs);
   LARGE_INTEGER pos = m_CurrentReadPoistion; 
   ok = ::GetFileSizeEx(m_fileDequeue, &NewPosition);
   ok = ::SetFilePointerEx(m_fileDequeue, m_CurrentReadPoistion, NULL, FILE_BEGIN);
   while(pos.QuadPart < NewPosition.QuadPart)
   {
     ok = ::ReadFile(m_fileDequeue, &mdh, sizeof(mdh), &dwRead, NULL);
     assert(ok == TRUE);
     assert(dwRead == sizeof(mdh));
     if(mdh.MessageIndex != 0 && !IsInDequeuing(mdh.MessageIndex))
       ++mc;
     if(mdh.Size)
       ok = ::SetFilePointer(m_fileDequeue, mdh.Size, NULL, FILE_CURRENT);
     pos.QuadPart += (sizeof(mdh) + mdh.Size);
   }
   ok = ::SetFilePointerEx(m_fileDequeue, m_CurrentReadPoistion, NULL, FILE_BEGIN);
   return (mc + m_qOut.GetSize()/sizeof(MB::U_UINT64));
 }

 void CMqFile::ConfirmDequeue(MB::U_UINT64 pos, bool fail)
 {
   BOOL ok;
   DWORD dwRead;
   MessageDecriptionHeader mdh;
   LARGE_INTEGER absPos;
   absPos.QuadPart = (LONGLONG)pos;
   EnsureAvailable();
   MB::CAutoLock al(m_cs);
   ok = ::SetFilePointerEx(m_fileDequeue, absPos, NULL, FILE_BEGIN);
   assert(ok == TRUE);
   ok = ::ReadFile(m_fileDequeue, &mdh, sizeof(mdh), &dwRead, NULL);
   assert(ok == TRUE);
   bool found = RemoveMsgIndex(mdh.MessageIndex);
   assert(found);
   if(fail)
   {
     if((LONGLONG)pos < m_CurrentReadPoistion.QuadPart)
      m_CurrentReadPoistion.QuadPart = (LONGLONG)pos;
   }
   else
   {
      mdh.MessageIndex = 0;
      ok = ::SetFilePointerEx(m_fileDequeue, absPos, NULL, FILE_BEGIN);
      assert(ok == TRUE);
      ok = ::WriteFile(m_fileDequeue, &mdh, sizeof(mdh), &dwRead, NULL);
      assert(ok == TRUE);
   }
   ok = ::SetFilePointerEx(m_fileDequeue, m_CurrentReadPoistion, NULL, FILE_BEGIN);
   assert(ok == TRUE);
 }

  MB::U_UINT64 CMqFile::Dequeue(MB::CUQueue &q, MB::U_UINT64 &mqIndex)
  {
	  BOOL ok;
	  LARGE_INTEGER NewPosition;
    MessageDecriptionHeader mdh;
	  DWORD dwRead = 0;
    MB::U_UINT64 pos = INVALID_MSG_INDEX;
    mqIndex = 0;
	  q.SetSize(0);
	  EnsureAvailable();
	  MB::CAutoLock al(m_cs);
    ok = ::GetFileSizeEx(m_fileDequeue, &NewPosition);
    while(m_CurrentReadPoistion.QuadPart < NewPosition.QuadPart)
    {
      ok = ::ReadFile(m_fileDequeue, &mdh, sizeof(mdh), &dwRead, NULL);
      assert(ok == TRUE);
      if(mdh.MessageIndex == 0 || IsInDequeuing(mdh.MessageIndex))
      {
        if(mdh.Size)
          ::SetFilePointer(m_fileDequeue, mdh.Size, NULL, FILE_CURRENT);
        m_CurrentReadPoistion.QuadPart += (sizeof(mdh) + mdh.Size);
      }
      else
      {
         mqIndex = mdh.MessageIndex;
         if(mdh.Size > q.GetMaxSize())
           q.ReallocBuffer(mdh.Size);
         ok = ::ReadFile(m_fileDequeue, (unsigned char*)q.GetBuffer(), mdh.Size, &dwRead, NULL);
         assert(ok == TRUE);
         q.SetSize(mdh.Size);
         pos = (MB::U_UINT64)(m_CurrentReadPoistion.QuadPart);
         m_CurrentReadPoistion.QuadPart += (sizeof(mdh) + mdh.Size);
         m_qOut.Push((const unsigned char*)&mqIndex, sizeof(mqIndex));
         break;
      }
    }
    if(pos == INVALID_MSG_INDEX && m_qOut.GetSize() == 0 && GetFileSize() > 0)
    {
      m_CurrentReadPoistion.QuadPart = 0;
      ok = ::SetFilePointerEx(m_fileDequeue, m_CurrentReadPoistion, NULL, FILE_BEGIN);
      assert(ok == TRUE);
      ok = ::SetFilePointerEx(m_fileEnqueue, m_CurrentReadPoistion, NULL, FILE_BEGIN);
      assert(ok == TRUE);
      ok = SetEndOfFile(m_fileDequeue);
      assert(ok == TRUE);
      ok = SetEndOfFile(m_fileEnqueue);
      assert(ok == TRUE);
    }
	  return pos;
  }

  void CMqFile::EnsureAvailable()
  {
    if(!IsAvailable())
      throw CMBExCode("Can not create a file to store message queue!", MB_CAN_NOT_CREATE_FILE);
  }

   bool CMqFile::IsAvailable() const
   {
     return (m_fileEnqueue != INVALID_HANDLE_VALUE && m_fileDequeue != INVALID_HANDLE_VALUE);
   }

   MB::U_UINT64 CMqFile::GetFileSize()
   {
	   LARGE_INTEGER NewPosition;
	   BOOL ok = ::GetFileSizeEx(m_fileDequeue, &NewPosition);
	   return (MB::U_UINT64)NewPosition.QuadPart;
   }

   MB::U_UINT64 CMqFile::Enqueue(const unsigned char *buffer, unsigned int size)
   {
     DWORD dwWritten;
     if(buffer == NULL)
       size = 0;
     EnsureAvailable();
     MB::CAutoLock al(m_cs);
     ++m_nInternalIndex;
     m_UQueue.SetSize(0);
     m_UQueue.Push((const unsigned char*)&m_nInternalIndex, sizeof(m_nInternalIndex));
     m_UQueue.Push((const unsigned char*)&size, sizeof(size));
     m_UQueue.Push(buffer, size);
     BOOL ok = ::WriteFile(m_fileEnqueue, m_UQueue.GetBuffer(), m_UQueue.GetSize(), &dwWritten, NULL);
     assert(ok == TRUE);
     assert(dwWritten == m_UQueue.GetSize());
     return m_nInternalIndex;
   }

    MB::U_UINT64 CMqFile::Enqueue(const MB::CUQueue &buffer)
    {
      return Enqueue(buffer.GetBuffer(), buffer.GetSize());
    }

   vector<MB::U_UINT64> CMqFile::Enqueue(const MB::CUQueue *pBuffer, unsigned int Count)
   {
     unsigned int n;
     unsigned int size;
     DWORD dwWritten;
     vector<MB::U_UINT64> vIndex;
     if(pBuffer == NULL)
       Count = 0;
     if(Count == 0)
       return vIndex;
     EnsureAvailable();
     MB::CAutoLock al(m_cs);
     m_UQueue.SetSize(0);
     for(n=0; n<Count; ++n)
     {
        ++m_nInternalIndex;
        m_UQueue.Push((const unsigned char*)&m_nInternalIndex, sizeof(m_nInternalIndex));
        size = pBuffer[n].GetSize();
        m_UQueue.Push((const unsigned char*)&size, sizeof(size));
        m_UQueue.Push(pBuffer[n].GetBuffer(), size);
        vIndex.push_back(m_nInternalIndex);
     }
     BOOL ok = ::WriteFile(m_fileEnqueue, m_UQueue.GetBuffer(), m_UQueue.GetSize(), &dwWritten, NULL);
     assert(ok == TRUE);
     assert(dwWritten == m_UQueue.GetSize());
     return vIndex;
   }
}