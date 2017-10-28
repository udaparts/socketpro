
#ifndef _UDAPARTS_GENERAL_CACHE_BASE_PEER_H_
#define _UDAPARTS_GENERAL_CACHE_BASE_PEER_H_

#include "aserverw.h"
#include "udatabase.h"

namespace SPA {
    namespace ServerSide {

        class CCacheBasePeer : public CClientPeer {
            CCacheBasePeer(const CCacheBasePeer &p);
            CCacheBasePeer& operator=(const CCacheBasePeer &p);

        public:

            CCacheBasePeer() {
            }

            bool SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index) const {
                //A client expects a rowset meta data and call index
                unsigned int ret = SendResult(SPA::UDB::idRowsetHeader, meta, index);
                return (ret != REQUEST_CANCELED && ret != SOCKET_NOT_FOUND);
            }

            bool SendRows(const SPA::UDB::CDBVariantArray &vData) const {
                unsigned int len;
                SPA::CScopeUQueue sb;
                for (auto it = vData.begin(), end = vData.end(); it != end; ++it) {
                    VARTYPE vt = it->Type();
                    switch (vt) {
                        case VT_BSTR:
                            len = ::SysStringLen(it->bstrVal);
                            if (len > UDB::DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                if (sb->GetSize() && !SendRows(sb, true))
                                    return false;
                                SPA::CScopeUQueue temp;
                                temp << *it;
                                const unsigned int *bytes = (const unsigned int *) temp->GetBuffer(sizeof (VARTYPE));
                                const unsigned char *buffer = temp->GetBuffer(sizeof (VARTYPE) + sizeof (unsigned int));
                                if (!SendBlob(vt, buffer, *bytes))
                                    return false;
                            } else {
                                sb << *it;
                            }
                            break;
                        case (VT_I1 | VT_ARRAY):
                        case (VT_UI1 | VT_ARRAY):
                            len = it->parray->rgsabound->cElements;
                            if (len > 2 * UDB::DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                if (sb->GetSize() && !SendRows(sb, true))
                                    return false;
                                const unsigned char *buffer;
                                ::SafeArrayAccessData(it->parray, (void**) &buffer);
                                bool ok = SendBlob(vt, buffer, len);
                                ::SafeArrayUnaccessData(it->parray);
                                if (!ok)
                                    return false;
                            } else {
                                sb << *it;
                            }
                            break;
                        default:
                            sb << *it;
                            break;
                    }
                }
                len = SendResult(SPA::UDB::idEndRows, sb);
                return (len != REQUEST_CANCELED && len != SOCKET_NOT_FOUND);
            }

        protected:
			virtual void GetCachedTables(const std::wstring &defaultDb, unsigned int flags, bool rowset, UINT64 index, int &res, std::wstring &errMsg) = 0;

            bool SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes) const {
                unsigned int ret = SendResult(UDB::idStartBLOB,
                        (unsigned int) (bytes + sizeof (unsigned short) + sizeof (unsigned int) + sizeof (unsigned int))/* extra 4 bytes for string null termination*/,
                        data_type, bytes);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                while (bytes > UDB::DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    ret = SendResult(UDB::idChunk, buffer, UDB::DEFAULT_BIG_FIELD_CHUNK_SIZE);
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                        return false;
                    }
                    assert(ret == UDB::DEFAULT_BIG_FIELD_CHUNK_SIZE);
                    buffer += UDB::DEFAULT_BIG_FIELD_CHUNK_SIZE;
                    bytes -= UDB::DEFAULT_BIG_FIELD_CHUNK_SIZE;
                }
                ret = SendResult(UDB::idEndBLOB, buffer, bytes);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                return true;
            }

            bool SendRows(CScopeUQueue& sb, bool transferring) const {
                bool batching = (GetBytesBatched() >= UDB::DEFAULT_RECORD_BATCH_SIZE);
                if (batching) {
                    CommitBatching();
                }
                unsigned int ret = SendResult(transferring ? UDB::idTransferring : UDB::idEndRows, sb->GetBuffer(), sb->GetSize());
                sb->SetSize(0);
                if (batching) {
                    StartBatching();
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                return true;
            }
        };
    } //namespace ServerSide
} //namespace SPA

#endif
