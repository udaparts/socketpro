#ifndef _U_TDS_TRANS_MANAGER_H_
#define _U_TDS_TRANS_MANAGER_H_

#include "reqbase.h"

namespace tds {

	class CTransManager : public CReqBase
	{
	public:
		CTransManager();

	public:
		enum class tagRequestType : unsigned short
		{
			rtBeginTrans = 0x05,
			rtCommit = 0x07,
			rtRollback = 0x08
		};

		enum class tagIsolationLevel : unsigned short
		{
			ilCurrent = 0,
			ilReadUncommitted = 0x01,
			ilReadCommitted = 0x02,
			ilRepeatableRead = 0x03,
			ilSerializable = 0x04,
			ilSnapshot = 0x05
		};
		static_assert(sizeof(tagRequestType) == 2, "Wrong tagRequestType size");
		static_assert(sizeof(tagIsolationLevel) == 2, "Wrong tagIsolationLevel size");

	public:
		bool GetClientMessage(unsigned char packet_id, tagRequestType rt, tagIsolationLevel il, SPA::UINT64 trans_decriptor, SPA::CUQueue &buffer);
		UINT64 GetTransDescriptor();

	protected:
		bool ParseStream();
		void Reset();
		void ParseTransChange(tagEnvchangeType type, TransChange& tc);

	protected:
		std::vector<TransChange> m_vTransChange;
		
	};

}

#endif