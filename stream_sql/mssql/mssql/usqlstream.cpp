#include "stdafx.h"
#include "usqstream.h"
#include "config.h"

using namespace System::IO;

bool USqlStream::IsRunning() {
	return CSqlPlugin::IsRunning();
}

String^ USqlStream::GetServerName(SqlConnection^ conn) {
	return SQLConfig::GetServerName(conn);
}

Object^ USqlStream::GetData(SqlDataReader^ reader, int ordinal) {
	if (reader->IsDBNull(ordinal))
		return nullptr;
	String^ native = reader->GetDataTypeName(ordinal);
	if (native->IndexOf(".") != -1)
		return reader->GetValue(ordinal)->ToString(); //customer defined data types, geometry, and geography
	else if (native == "xml")
		return reader->GetSqlXml(ordinal)->ToString();
	else if (native == "time")
		return reader->GetValue(ordinal)->ToString();
	else if (native == "datetimeoffset")
		return reader->GetDateTimeOffset(ordinal).ToString();
	return reader->GetValue(ordinal);
}

List<array<Object^>^>^ USqlStream::GetRows(SqlConnection^ conn, bool del, [Out] DataTable^ %dt) {
	dt = nullptr;
	SqlDataReader ^reader = nullptr;
	List<array<Object^>^> ^v = gcnew List<array<Object^>^>;
	try {
		SqlCommand cmd(del ? "SELECT * FROM DELETED" : "SELECT * FROM INSERTED", conn);
		reader = cmd.ExecuteReader(CommandBehavior::KeyInfo);
		dt = reader->GetSchemaTable();
		int count = reader->FieldCount;
		while (reader->Read()) {
			array<Object^> ^r = gcnew array<Object^>(count + 5);
			r[0] = (del ? (int)tagUpdateEvent::ueDelete : (int)tagUpdateEvent::ueInsert);
			for (int n = 0; n < count; ++n) {
				r[n + 5] = GetData(reader, n);
			}
			v->Add(r);
		}
	}
	catch (...) {
	}
	return v;
}

List<array<Object^>^>^ USqlStream::GetUpdateRows(SqlConnection^ conn, [Out] DataTable^ %dt) {
	dt = nullptr;
	SqlDataReader ^reader = nullptr;
	List<array<Object^>^> ^rows = gcnew List<array<Object^>^>;

	return rows;
}

array<String^>^ USqlStream::GetUSqlServerKeys(SqlConnection^ conn) {
	Exception ^ex = nullptr;
	if (!conn || conn->State != ConnectionState::Open)
		throw gcnew InvalidOperationException("An opened connection required");
	array<String^>^ v = nullptr;
	SqlDataReader ^dr = nullptr;
	String ^sqlCmd = "select SUSER_NAME(),DB_NAME()";
	try {
		SqlCommand cmd(sqlCmd, conn);
		dr = cmd.ExecuteReader();
		if (dr->Read()) {
			v = gcnew array<String^>(2);
			v[0] = dr->GetString(0);
			v[1] = dr->GetString(1);
		}
	}
	catch (Exception ^err) {
		ex = err;
	}
	if (dr)
		dr->Close();
	if (ex)
		throw ex;
	return v;
}

int USqlStream::LogError(SqlConnection ^conn, String ^errMsg) {
	if (!errMsg)
		errMsg = "";
	if (!conn || conn->State != ConnectionState::Open)
		return -1;
	try {
		String ^sql = String::Format("UPDATE sp_streaming_db.dbo.config set value='{0}' WHERE mykey='{1}'", errMsg, "usql_streaming_last_error");
		SqlCommand cmd(sql, conn);
		return cmd.ExecuteNonQuery();
	}
	catch (...) {}
	return -2;
}

void USqlStream::StopSPServer([Out] int %res) {
	res = 0;
	SqlConnection conn("context connection=true");
	try {
		conn.Open();
		SPA::CAutoLock al(*m_cs);
		if (Plugin) {
			if (IsRunning()) {
				Plugin->StopSocketProServer();
				res += 10;
			}
			delete Plugin;
			Plugin = nullptr;
		}
		res += 1;
	}
	catch (Exception ^err) {
		LogError(%conn, err->Message);
	}
	conn.Close();
}

void USqlStream::StartSPServer([Out] int %res) {
	res = 0;
	SqlConnection conn("context connection=true");
	try {
		SPA::CAutoLock al(*m_cs);
		if (!Plugin) {
			res += 100;
			if (!Directory::Exists(SQLConfig::WorkingDirectory))
				Directory::CreateDirectory(SQLConfig::WorkingDirectory);
			Directory::SetCurrentDirectory(SQLConfig::WorkingDirectory);
			Plugin = new CSqlPlugin(SQLConfig::Param);
		}
		if (!IsRunning()) {
			res += 10;
			if (SQLConfig::StoreOrPfx && SQLConfig::SubjectOrPassword && SQLConfig::StoreOrPfx->Length && SQLConfig::SubjectOrPassword->Length) {
				pin_ptr<const wchar_t> pfx = PtrToStringChars(SQLConfig::StoreOrPfx);
				pin_ptr<const wchar_t> pwd = PtrToStringChars(SQLConfig::SubjectOrPassword);
				std::string StoreOrPfx = SPA::Utilities::ToUTF8(pfx);
				std::string SubjectOrPassword = SPA::Utilities::ToUTF8(pwd);
				if (SQLConfig::StoreOrPfx->IndexOf(".pfx") == -1) {
					//load cert and private key from windows system cert store
					Plugin->UseSSL(StoreOrPfx.c_str()/*"my"*/, SubjectOrPassword.c_str(), "");
				}
				else {
					Plugin->UseSSL(StoreOrPfx.c_str(), "", SubjectOrPassword.c_str());
				}
			}
			if (Plugin->Run(SQLConfig::Port, 16, !SQLConfig::NoV6))
				res += 1;
		}
	}
	catch (Exception ^err) {
		LogError(%conn, err->Message);
	}
	conn.Close();
}
