
#pragma once
using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

ref class SQLConfig {
private:
	static String ^m_server = Environment::MachineName;
	static bool m_bNoV6 = false;
	static bool m_readOnly = true;
	static bool m_bWebSocket = false;
	static unsigned int m_nPort = 20903;
	static String ^m_WorkingDirectory = "C:\\ProgramData\\MSSQL\\";
	static String ^m_services = "";
	static String ^m_store_or_pfx = "";
	static String ^m_subject_or_password = "";
	static int m_Param = 1;
	static String ^m_odbc = "{SQL Server}";
	static SQLConfig() {
		SqlConnection conn("context connection=true");
		SqlDataReader ^reader = nullptr;
		try {
			conn.Open();
			SqlCommand cmd("SELECT * from sp_streaming_db.dbo.config", %conn);
			reader = cmd.ExecuteReader();
			SetConfig(reader);
			reader->Close();
			reader = nullptr;
			m_server = GetServerName(%conn);
		}
		catch (...) {
			if (reader) {
				reader->Close();
			}
		}
		conn.Close();
	}

private:
	static void SetConfig(SqlDataReader ^reader);
	
public:
	static String^ GetServerName(SqlConnection ^conn);

	static property String^ Server
	{
		String^ get()
		{
			return m_server;
		}
	}

	static property String^ WorkingDirectory
	{
		String^ get()
		{
			return m_WorkingDirectory;
		}
	}

	static property String^ Services
	{
		String^ get()
		{
			return m_services;
		}
	}

	static property String^ StoreOrPfx
	{
		String^ get()
		{
			return m_store_or_pfx;
		}
	}

	static property String^ SubjectOrPassword
	{
		String^ get()
		{
			return m_subject_or_password;
		}
	}

	static property String^ ODBCDriver
	{
		String^ get()
		{
			return m_odbc;
		}
	}

	static property bool NoV6
	{
		bool get()
		{
			return m_bNoV6;
		}
	}

	static property bool ReadOnly
	{
		bool get()
		{
			return m_readOnly;
		}
	}

	static property bool HttpWebSocket
	{
		bool get()
		{
			return m_bWebSocket;
		}
	}

	static property unsigned int Port
	{
		unsigned int get()
		{
			return m_nPort;
		}
	}

	static property int Param
	{
		int get()
		{
			return m_Param;
		}
	}
};
