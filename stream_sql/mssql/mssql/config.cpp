
#include "stdafx.h"
#include "config.h"

String^ SQLConfig::GetServerName(SqlConnection ^conn) {
	if (conn == nullptr || conn->State != ConnectionState::Open)
		throw gcnew InvalidOperationException("An opened connection required");
	String ^serverName = Environment::MachineName;
	SqlDataReader ^dr = nullptr;
	try {
		String ^sqlCmd = "SELECT @@servername";
		SqlCommand cmd(sqlCmd, conn);
		dr = cmd.ExecuteReader();
		if (dr->Read()) {
			if (dr->IsDBNull(0)) {
				dr->Close();
				sqlCmd = "SELECT @@SERVICENAME";
				cmd.CommandText = sqlCmd;
				dr = cmd.ExecuteReader();
				if (dr->Read())
					serverName += ("\\" + dr->GetString(0));
			}
			else
				serverName = dr->GetString(0);
		}
	}
	finally
	{
		if (dr)
			dr->Close();
	}
	return serverName;
}

void SQLConfig::SetConfig(SqlDataReader ^reader) {
	while (reader->Read()) {
		String ^key = reader->GetString(0);
		String ^value = reader->GetString(1);
		key = key->ToLower();
		if (key == "disable_ipv6") {
			try {
				m_bNoV6 = ((int::Parse(value) == 0) ? false : true);
			}
			catch (...) {
				m_bNoV6 = false;
			}
		}
		else if (key == "read_only") {
			try {
				m_readOnly = ((int::Parse(value) == 0) ? false : true);
			}
			catch (...) {
				m_readOnly = true;
			}
		}
		else if (key == "main_threads") {
			try {
				m_Param = int::Parse(value);
			}
			catch (...) {
				m_Param = 1;
			}
		}
		else if (key == "enable_http_websocket") {
			try {
				m_bWebSocket = ((int::Parse(value) == 0) ? false : true);
			}
			catch (...) {
				m_bWebSocket = false;
			}
		}
		else if (key == "port") {
			try {
				m_nPort = unsigned int::Parse(value);
			}
			catch (...) {
				m_nPort = 20903;
			}
		}
		else if (key == "services") {
			m_services = value;
		}
		else if (key == "store_or_pfx") {
			m_store_or_pfx = value;
		}
		else if (key == "subject_or_password") {
			m_subject_or_password = value;
		}
		else if (key == "working_directory") {
			m_WorkingDirectory = value;
		}
		else if (key == "odbcdriver") {
			m_odbc = value;
		}
		else {
			//ignored
		}
	}
}
