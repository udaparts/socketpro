#ifndef _UQUEUECP_H_
#define _UQUEUECP_H_

template <class T>
class CProxy_IURequestEvent : public IConnectionPointImpl<T, &DIID__IURequestEvent, CComDynamicUnkArray>
{
	//Warning this class may be recreated by the wizard.
public:
	HRESULT Fire_OnRequestProcessed(LONG hSocket, SHORT nRequestID, LONG lLen, LONG lLenInBuffer, SHORT sFlag)
	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
//		CComVariant* pvars = new CComVariant[5];
		CComVariant pvars[5];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)
			{
				VariantClear(&varResult);
				pvars[4] = hSocket;
				pvars[3] = nRequestID;
				pvars[2] = lLen;
				pvars[1] = lLenInBuffer;
				pvars[0] = sFlag;
				DISPPARAMS disp = { pvars, NULL, 5, 0 };
				pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
//		delete[] pvars;
		return varResult.scode;
	
	}
};
#endif