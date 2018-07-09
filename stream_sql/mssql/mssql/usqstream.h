#pragma once

using namespace Microsoft::SqlServer::Server;
using namespace System::Collections::Generic;

#include "sqlplugin.h"

ref class USqlStream
{
private:
	static CSqlPlugin *Plugin = nullptr;
	static SPA::CUCriticalSection *m_cs = new SPA::CUCriticalSection;
	static String ^ServerHost = nullptr;

public:
	static property const CSqlPlugin* Server {
		const CSqlPlugin* get() {
			SPA::CAutoLock al(*m_cs);
			return Plugin;
		}
	}

	static int LogError(SqlConnection ^conn, String ^errMsg);

	[SqlProcedure()]
	static void StopSPServer([Out] int %res);

	[SqlProcedure()]
	static void StartSPServer([Out] int %res);

private:
	static bool IsRunning();
	static String^ GetServerName(SqlConnection^ conn);
	static List<array<Object^>^>^ GetRows(SqlConnection^ conn, bool del, [Out] DataTable^ %dt);
	static Object^ GetData(SqlDataReader^ reader, int ordinal);
	static array<String^>^ GetUSqlServerKeys(SqlConnection^ conn);
	static List<array<Object^>^>^ GetUpdateRows(SqlConnection^ conn, [Out] DataTable^ %dt);
};
