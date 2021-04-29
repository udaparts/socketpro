#ifndef _U_TDS_TRANS_MANAGER_H_
#define _U_TDS_TRANS_MANAGER_H_

#include "reqbase.h"

namespace tds {

	class CTransManager : public CReqBase
	{
	public:
		CTransManager();

	protected:
		bool ParseStream();
		void Reset();
		void ParseTransChange(tagEnvchangeType type, TransChange& tc);

	protected:
		std::vector<TransChange> m_vTransChange;
		
	};

}

#endif