// SQLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;

#include "../shared/sqlite3.h"
#include "../shared/Base64.h"
#include "../shared/SqliteDb.h"

int main(int argc, char* argv[])
{
  char c;
  int rec;
  char *input = "This is a test from charlie! uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];";
  MB::CScopeUQueue su;

  SQLite::OfflineMsg om;
  om.Sender = "Kerui";
  om.Receiver = "cye";
  om.ClientMsgIndex = 1234568;
  
  SQLite::CSqliteDb sqlDb;
  rec = sqlDb.openDb();
  if(!sqlDb.isOpen())
     cout<<"Failed in starting sqlite database!"<<endl;
  else
    rec = sqlDb.initilizeDB();
  rec = sqlDb.startRun();

  su << sqlDb.getStartTime() << input << (int)123456;

  om.SetMsg(su->GetBuffer(), su->GetSize());
  rec = sqlDb.InsertOfflineMsg(om);

  SQLite::VOfflineMsg vMsg = sqlDb.getOfflineMsgs("Cye");
  cout<<"Input a char to end ......"<<endl;
  cin >> c;
	return 0;
}

