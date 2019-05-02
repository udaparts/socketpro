
#pragma once

#include <vector>
#include <string>
#include "../../include/membuffer.h"

namespace MQ_FILE
{
  using namespace std;

  class CMqFile
  {
  private:
    #pragma pack(push,1)
    struct MessageDecriptionHeader
    {
      MB::U_UINT64  MessageIndex;
      unsigned int  Size;
    };
    #pragma pack(pop)

  public:
    CMqFile(const char *fileName, unsigned int nMQId, unsigned short port);
    CMqFile(CMqFile &&mq);

    virtual ~CMqFile();

  public:
    //check if message queue file is available
    bool IsAvailable() const;

    const char *GetMQFileNmae() const;
    unsigned int GetMQId() const;

    //compute the count of messages during dequeuing
    unsigned int GetMessagesInDequeuing();

    //return a non-zero unique message index after enqueuing a message
    MB::U_UINT64 Enqueue(const unsigned char *buffer, unsigned int size);
	  
    //return a position and non-zero unique message index
    MB::U_UINT64 Dequeue(MB::CUQueue &q, MB::U_UINT64 &mqIndex);
	  
    //return MQ file size in byte
    MB::U_UINT64 GetMQSize();

    //confirm the message is either dequeued successfully or failed.
    void ConfirmDequeue(MB::U_UINT64 pos, bool fail = false);
    
    //compute the number of messages remaining in queue. This call is slow, and don't use the method if required only.
    MB::U_UINT64 GetMessageCount();

    CMqFile& operator=(CMqFile &&mq);

    vector<MB::U_UINT64> Enqueue(const MB::CUQueue *pBuffer, unsigned int Count);
    MB::U_UINT64 Enqueue(const MB::CUQueue &buffer);

    template<unsigned int InitSize, unsigned int BlockSize, typename mb>
    MB::U_UINT64 Enqueue(const MB::CScopeUQueueEx<InitSize, BlockSize, mb> &buffer)
    {
      return Enqueue(buffer->GetBuffer(), buffer->GetSize());
    }

    static const MB::U_UINT64 INVALID_MSG_INDEX = (~0);

  private:
    CMqFile(const CMqFile &file);
    CMqFile& operator=(const CMqFile &file);
	void SetInitialEnqueuePosition();
    bool IsInDequeuing(MB::U_UINT64 nMsgIndex);
    bool RemoveMsgIndex(MB::U_UINT64 nMsgIndex);
    void EnsureAvailable();
	void SetFileHandlers();
	MB::U_UINT64 GetFileSize();

  private:
    MB::CScopeUQueue m_suOut;
    MB::CScopeUQueue m_su;
	LARGE_INTEGER m_CurrentReadPoistion;
    string       m_fileName;
    unsigned int  m_nMQId;
    HANDLE        m_fileEnqueue;
    HANDLE        m_fileDequeue;
    MB::CUQueue   &m_qOut;
    MB::CUQueue   &m_UQueue;
	MB::U_UINT64  m_nInternalIndex;
	MB::CUCriticalSection	m_cs;
  };
  
};