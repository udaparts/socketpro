#pragma once

#include "sqlite3.h"
#include <time.h>
#include <vector>
#include "base64.h"
#include "../../include/membuffer.h"

namespace SQLite{

 extern const char *serverDbName;

 class CSqliteDb;

struct ReplyMsg
{
	sqlite_int64 ReplyIndex;
	int			 RunIndex;
	sqlite_int64 MsgIndex;
	sqlite_int64 ClientMsgIndex;
};

class OfflineMsg
{
public:
  OfflineMsg()
    :  RunIndex(0), MsgIndex(0), ClientMsgIndex(0)
  {

  }

  OfflineMsg(OfflineMsg &&om)
    : RunIndex(om.RunIndex), MsgIndex(om.MsgIndex),
    ClientMsgIndex(om.ClientMsgIndex),
    Sender(std::move(om.Sender)),
    Receiver(std::move(om.Receiver)),
    Msg(std::move(om.Msg))
  {

  }

  OfflineMsg(const OfflineMsg &om)
    : RunIndex(om.RunIndex), MsgIndex(om.MsgIndex),
    ClientMsgIndex(om.ClientMsgIndex),
    Sender(om.Sender),
    Receiver(om.Receiver),
    Msg(om.Msg)
  {

  }

  OfflineMsg& operator=(OfflineMsg && om)
  {
	  RunIndex = om.RunIndex;
      MsgIndex = om.MsgIndex;
      ClientMsgIndex = om.ClientMsgIndex;
      Sender = std::move(om.Sender);
      Receiver = std::move(om.Receiver);
      Msg = std::move(om.Msg);
      return *this;
  }

  void SetMsg(const unsigned char *msg, unsigned int len)
  {
    Msg.clear();
    MB::CScopeUQueue su(true, MY_OPERATION_SYSTEM, (len + 1)*4/3);
    unsigned int res = Utilities::CBase64::encode(msg, len, (char*)su->GetBuffer());
    Msg = (const char*)su->GetBuffer();
  }

  MB::CScopeUQueue GetMsg()
  {
    unsigned int len = (unsigned int)Msg.size();
    MB::CScopeUQueue su(true, MY_OPERATION_SYSTEM, len*3/4 + 1);
    unsigned int res = Utilities::CBase64::decode(Msg.c_str(), len, (unsigned char*)su->GetBuffer());
    su->SetSize(res);
    return su;
  }

  int RunIndex;
  sqlite_int64 MsgIndex;
  sqlite_int64 ClientMsgIndex;
  std::string Sender;
  std::string Receiver;
 
private:
  std::string Msg;
  OfflineMsg& operator=(const OfflineMsg &om);
  friend class CSqliteDb;
};

typedef std::vector<OfflineMsg> VOfflineMsg;
typedef std::vector<ReplyMsg> VReplyMsg;

class CSqliteDb
{
	static const unsigned int BIT_SHIFT = 46;

public:
  CSqliteDb();
  virtual ~CSqliteDb();
  int beginTrans();
  int commit();
  int rollback();
  int openDb(const char *dbName = serverDbName);
  int executeSql(const char *sql) const;
  VOfflineMsg getOfflineMsgs(const char *receiver);
  int InsertOfflineMsg(const OfflineMsg &om);

  int initilizeDB();
  int startRun();
  int getRunIndex() const;
  time_t getStartTime() const;
  sqlite3* getDb() const;
  bool isOpen() const;
  void close();

private:
  static int OnOMReturn(void* param, int column_count, char** values, char** columns);

private:
  sqlite3		*m_db;
  int			m_runIndex;
  time_t		m_startTime;
  sqlite_int64	m_msgIndex;
};

};

