
#include "stdafx.h"
#include "mqfile.h"
#include <string>
#include <assert.h>

using namespace std;

namespace MQ_FILE
{
  CMqFile::CMqFile(const char *fileName, unsigned int nMQId, unsigned short port)
    : m_CurrentReadPos(0),
    m_fileName(fileName),
    m_nMQId(nMQId),
    m_hFile(NULL),
    m_qOut(*m_suOut),
    m_UQueue(*m_su),
	  m_nInternalIndex(0)
  {
    char str[128 + 1] = {0};
    sprintf(str, "_%d_%d.umq", port, nMQId);
    m_fileName += str;

    SetFileHandlers();
    if(IsAvailable())
      SetInitialEnqueuePosition();
  }

  CMqFile::CMqFile(CMqFile &&mq)
    : m_CurrentReadPos(std::move(mq.m_CurrentReadPos)),
	  m_fileName(std::move(mq.m_fileName)),
    m_nMQId(std::move(mq.m_nMQId)),
    m_hFile(std::move(mq.m_hFile)),
    m_qOut(std::move(mq.m_qOut)),
    m_UQueue(*m_su),
    m_nInternalIndex(std::move(mq.m_nInternalIndex))
  {

  }

  CMqFile::~CMqFile()
  {
    CloseFile();
  }

  void CMqFile::CloseFile()
  {
    if(m_hFile != NULL)
    {
      ::fclose(m_hFile);
      m_hFile = NULL;
    }
  }

  void CMqFile::SetFileHandlers()
  {
    m_hFile = ::fopen(m_fileName.c_str(), "r+b");
    if(m_hFile == NULL)
    {  
      m_hFile = ::fopen(m_fileName.c_str(), "w+b");
    }
  }

  fpos_t CMqFile::GetFileSize()
  {
    if(m_hFile != NULL)
    {
      ::fflush(m_hFile);
#ifdef WIN32_64
      ::_fseeki64(m_hFile, 0, SEEK_END);
      return _ftelli64(m_hFile);
#else
      ::fseeko64(m_fileEnqueue, 0, SEEK_END);
      return ftello64(m_fileEnqueue);
#endif
    }
    return 0;
  }

  CMqFile& CMqFile::operator=(CMqFile &&mq)
  {
    MB::CAutoLock al(m_cs);
	  std::swap(m_CurrentReadPos, mq.m_CurrentReadPos);
    std::swap(m_fileName, mq.m_fileName);
    std::swap(m_nMQId, mq.m_nMQId);
    std::swap(m_hFile, mq.m_hFile);
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
    MB::CAutoLock al(m_cs);
    return (MB::U_UINT64)GetFileSize();
  }

  bool CMqFile::SetFilePointer(FILE *stream, fpos_t offset, int origin)
  {
#ifdef WIN32_64
    return (::_fseeki64(stream, offset, origin) == 0);
#else
    return (::fseeko64(stream, offset, origin) == 0);
#endif
  }

 void CMqFile::SetInitialEnqueuePosition()
 {
   size_t read;
   MB::U_UINT64 pos = 0;
   MB::U_UINT64 fileSize = (MB::U_UINT64)GetFileSize();
   SetFilePointer(m_hFile, 0, SEEK_SET);
   MessageDecriptionHeader mdh;
   while((pos + sizeof(mdh)) <= fileSize)
   {
     read = ::fread(&mdh, sizeof(mdh), 1, m_hFile);
     assert(read == 1);
     pos += sizeof(mdh);
     if(mdh.Size > 0)
     {
       read = SetFilePointer(m_hFile, mdh.Size, SEEK_CUR);
       assert(read == 1);
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
   bool ok;
   fpos_t fileSize;
   MessageDecriptionHeader mdh;
   size_t read;
   MB::U_UINT64 mc = 0;
   EnsureAvailable();
   MB::CAutoLock al(m_cs);
   fpos_t pos = m_CurrentReadPos; 
   fileSize = GetFileSize();
   ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
   assert(ok);
   while(pos < fileSize)
   {
     read = ::fread(&mdh, sizeof(mdh), 1, m_hFile);
     assert(read == 1);
     if(mdh.MessageIndex != 0 && !IsInDequeuing(mdh.MessageIndex))
       ++mc;
     if(mdh.Size)
     {
       ok = SetFilePointer(m_hFile, mdh.Size, SEEK_CUR);
       assert(ok);
     }
     pos += (sizeof(mdh) + mdh.Size);
   }
   ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
   assert(ok);
   return (mc + m_qOut.GetSize()/sizeof(MB::U_UINT64));
 }

 void CMqFile::ConfirmDequeue(MB::U_UINT64 pos, bool fail)
 {
   bool ok;
   size_t read;
   MessageDecriptionHeader mdh;
   fpos_t absPos = (fpos_t)pos;
   EnsureAvailable();
   MB::CAutoLock al(m_cs);
   ok = SetFilePointer(m_hFile, absPos, SEEK_SET);
   assert(ok);
   read = ::fread(&mdh, sizeof(mdh), 1, m_hFile);
   assert(read == 1);
   bool found = RemoveMsgIndex(mdh.MessageIndex);
   assert(found);
   if(fail)
   {
     if(pos < (MB::U_UINT64)m_CurrentReadPos)
       m_CurrentReadPos = (fpos_t)pos;
   }
   else
   {
     mdh.MessageIndex = 0;
     ok = SetFilePointer(m_hFile, absPos, SEEK_SET);
     assert(ok);
     read = ::fwrite(&mdh, sizeof(mdh), 1, m_hFile);
     fflush(m_hFile);
     assert(read == 1);
   }
   ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
   assert(ok);
 }

  MB::U_UINT64 CMqFile::Dequeue(MB::CUQueue &q, MB::U_UINT64 &mqIndex)
  {
	  fpos_t fileSize;
    MessageDecriptionHeader mdh;
	  size_t read = 0;
    MB::U_UINT64 pos = INVALID_MSG_INDEX;
    mqIndex = 0;
	  q.SetSize(0);
	  EnsureAvailable();
	  MB::CAutoLock al(m_cs);
    fileSize = GetFileSize();
    bool ok = SetFilePointer(m_hFile, m_CurrentReadPos, SEEK_SET);
    assert(ok);
    while(m_CurrentReadPos < fileSize)
    {
      read = ::fread(&mdh, sizeof(mdh), 1, m_hFile);
      if(!read)
        break;
      if(mdh.MessageIndex == 0 || IsInDequeuing(mdh.MessageIndex))
      {
        if(mdh.Size)
          SetFilePointer(m_hFile, mdh.Size, SEEK_CUR);
        m_CurrentReadPos += (sizeof(mdh) + mdh.Size);
      }
      else
      {
        mqIndex = mdh.MessageIndex;
        if(mdh.Size > q.GetMaxSize())
        q.ReallocBuffer(mdh.Size);
        read = ::fread((unsigned char*)q.GetBuffer(), mdh.Size, 1, m_hFile);
        assert(read == 1);
        q.SetSize(mdh.Size);
        pos = (MB::U_UINT64)m_CurrentReadPos;
        m_CurrentReadPos += (sizeof(mdh) + mdh.Size);
        m_qOut.Push((const unsigned char*)&mqIndex, sizeof(mqIndex));
        break;
      }
    }
    if(pos == INVALID_MSG_INDEX && m_qOut.GetSize() == 0 && GetFileSize())
    {
      m_CurrentReadPos = 0;
      CloseFile();
      m_hFile = ::fopen(m_fileName.c_str(), "w+b");
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
     return (m_hFile != NULL);
   }

   MB::U_UINT64 CMqFile::Enqueue(const unsigned char *buffer, unsigned int size)
   {
     size_t written;
     if(buffer == NULL)
       size = 0;
     EnsureAvailable();
     MB::CAutoLock al(m_cs);
     ++m_nInternalIndex;
     m_UQueue.SetSize(0);
     m_UQueue.Push((const unsigned char*)&m_nInternalIndex, sizeof(m_nInternalIndex));
     m_UQueue.Push((const unsigned char*)&size, sizeof(size));
     m_UQueue.Push(buffer, size);
     SetFilePointer(m_hFile, 0, SEEK_END);
     written = ::fwrite(m_UQueue.GetBuffer(), m_UQueue.GetSize(), 1, m_hFile);
     assert(written == 1);
     fflush(m_hFile);
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
     size_t written;
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
     SetFilePointer(m_hFile, 0, SEEK_END);
     written = ::fwrite(m_UQueue.GetBuffer(), m_UQueue.GetSize(), 1, m_hFile);
     assert(written == 1);
     fflush(m_hFile);
     return vIndex;
   }
}