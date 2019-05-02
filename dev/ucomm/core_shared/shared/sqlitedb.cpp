#include "sqlitedb.h"
#include <string>
#include <iostream>

namespace SQLite{

  const char *serverDbName = "uolmsg.db";
  const char *sqlRun= "create table AppRun(RunIndex INTEGER PRIMARY KEY AUTOINCREMENT, StartTime BIG INT UNIQUE NOT NULL)";
  const char *srFormat = "insert into AppRun (StartTime) values (%lld)";
  const char *sqlOffLine = "create table OffLineMsg(MsgIndex BIG INT, ClientMsgIndex BIG INT NULL, RunIndex INTEGER NOT NULL, Sender VARCHAR(510) NOT NULL, Receiver VARCHAR(510) NOT NULL, Msg Text NULL, PRIMARY KEY (MsgIndex, RunIndex), FOREIGN KEY(RunIndex) REFERENCES AppRun(RunIndex) ON DELETE CASCADE)";
  const char *sqlReply = "create table ReplyMsg(ReplyIndex BIG INT PRIMARY KEY, RunIndex INTEGER NOT NULL, MsgIndex BIG INT NOT NULL, ClientMsgIndex BIG INT NULL, FOREIGN KEY(MsgIndex) REFERENCES OffLineMsg(MsgIndex) ON DELETE CASCADE, FOREIGN KEY(RunIndex) REFERENCES AppRun(RunIndex) ON DELETE CASCADE)";
  const char *idx_sender_om = "Create index if not exists idx_sender_om on OffLineMsg (Sender)";
  const char *idx_receiver_om = "Create index if not exists idx_receiver_om on OffLineMsg (Receiver)";
  const char *idx_msgindex_rm = "Create index if not exists idx_msgindex_rm on ReplyMsg (MsgIndex)";
  const char *idx_msgindex_om = "Create index if not exists idx_msgindex_om on OffLineMsg (MsgIndex)";

CSqliteDb::CSqliteDb()
  : m_db(NULL),
  m_runIndex(0),
  m_startTime(0),
  m_msgIndex(0)
{

}

CSqliteDb::~CSqliteDb()
{
  close();
}

int CSqliteDb::OnOMReturn(void* param, int column_count, char** values, char** columns)
{
  OfflineMsg om;
  VOfflineMsg *pMsg = (VOfflineMsg*)param;

  om.MsgIndex = ::_atoi64(values[0]);
  if(values[1])
    om.ClientMsgIndex = ::_atoi64(values[1]);
  else
    om.ClientMsgIndex = 0;
  om.RunIndex = ::atoi(values[2]);
  om.Sender = values[3];
  om.Receiver = values[4];
  if(values[5])
    om.Msg = values[5];
  else
    om.Msg.clear();
  pMsg->push_back(om);
  return 0;
}

int CSqliteDb::getRunIndex() const
{
  return m_runIndex;
}

time_t CSqliteDb::getStartTime() const
{
  return m_startTime;
}

VOfflineMsg CSqliteDb::getOfflineMsgs(const char *receiver)
{
  char *errMsg = NULL;
  const char *format = "select * from offLineMsg where receiver like '%s'";
  char sql[1024];
  VOfflineMsg vMsg;
  if(receiver == NULL)
    receiver = "";
  ::sprintf(sql, format, receiver);
  int rec = sqlite3_exec(m_db, sql, &OnOMReturn, &vMsg, &errMsg);
  if(errMsg)
    sqlite3_free(errMsg);
  return vMsg;
}

int CSqliteDb::startRun()
{
	char sql[1024];
	m_startTime = time(NULL);
	::sprintf(sql, srFormat, m_startTime);
	int rec = sqlite3_exec(m_db, sql, NULL, this, NULL);
	if(rec == SQLITE_OK)
		m_runIndex = (int)sqlite3_last_insert_rowid(m_db);
	else
		m_startTime = 0;
	return  m_runIndex;
}

int CSqliteDb::beginTrans()
{
  return sqlite3_exec(m_db, "begin transaction", NULL, NULL, NULL);
}

int CSqliteDb::commit()
{
  return sqlite3_exec(m_db, "commit transaction", NULL, NULL, NULL);
}

int CSqliteDb::rollback()
{
  return sqlite3_exec(m_db, "rollback transaction", NULL, NULL, NULL);
}

int CSqliteDb::openDb(const char *dbName)
{
  if(isOpen())
    return SQLITE_OK;
  return sqlite3_open(dbName, &m_db);
}

int CSqliteDb::InsertOfflineMsg(const OfflineMsg &om)
{
	char str[128];
	MB::CScopeUQueue su;
	su->Push("Insert into OffLineMsg values(");
	::sprintf(str, "%lld,%lld,%d,'", m_msgIndex, om.ClientMsgIndex, m_runIndex);
	su->Push(str);
	su->Push(om.Sender.c_str(), (unsigned int)om.Sender.size());
	su->Push("','");
	su->Push(om.Receiver.c_str(), (unsigned int)om.Receiver.size());
	su->Push("','");
	su->Push(om.Msg.c_str(), (unsigned int)om.Msg.size());
	su->Push("')");
	++m_msgIndex;
	su->NullTerminated();
	const char *sql = (const char*)su->GetBuffer();
	return executeSql(sql);
}

int CSqliteDb::executeSql(const char *sql) const
{
   return sqlite3_exec(m_db, sql, NULL, NULL, NULL);
}

int CSqliteDb::initilizeDB()
{
  int rec = executeSql( SQLite::sqlRun);
  if(rec > SQLITE_ERROR )
    return rec;
  rec = executeSql( SQLite::sqlOffLine);
  if(rec > SQLITE_ERROR )
    return rec;
  rec = executeSql( SQLite::sqlReply);
  if(rec > SQLITE_ERROR )
    return rec;

  rec = executeSql( SQLite::idx_msgindex_om);
  rec = executeSql( SQLite::idx_msgindex_rm);
  rec = executeSql( SQLite::idx_receiver_om);
  rec = executeSql( SQLite::idx_sender_om);

  return SQLITE_OK;
}

sqlite3* CSqliteDb::getDb() const
{
  return m_db;
}

bool CSqliteDb::isOpen() const
{
  return (m_db != NULL);
}

void CSqliteDb::close()
{
  if(m_db != NULL)
  {
    sqlite3_close(m_db);
    m_db = NULL;
  }
  m_startTime = 0;
  m_runIndex = 0;
}

}