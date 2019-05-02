// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com


#ifndef _UDAPART_U_SOCKET_QUEUE_H___
#define	_UDAPART_U_SOCKET_QUEUE_H___

#include <atlbase.h>
#define	DEFAULT_UQUEUE_BLOCK_SIZE		1024
#define DEFAULT_INITIAL_QUEUE_SIZE		128
#define	UQUEUE_END_POSTION				(0xFFFFFFFF)
#define	UQUEUE_NULL_LENGTH				(0xFFFFFFFF)
#define UQUEUE_SIZE_NULL				(0xFFFFFFFF)
#include <exception>
#include <string>

#if _MSC_VER >= 1600
	#define UNULL_PTR	nullptr
#else
	#define UNULL_PTR	0
#endif


namespace SocketProAdapter
{
	#define SOCKETPRO_COMPILE_CHECK(expr) {char strTemp[(expr) ? 1 : 0];}

	class CUQueue;

	/*class IUSerializer
	{
	public:
		virtual unsigned long LoadFrom(CUQueue &UQueue) = 0;
		virtual void SaveTo(CUQueue &UQueue) = 0;
	};*/

	#pragma warning(disable: 4267) // warning C4267: '=' : conversion from 'size_t' to 'ULONG', possible loss of data
	class CUQueue
	{
	public:
		CUQueue(unsigned long ulMaxLen = DEFAULT_INITIAL_QUEUE_SIZE, unsigned long ulBlockSize = DEFAULT_UQUEUE_BLOCK_SIZE)
			: m_ulMaxBuffer(ulMaxLen)
			, m_ulBlockSize(ulBlockSize ? ulBlockSize : DEFAULT_UQUEUE_BLOCK_SIZE)
			, m_ulLen(0)
			, m_ulHeadPos(0)
			, m_bScope(false)
		{
			if(m_ulMaxBuffer)
				m_pBuffer = (unsigned char*)::malloc(m_ulMaxBuffer);
			else
				m_pBuffer = UNULL_PTR;
		}
		
		virtual ~CUQueue()
		{
			Empty();
		}
		
		//no copy constructor!
		CUQueue(const CUQueue &UQueue);
	public:
		//no assignment operator! 
		CUQueue& operator=(const CUQueue &UQueue);

	public:
		inline const unsigned char* const GetBuffer(unsigned long ulHead = 0) const
		{
			if(ulHead >= m_ulLen)
				ulHead = m_ulLen;
			return (m_pBuffer + ulHead + m_ulHeadPos);
		}

		inline unsigned long GetSize() const
		{
			return m_ulLen;
		}

		inline void SetSize(unsigned long ulSize, bool bAdjustHeadPos = true)
		{
			if(bAdjustHeadPos && m_ulHeadPos)
			{
				m_ulLen = ulSize;
				if(m_ulLen)
					::memmove(m_pBuffer, m_pBuffer+m_ulHeadPos, m_ulLen);
				m_ulHeadPos = 0;
			}
			else
			{
				m_ulLen = ulSize;
				if(ulSize == 0)
					m_ulHeadPos = 0;
			}
			ATLASSERT(m_ulMaxBuffer >= (ulSize + m_ulHeadPos));
		}

		inline unsigned long GetMaxSize() const
		{
			return m_ulMaxBuffer;
		}

		inline unsigned long GetBlockSize() const
		{
			return m_ulBlockSize;
		}

		inline void SetBlockSize(unsigned long ulBlockSize = DEFAULT_UQUEUE_BLOCK_SIZE)
		{
			if(ulBlockSize == 0)
				ulBlockSize = DEFAULT_UQUEUE_BLOCK_SIZE;
			m_ulBlockSize = ulBlockSize;
		}

		inline void Empty()
		{
			if(m_pBuffer)
			{
				::free(m_pBuffer);
				m_pBuffer = UNULL_PTR;
				m_ulMaxBuffer = 0;
				m_ulLen = 0;
				m_ulHeadPos = 0;
			}
		}

		inline void Push(const unsigned char* pData, unsigned long ulBufferLen)
		{
			Insert(pData, ulBufferLen, UQUEUE_END_POSTION);
		}

		inline unsigned long GetHeadPosition() const
		{
			return m_ulHeadPos;
		}

		inline void SetHeadPosition()
		{
			if(m_ulHeadPos)
			{
				if(m_ulLen)
					::memmove(m_pBuffer, m_pBuffer+m_ulHeadPos, m_ulLen);
				m_ulHeadPos = 0;
			}
		}

		inline void CleanTrack()
		{
			ATLASSERT(m_ulMaxBuffer >= (m_ulHeadPos + m_ulLen));
			if(m_pBuffer != UNULL_PTR && m_ulMaxBuffer > 0)
			{
				if(m_ulHeadPos > 0)
				{
					::memset(m_pBuffer, 0, m_ulHeadPos);
				}
				ULONG ulPos = m_ulHeadPos + m_ulLen;
				ULONG ulLen = m_ulMaxBuffer - ulPos;
				if(ulLen > 0)
				{
					::memset(m_pBuffer + ulPos, 0, ulLen);
				}
			}
		}

		inline void ReallocBuffer(unsigned long ulSize, bool bAdjustHeadPos = true)
		{
			if(bAdjustHeadPos && m_ulHeadPos)
			{
				if(m_ulLen)
					::memmove(m_pBuffer, m_pBuffer+m_ulHeadPos, m_ulLen);
				m_ulHeadPos = 0;
			}
			if(ulSize == 0)
				ulSize = m_ulBlockSize;
			if(m_ulLen > ulSize)
				m_ulLen = ulSize;
			m_ulMaxBuffer = (ulSize + m_ulHeadPos);
			m_pBuffer = (unsigned char*)::realloc(m_pBuffer, m_ulMaxBuffer);
			if(m_pBuffer == UNULL_PTR)
			{
				m_ulLen = 0;
				m_ulMaxBuffer = 0;
				m_ulHeadPos = 0;
			}
		}

		inline void Insert(const unsigned char* pData, unsigned long ulBufferLen, unsigned long ulPos=0)
		{
			if(!pData || ulBufferLen==0)
				return;
			if(ulPos > m_ulLen)
				ulPos = m_ulLen;
			if((m_ulLen+ulBufferLen+m_ulHeadPos) > m_ulMaxBuffer)
			{
				m_ulMaxBuffer = (m_ulLen + ulBufferLen + m_ulBlockSize + m_ulHeadPos);
				m_pBuffer = (unsigned char*)::realloc(m_pBuffer, m_ulMaxBuffer);
			}
			if(m_ulLen-ulPos)
			{
				::memmove(m_pBuffer+ulPos+ulBufferLen+m_ulHeadPos, m_pBuffer+ulPos+m_ulHeadPos, m_ulLen-ulPos);
			}
			m_ulLen += ulBufferLen;
			::memmove(m_pBuffer+ulPos+m_ulHeadPos, pData, ulBufferLen);
		}
		
		/*
			Serializing an ASCII string into memory queue without storing a length ahead.
		*/
		inline void Insert(const char* str, unsigned long ulLen, unsigned long ulPos=0)
		{
			if(str)
			{
				if(!ulLen)
					ulLen = strlen(str);
				Insert((const unsigned char*)str, ulLen, ulPos);
			}
		}

		inline void Push(const char* str, unsigned long ulLen = 0)
		{
			Insert(str, ulLen, UQUEUE_END_POSTION);
		}
		
		/*
			Serializing a UNICODE string into memory queue without storing a length ahead.
		
			There is a potential pitfall for inserting a unsigned short here.
			
			For inserting a unsigned short, call it like the following way:
			
			Insert((const BYTE*)&usData, sizeof(usData), ulPos);
		*/
		inline void Insert(const wchar_t* str, unsigned long ulLen = 0, unsigned long ulPos = 0)
		{
			if(str)
			{
				if(!ulLen)
					ulLen = ::wcslen(str);
				ulLen *= sizeof(wchar_t);
				Insert((const unsigned char*)str, ulLen, ulPos);
			}
		}

		/*
			There is a potential pitfall for pushing a unsigned short here.

			For pushing a unsigned short, call it like the following way:

			Push((const BYTE*)&usData, sizeof(usData));
		*/
		inline void Push(const wchar_t* str, unsigned long ulLen = 0) 
		{
			Insert(str, ulLen, UQUEUE_END_POSTION);
		}

		void Insert(const CComVariant *pvtData, unsigned long ulPos = 0)
		{
			if(pvtData == UNULL_PTR)
				return;
			InsertVT(*pvtData, ulPos);
		}
		
		void Insert(const VARIANT *pvtData, unsigned long ulPos = 0)
		{
			if(pvtData == UNULL_PTR)
				return;
			InsertVT(*pvtData, ulPos);
		}
		
		void InsertVT(const VARIANT &vtData, unsigned long ulPos = 0)
		{
			if(ulPos > GetSize())
				ulPos = GetSize();
			Insert((const unsigned char*)&(vtData.vt), sizeof(vtData.vt), ulPos);
			ulPos += sizeof(vtData.vt);
			switch(vtData.vt)
			{
			case VT_NULL:
			case VT_EMPTY:
				break;
			case VT_I1:
				Insert((const unsigned char*)&(vtData.cVal), sizeof(vtData.cVal), ulPos);
				break;
			case VT_UI1:
				Insert((const unsigned char*)&(vtData.bVal), sizeof(vtData.bVal), ulPos);
				break;
			case VT_I2:
				Insert((const unsigned char*)&(vtData.iVal), sizeof(vtData.iVal), ulPos);
				break;
			case VT_UI2:
				Insert((const unsigned char*)&(vtData.uiVal), sizeof(vtData.uiVal), ulPos);
				break;
			case VT_I4:
				Insert((const unsigned char*)&(vtData.lVal), sizeof(vtData.lVal), ulPos);
				break;
			case VT_UI4:
				Insert((const unsigned char*)&(vtData.ulVal), sizeof(vtData.ulVal), ulPos);
				break;
			case VT_R4:
				Insert((const unsigned char*)&(vtData.fltVal), sizeof(vtData.fltVal), ulPos);
				break;
			case VT_R8:
				Insert((const unsigned char*)&(vtData.dblVal), sizeof(vtData.dblVal), ulPos);
				break;
			case VT_CY:
				Insert((const unsigned char*)&(vtData.cyVal), sizeof(vtData.cyVal), ulPos);
				break;
			case VT_DATE:
				Insert((const unsigned char*)&(vtData.date), sizeof(vtData.date), ulPos);
				break;
			case VT_BOOL:
				Insert((const unsigned char*)&(vtData.boolVal), sizeof(vtData.boolVal), ulPos);
				break;
			case VT_BSTR:
				{
					if(vtData.bstrVal == UNULL_PTR)
					{
						//0xFFFFFFFF == UNULL_PTR
						//0 == L""
						unsigned long ulLen = UQUEUE_NULL_LENGTH; 
						Insert((const unsigned char*)&ulLen, sizeof(ulLen), ulPos);
					}
					else
					{
						//0xFFFFFFFF == UNULL_PTR
						//0 == L""
						unsigned long ulLen = ::SysStringLen(vtData.bstrVal)*sizeof(wchar_t);
						Insert((const unsigned char*)&ulLen, sizeof(ulLen), ulPos);
						ulPos += sizeof(ulLen);
						if(ulLen)
						{
							Insert((const unsigned char*)vtData.bstrVal, ulLen, ulPos);
						}
					}
				}
				break;
			case VT_DECIMAL:
				Insert((const unsigned char*)&(vtData.decVal), sizeof(vtData.decVal), ulPos);
				break;
			case VT_INT:
				Insert((const unsigned char*)&(vtData.intVal), sizeof(vtData.intVal), ulPos);
				break;
			case VT_UINT:
				Insert((const unsigned char*)&(vtData.uintVal), sizeof(vtData.uintVal), ulPos);
				break;
	#ifndef _WIN32_WCE
			case VT_I8:
				Insert((const unsigned char*)&(vtData.llVal), sizeof(vtData.llVal), ulPos);
				break;
			case VT_UI8:
				Insert((const unsigned char*)&(vtData.ullVal), sizeof(vtData.ullVal), ulPos);
				break;
	#else
			case VT_I8:
				Insert((const unsigned char*)&(vtData.lVal), sizeof(LONGLONG), ulPos);
				break;
			case VT_UI8:
				Insert((const unsigned char*)&(vtData.ulVal), sizeof(ULONGLONG), ulPos);
				break;
	#endif
			case VT_FILETIME:
				Insert((const unsigned char*)&(vtData.lVal), sizeof(FILETIME), ulPos);
				break;
			default:
				if((vtData.vt & VT_ARRAY) == VT_ARRAY)
				{	
					ATLASSERT(vtData.parray->cDims == 1);
					ATLASSERT(vtData.parray->rgsabound[0].lLbound == 0);
					const unsigned char *pBuffer = UNULL_PTR;
					ULONG ulSize = vtData.parray->rgsabound[0].cElements;
					ATLASSERT(ulSize);
					Insert((const unsigned char*)&ulSize, sizeof(ulSize), ulPos);
					ulPos += sizeof(ulSize);
					::SafeArrayAccessData(vtData.parray, (void**)&pBuffer);
					ATLASSERT(pBuffer != UNULL_PTR);

#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
					if(pBuffer == UNULL_PTR)
						throw std::exception();
#endif
					switch(vtData.vt)
					{
					case (VT_UI1|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(CHAR), ulPos);
						break;
					case (VT_I1|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(BYTE), ulPos);
						break;
					case (VT_I2|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(SHORT), ulPos);
						break;
					case (VT_UI2|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(USHORT), ulPos);
						break;
					case (VT_I4|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(LONG), ulPos);
						break;
					case (VT_UI4|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(ULONG), ulPos);
						break;
					case (VT_R4|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(FLOAT), ulPos);
						break;
					case (VT_R8|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(DOUBLE), ulPos);
						break;
					case (VT_CY|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(CY), ulPos);
						break;
					case (VT_DATE|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(DATE), ulPos);
						break;
					case (VT_BOOL|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(VARIANT_BOOL), ulPos);
						break;
					case (VT_VARIANT|VT_ARRAY):
						{
							unsigned long ul;
							VARIANT *pvt = (VARIANT *)pBuffer;
							if(ulPos >= GetSize())
							{
								for(ul=0; ul<ulSize; ul++)
								{
									PushVT(pvt[ul]);
								}
							}
							else
							{
								CUQueue	UQueue;
								for(ul=0; ul<ulSize; ul++)
								{
									UQueue.PushVT(pvt[ul]);
								}
								Insert(UQueue.GetBuffer(), UQueue.GetSize(), ulPos);
							}
						}
						break;
					case (VT_BSTR|VT_ARRAY):
						{
							unsigned long ul;
							BSTR *pbstr = (BSTR *)pBuffer;
							if(ulPos >= GetSize())
							{
								for(ul=0; ul<ulSize; ul++)
								{
									if(pbstr[ul] != UNULL_PTR)
									{
										//0xFFFFFFFF == UNULL_PTR
										//0 == L""
										unsigned long ulLen = ::SysStringLen(pbstr[ul])*sizeof(wchar_t);
										Push(&ulLen);
										Push((const unsigned char*)pbstr[ul], ulLen);
									}
									else
									{
										//0xFFFFFFFF == UNULL_PTR
										//0 == L""
										unsigned long ulLen = UQUEUE_NULL_LENGTH;
										Push(&ulLen);
									}
								}
							}
							else
							{
								CUQueue	UQueue;
								for(ul=0; ul<ulSize; ul++)
								{
									if(pbstr[ul])
									{
										//0xFFFFFFFF == UNULL_PTR
										//0 == L""
										unsigned long ulLen = ::SysStringLen(pbstr[ul])*sizeof(wchar_t);
										UQueue.Push(&ulLen);
										UQueue.Push((const unsigned char*)pbstr[ul], ulLen);
									}
									else
									{
										//0xFFFFFFFF == UNULL_PTR
										//0 == L""
										unsigned long ulLen = UQUEUE_NULL_LENGTH;
										UQueue.Push(&ulLen);
									}
								}
								Insert(UQueue.GetBuffer(), UQueue.GetSize(), ulPos);
							}
						}
						break;
					case (VT_DECIMAL|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(DECIMAL), ulPos);
						break;
					case (VT_INT|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(INT), ulPos);
						break;
					case (VT_UINT|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(UINT), ulPos);
						break;
					case (VT_I8|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(LONGLONG), ulPos);
						break;
					case (VT_UI8|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(ULONGLONG), ulPos);
						break;
					case (VT_FILETIME|VT_ARRAY):
						Insert(pBuffer, ulSize*sizeof(FILETIME), ulPos);
						break;
					default:
						ATLASSERT(false);	//not implemented
						break;
					}
					::SafeArrayUnaccessData(vtData.parray);
				}
				else
				{
					ATLASSERT(false);	//not implemented
				}
				break;
			}
		}

		void Push(const CComVariant *pvtData)
		{
			if(pvtData == UNULL_PTR)
				return;
			InsertVT(*pvtData, UQUEUE_END_POSTION);
		}

		void Push(const VARIANT *pvtData)
		{
			if(pvtData == UNULL_PTR)
				return;
			InsertVT(*pvtData, UQUEUE_END_POSTION);
		}

		void PushVT(const VARIANT &vtData) 
		{
			InsertVT(vtData, UQUEUE_END_POSTION);
		}

		inline unsigned long Pop(unsigned long ulBufferLen, unsigned long ulPos = 0)
		{
			if(ulBufferLen == 0)
				return 0;
			if(ulPos >= m_ulLen)
				return 0;
			if(ulBufferLen > m_ulLen - ulPos)
			{
				ulBufferLen = m_ulLen - ulPos;
			}
			m_ulLen -=ulBufferLen;
			if(m_ulLen)
			{
				if(ulPos == 0)
					m_ulHeadPos += ulBufferLen;
				else
					::memcpy(m_pBuffer+ulPos+m_ulHeadPos, m_pBuffer+ulBufferLen+ulPos+m_ulHeadPos, m_ulLen-ulPos);
			}
			else
			{
				m_ulHeadPos = 0;
			}
			return ulBufferLen;
		}

		inline unsigned long Pop(unsigned char* pData, unsigned long ulBufferLen, unsigned long ulPos = 0)
		{
			if(pData==UNULL_PTR || ulBufferLen==0 || m_pBuffer==UNULL_PTR || m_ulLen==0)
				return 0;
			if(ulPos >= m_ulLen)
				return 0;
			if(ulBufferLen> m_ulLen - ulPos)
				ulBufferLen = m_ulLen - ulPos;
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
			if(ulBufferLen > m_ulLen)
				throw std::exception();
#endif
			unsigned long ulTemp = ulPos+m_ulHeadPos;
			::memcpy(pData, m_pBuffer + ulTemp, ulBufferLen);
			m_ulLen -=ulBufferLen;
			if(m_ulLen)
			{
				if(ulPos != 0)
					::memcpy(m_pBuffer + ulTemp, m_pBuffer + ulBufferLen + ulTemp, m_ulLen-ulPos);
				else
					m_ulHeadPos += ulBufferLen;
			}
			else
			{
				m_ulHeadPos = 0;
			}
			return ulBufferLen;
		}

		unsigned long Pop(CComVariant *pvtData, unsigned long ulPos = 0)
		{
			if(pvtData == UNULL_PTR)
				return 0;
			return PopVT(*pvtData, ulPos);
		}

		unsigned long Pop(VARIANT *pvtData, unsigned long ulPos = 0)
		{
			if(pvtData == UNULL_PTR)
				return 0;
			return PopVT(*pvtData, ulPos);
		}

		unsigned long PopVT(VARIANT& vtData, unsigned long ulPos = 0)
		{
			unsigned long ulTotal = 0;
#ifndef _WIN32_WCE
			try
			{
#endif
				if(vtData.vt == VT_BSTR)
					::VariantClear(&vtData);
				else if((vtData.vt & VT_ARRAY) == VT_ARRAY)
					::VariantClear(&vtData);
#ifndef _WIN32_WCE
			}
			catch(...)
			{
			}
#endif
			ulTotal = Pop((unsigned char*)&(vtData.vt), sizeof(vtData.vt), ulPos);
			switch(vtData.vt)
			{
			case VT_NULL:
				break;
			case VT_EMPTY:
				break;
			case VT_I1:
				ulTotal += Pop((unsigned char*)&(vtData.cVal), sizeof(vtData.cVal), ulPos);
				break;
			case VT_UI1:
				ulTotal += Pop((unsigned char*)&(vtData.bVal), sizeof(vtData.bVal), ulPos);
				break;
			case VT_I2:
				ulTotal += Pop((unsigned char*)&(vtData.iVal), sizeof(vtData.iVal), ulPos);
				break;
			case VT_UI2:
				ulTotal += Pop((unsigned char*)&(vtData.uiVal), sizeof(vtData.uiVal), ulPos);
				break;
			case VT_I4:
				ulTotal += Pop((unsigned char*)&(vtData.lVal), sizeof(vtData.lVal), ulPos);
				break;
			case VT_UI4:
				ulTotal += Pop((unsigned char*)&(vtData.ulVal), sizeof(vtData.ulVal), ulPos);
				break;
			case VT_R4:
				ulTotal += Pop((unsigned char*)&(vtData.fltVal), sizeof(vtData.fltVal), ulPos);
				break;
			case VT_R8:
				ulTotal += Pop((unsigned char*)&(vtData.dblVal), sizeof(vtData.dblVal), ulPos);
				break;
			case VT_DATE:
				ulTotal += Pop((unsigned char*)&(vtData.date), sizeof(vtData.date), ulPos);
				break;
			case VT_BOOL:
				ulTotal += Pop((unsigned char*)&(vtData.boolVal), sizeof(vtData.boolVal), ulPos);
				break;
			case VT_CY:
				ulTotal += Pop((unsigned char*)&(vtData.cyVal), sizeof(vtData.cyVal), ulPos);
				break;
			case VT_BSTR:
				{
					unsigned long ulLen = 0;
					ulTotal += Pop((unsigned char*)&ulLen, sizeof(ulLen), ulPos);
					if(ulLen == UQUEUE_NULL_LENGTH)
					{
						vtData.bstrVal = UNULL_PTR;
					}
					else if(ulLen)
					{
						ATLASSERT(ulLen<=GetSize());
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
						if(ulLen > GetSize() || (ulLen%sizeof(wchar_t)) > 0)
							throw std::exception();
#endif
						vtData.bstrVal = ::SysAllocStringLen((const wchar_t*)GetBuffer(), ulLen/sizeof(wchar_t));

#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
						if(vtData.bstrVal == UNULL_PTR)
							throw std::exception();
#endif
						Pop(ulLen, ulPos);
						ulTotal += ulLen;
					}
					else
					{
						vtData.bstrVal = ::SysAllocString(L"");
					}
				}
				break;
			case VT_INT:
				ulTotal += Pop((unsigned char*)&(vtData.intVal), sizeof(vtData.intVal), ulPos);
				break;
			case VT_UINT:
				ulTotal += Pop((unsigned char*)&(vtData.uintVal), sizeof(vtData.uintVal), ulPos);
				break;
	#ifndef _WIN32_WCE
			case VT_I8:
				ulTotal += Pop((unsigned char*)&(vtData.llVal), sizeof(vtData.llVal), ulPos);
				break;
			case VT_UI8:
				ulTotal += Pop((unsigned char*)&(vtData.ullVal), sizeof(vtData.ullVal), ulPos);
				break;
	#else
			case VT_I8:
				ulTotal += Pop((unsigned char*)&(vtData.lVal), sizeof(LONGLONG), ulPos);
				break;
			case VT_UI8:
				ulTotal += Pop((unsigned char*)&(vtData.ulVal), sizeof(ULONGLONG), ulPos);
				break;
	#endif
			case VT_FILETIME:
				ulTotal += Pop((unsigned char*)&(vtData.lVal), sizeof(FILETIME), ulPos);
				break;
			case VT_DECIMAL:
				ulTotal += Pop((unsigned char*)&(vtData.decVal), sizeof(vtData.decVal), ulPos);
				vtData.vt = VT_DECIMAL;
				break;
			default:
				if((vtData.vt & VT_ARRAY) == VT_ARRAY)
				{
					ULONG	ulSize;
					SAFEARRAY	*psa;
					void		*pBuffer;
					ulTotal += Pop((unsigned char*)&ulSize, sizeof(ulSize), ulPos);
					ATLASSERT(ulSize);
					SAFEARRAYBOUND sab[1] = {ulSize, 0};
					switch (vtData.vt)
					{
					case (VT_I1|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_I1, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(CHAR), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_UI1|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_UI1, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(BYTE), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_I2|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_I2, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(SHORT), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_UI2|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_UI2, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(USHORT), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_I4|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_I4, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(LONG), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_UI4|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_UI4, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(ULONG), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_R4|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_R4, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(FLOAT), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_R8|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_R8, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(DOUBLE), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_BOOL|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_BOOL, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(VARIANT_BOOL), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_DATE|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_DATE, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(DATE), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_CY|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_CY, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(CY), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_VARIANT|VT_ARRAY):
						{
							unsigned long ul;
							psa = ::SafeArrayCreate(VT_VARIANT, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							for(ul=0; ul<ulSize; ul++)
							{
								ulTotal += PopVT(((VARIANT*)pBuffer)[ul], ulPos);
							}
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_BSTR|VT_ARRAY):
						{
							unsigned long ulLen;
							unsigned long ulIndex;
							BSTR			*pbstr;
							psa = ::SafeArrayCreate(VT_BSTR, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							pbstr = (BSTR*)pBuffer;
							for(ulIndex=0; ulIndex<ulSize; ulIndex++)
							{
								ulTotal += Pop((unsigned char*)&ulLen, sizeof(ulLen), ulPos);
								if(ulLen == UQUEUE_NULL_LENGTH) //null
								{
									pbstr[ulIndex] = UNULL_PTR;
								}
								else if(ulLen)
								{
									ATLASSERT(ulLen<=GetSize());
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
									if(ulLen > GetSize() || (ulLen%sizeof(wchar_t)) > 0)
										throw std::exception();
#endif
									pbstr[ulIndex] = ::SysAllocStringLen((const wchar_t*)GetBuffer(), ulLen/sizeof(wchar_t));
									
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
									if(pbstr[ulIndex] == UNULL_PTR)
										throw std::exception();
#endif
									Pop(ulLen, ulPos);
									ulTotal += ulLen;
								}
								else //0
								{
									pbstr[ulIndex] = ::SysAllocString(L"");
								}
							}
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_INT|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_INT, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(INT), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_UINT|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_UINT, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(UINT), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_I8|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_I8, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(LONGLONG), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_UI8|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_UI8, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(ULONGLONG), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_FILETIME|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_FILETIME, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(FILETIME), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					case (VT_DECIMAL|VT_ARRAY):
						{
							psa = ::SafeArrayCreate(VT_DECIMAL, 1, sab);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
							if(psa == UNULL_PTR)
								throw std::exception();
#endif
							::SafeArrayAccessData(psa, &pBuffer);
							ulTotal += Pop((unsigned char*)pBuffer, ulSize*sizeof(DECIMAL), ulPos);
							::SafeArrayUnaccessData(psa);
						}
						break;
					default:
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
						throw std::exception();
#endif
						break;
					}
					vtData.parray = psa;
				}
				else
				{
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
					throw std::exception();
#endif
				}
				break;
			}
			return ulTotal;
		}

		template<class ctype>
		inline void Insert(const ctype* pData, unsigned long ulPos=0)
		{
			Insert((const unsigned char*)pData, sizeof(ctype), ulPos);
		}

		/*inline unsigned long Pop(IUSerializer *p)
		{
			unsigned long ulStart = GetSize();
			if(p != UNULL_PTR)
				p->LoadFrom(*this);
			return (ulStart - GetSize());
		}*/

		template <class ctype>
		inline unsigned long Pop(ctype *pData, unsigned long ulPos = 0) 
		{
			return Pop((unsigned char*)pData, sizeof(ctype), ulPos);
		}
		
		template <class ctype>
		inline void Push(const ctype *pData)
		{
			Push((const unsigned char*)pData, sizeof(ctype));
		}

		/*inline void Push(IUSerializer *p)
		{
			if(p != UNULL_PTR)
				p->SaveTo(*this);
		}*/

		/*
			Insert an array of data into memory queue with a size ahead.
		*/
		template<class ctype>
		inline void InsertWithSize(const ctype* pData, unsigned long ulSize, unsigned long ulPos=0)
		{
			if(pData == UNULL_PTR) 
			{
				ulSize = UQUEUE_SIZE_NULL;
			}
			Insert(&ulSize, ulPos);
			if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
			{
				if(ulPos > GetSize())
				{
					ulPos = GetSize();
				}
				else
				{
					ulPos += sizeof(ulSize);
				}
				Insert((const unsigned char*)pData, sizeof(ctype)*ulSize, ulPos);
			}
		}

		void InsertWithSize(CComVariant *pData, unsigned long ulSize, unsigned long ulPos=0)
		{
			InsertWithSize((VARIANT *)pData, ulSize, ulPos);
		}

		void InsertWithSize(VARIANT *pData, unsigned long ulSize, unsigned long ulPos=0)
		{
			if(pData == UNULL_PTR) 
			{
				ulSize = UQUEUE_SIZE_NULL;
			}
			Insert(&ulSize, ulPos);
			if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
			{
				unsigned long ul;
				if(ulPos > GetSize())
				{
					ulPos = GetSize();
				}
				else
				{
					ulPos += sizeof(ulSize);
				}
				if(ulPos >= GetSize())
				{
					for(ul=0; ul<ulSize; ul++)
					{
						PushVT(pData[ul]);
					}
				}
				else
				{
					CUQueue	UQueue;
					for(ul=0; ul<ulSize; ul++)
					{
						UQueue.PushVT(pData[ul]);
					}
					Insert(UQueue.GetBuffer(), UQueue.GetSize(), ulPos);
				}
			}
		}

		template <class ctype>
		inline void PushWithSize(const ctype *pData, unsigned long ulSize)
		{
			ULONG ulPos = GetSize();
			InsertWithSize(pData, ulSize, ulPos);
		}

		CUQueue& operator << (const std::string &s)
		{
			ULONG ulSize = (ULONG)s.size();
			Push((const unsigned char*)&ulSize, sizeof(ulSize));
			Push((const unsigned char*)s.c_str(), ulSize);
			return *this;
		}

		CUQueue& operator >> (std::string &s)
		{
			ULONG ulSize;
			ATLASSERT(GetSize() >= sizeof(ulSize));
			Pop((unsigned char*)&ulSize, sizeof(ulSize));
			if(ulSize == 0 || ulSize ==  UQUEUE_SIZE_NULL)
			{
#if _MSC_VER < 1350
				s = "";
#else
				s.clear();
#endif
			}
			else
			{
				ATLASSERT(ulSize <= GetSize());
				s.assign((const char*)GetBuffer(), ulSize);
				Pop(ulSize);
			}
			return *this;
		}

		CUQueue& operator << (const std::wstring &s)
		{
			ULONG ulSize = (ULONG)s.size()*sizeof(WCHAR);
			Push((const unsigned char*)&ulSize, sizeof(ulSize));
			Push((const unsigned char*)s.c_str(), ulSize);
			return *this;
		}

		CUQueue& operator >> (std::wstring &s)
		{
			ULONG ulSize;
			ATLASSERT(GetSize() >= sizeof(ulSize));
			Pop((unsigned char*)&ulSize, sizeof(ulSize));

			//make sure that you really serialized a unicode string before!
			if(ulSize == 0 || ulSize ==  UQUEUE_SIZE_NULL)
			{
#if _MSC_VER < 1350
				s = L"";
#else
				s.clear();
#endif
			}
			else
			{
				ATLASSERT((ulSize%sizeof(WCHAR)) == 0 && ulSize <= GetSize());
				s.assign((LPCWSTR)GetBuffer(), ulSize/sizeof(WCHAR));
				Pop(ulSize);
			}
			return *this;
		}

		/*
			All of string operations with operators <<, >>, +=, +, and & lead to 
			add a length in byte ahead.

			It will make coding more user-friendly and portable 
			with the expense of extra 4 bytes among various types of strings.
		*/
		CUQueue& operator << (const CComBSTR &bstr )
		{
			ULONG ulSize;
			if(bstr.m_str == UNULL_PTR)
				ulSize = UQUEUE_SIZE_NULL;
			else
				ulSize = bstr.Length() * sizeof(WCHAR);
			Push((const unsigned char*)&ulSize, sizeof(ulSize));
			if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
			{
				Push((const BYTE*)bstr.m_str, ulSize);
			}
			return *this;
		}

		CUQueue& operator >> (CComBSTR &bstr)
		{
			ULONG ulSize;
			ATLASSERT(GetSize() >= sizeof(ulSize));
			Pop((unsigned char*)&ulSize, sizeof(ulSize));
			if(ulSize == UQUEUE_SIZE_NULL)
			{
				bstr.Empty();
			}
			else if (ulSize == 0)
			{
				bstr = L"";
			}
			else
			{
				ATLASSERT((ulSize % sizeof(WCHAR)) == 0);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
				if((ulSize % sizeof(WCHAR)) != 0)
					throw std::exception();
#endif
				ATLASSERT(GetSize() >= ulSize);
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
				if(GetSize() < ulSize)
					throw std::exception();
#endif
				BSTR bstrTemp = ::SysAllocStringLen((LPCWSTR)GetBuffer(), ulSize/sizeof(WCHAR));
				bstr.Attach(bstrTemp);
				Pop(ulSize);
			}
			return *this;
		}

		inline CUQueue& operator << (CUQueue &UQueue )
		{
			ULONG ulSize = UQueue.GetSize();
			Push((const unsigned char*)&ulSize, sizeof(ulSize));
			Push(UQueue.GetBuffer(), ulSize);
			return *this;
		}

		inline CUQueue& operator>>(CUQueue &UQueue)
		{
			ULONG ulSize;
			ATLASSERT(GetSize() >= sizeof(ulSize));
			Pop((unsigned char*)&ulSize, sizeof(ulSize));
			if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
			{
				ATLASSERT(ulSize <= GetSize());
#if !defined(_WIN32_WCE) || (_MSC_VER > 1300)
				if(ulSize > GetSize())
					throw std::exception();
#endif
				UQueue.Push(GetBuffer(), ulSize);
				Pop(ulSize);
			}
			return *this;
		}

		CUQueue& operator<<(const VARIANT &vtData)
		{
			PushVT(vtData);
			return *this;
		}

		CUQueue& operator>>(VARIANT &vtData)
		{
			PopVT(vtData);
			return *this;
		}

		CUQueue& operator<<(const CComVariant &vtData)
		{
			PushVT(vtData);
			return *this;
		}

		CUQueue& operator>>(CComVariant &vtData)
		{
			PopVT(vtData);
			return *this;
		}
		
		/*inline CUQueue& operator << (IUSerializer *p)
		{
			if(p != UNULL_PTR)
				p->SaveTo(*this);
			return *this;
		}

		inline CUQueue& operator >> (IUSerializer *p)
		{
			if(p != UNULL_PTR)
				p->LoadFrom(*this);
			return *this;
		}*/

		inline CUQueue& operator << (LPCSTR strData)
		{
			ULONG ulSize;
			if(strData == UNULL_PTR)
			{
				ulSize = UQUEUE_SIZE_NULL;
			}
			else
			{
				ulSize = ::strlen(strData);
			}
			Push(&ulSize);
			if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
			{
				Push(strData, ulSize);
			}
			return *this;
		}

		inline CUQueue& operator << (LPCWSTR strData)
		{
			ULONG ulSize;
			if(strData == UNULL_PTR)
			{
				ulSize = UQUEUE_SIZE_NULL;
			}
			else
			{
				ulSize = ::wcslen(strData)*sizeof(WCHAR);
			}
			Push(&ulSize);
			if(ulSize != UQUEUE_SIZE_NULL && ulSize != 0)
			{
				Push((const BYTE*)strData, ulSize);
			}
			return *this;
		}

		inline CUQueue& operator << (unsigned short data)
		{
			Push((BYTE*)&data, sizeof(data));
			return *this;
		}

#if _MSC_VER >= 1350
		template<class ctype>
		inline CUQueue& operator << (const ctype &data)
		{
			Push(&data);
			return *this;
		}

		template<class ctype>
		inline CUQueue& operator >> (ctype &data)
		{
			Pop(&data);
			return *this;
		}
#endif


	private:
		unsigned long	m_ulMaxBuffer;
		unsigned long	m_ulLen;
		unsigned char	*m_pBuffer;
		unsigned long	m_ulBlockSize;
		unsigned long	m_ulHeadPos;
		bool			m_bScope;
		friend class CScopeUQueue;
	};

	#pragma warning(default: 4267) // warning C4267: '=' : conversion from 'size_t' to 'ULONG', possible loss of data
	
#if _MSC_VER < 1350
	template<class ctype>
	inline CUQueue& operator << (CUQueue &UQueue, const ctype &data)
	{
		UQueue.Push(&data);
		return UQueue;
	}

	template<class ctype>
	inline CUQueue& operator >> (CUQueue &UQueue, ctype &data)
	{
		UQueue.Pop(&data);
		return UQueue;
	}
#endif
	

	/*
		You may need to overwrite the template operators << and >> for your data types.
		You can see samples inside the files ..\SocketProAdapter\cplusplus\ucomutil.h, umfcutil.h and stdustring.h
	*/
/*	template<class ctype>
	CUQueue& operator << (CUQueue &UQueue, const ctype &data)
	{
		UQueue.Push(&data);
		return UQueue;
	}

	template<class ctype>
	CUQueue& operator >> (CUQueue &UQueue, ctype &data)
	{
		ATLASSERT(UQueue.GetSize() >= sizeof(data));
		UQueue.Pop(&data);
		return UQueue;
	}*/
};

#endif

// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com

