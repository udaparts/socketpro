
#include "stdafx.h"
#include "masterpool.h"

extern CTableCache g_cache;

CMasterPool::CMasterPool()
{

}

void CMasterPool::OnSocketPoolEvent(tagSocketPoolEvent spe, const PHandler &asyncSQL) {
	switch (spe)
	{
	case speConnected:
		if (asyncSQL == GetAsyncHandlers()[0]) {
			asyncSQL->GetAttachedClientSocket()->GetPush().OnPublish = [](CClientSocket*cs, const CMessageSender& sender, const unsigned int* groups, unsigned int count, const SPA::UVariant& vtMsg) {

			};

			asyncSQL->Open(L"", [this](CAsyncSQLHandler &h, int res, const std::wstring &errMsg){
				this->m_cache.Empty();
			}, ENABLE_TABLE_UPDATE_MESSAGES);
			asyncSQL->Execute(L"", [this](CAsyncSQLHandler &h, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
				if (res == 0) {
					g_cache.Swap(this->m_cache);
				}
			}, [this](CAsyncSQLHandler &h, CDBVariantArray &vData){
				auto meta = h.GetColumnInfo();
				const CDBColumnInfo &info = meta.front();
				this->m_cache.AddRows(info.DBPath.c_str(), info.OriginalName.c_str(), vData);
			}, [this](CAsyncSQLHandler &h) {
				this->m_cache.AddEmptyRowset(h.GetColumnInfo());
			});
		}
		break;
	default:
		break;
	}
}