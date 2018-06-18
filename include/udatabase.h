
#ifndef __UDAPARTS_DATABASE_COMM_H___
#define __UDAPARTS_DATABASE_COMM_H___

#include "ucomm.h"
#include "membuffer.h"

namespace SPA {

    namespace UDB {

        enum tagTransactionIsolation {
            tiUnspecified = -1,
            tiChaos = 0,
            tiReadUncommited = 1,
            tiBrowse = 2,
            tiCursorStability = 3,
            tiReadCommited = tiCursorStability,
            tiRepeatableRead = 4,
            tiSerializable = 5,
            tiIsolated = 6
        };

        enum tagRollbackPlan {
            /**
             * Manual transaction will rollback whenever there is an error by default
             */
            rpDefault = 0,
            /**
             * Manual transaction will rollback whenever there is an error by default
             */
            rpRollbackErrorAny = rpDefault,

            /**
             * Manual transaction will rollback as long as the number of errors is less than the number of ok processing statements
             */
            rpRollbackErrorLess = 1,

            /**
             * Manual transaction will rollback as long as the number of errors is less or equal than the number of ok processing statements
             */
            rpRollbackErrorEqual = 2,

            /**
             * Manual transaction will rollback as long as the number of errors is more than the number of ok processing statements
             */
            rpRollbackErrorMore = 3,

            /**
             * Manual transaction will rollback only if all the processing statements are failed
             */
            rpRollbackErrorAll = 4,

            /**
             * Manual transaction will rollback always no matter what happens.
             */
            rpRollbackAlways = 5
        };

        enum tagUpdateEvent {
            ueUnknown = -1,

            /**
             * An event for inserting a record into a table
             */
            ueInsert = 0,

            /**
             * An event for updating a record of a table
             */
            ueUpdate = 1,

            /**
             * An event for deleting a record from a table
             */
            ueDelete = 2,
        };

        enum tagManagementSystem {
            msUnknown = -1,
            msSqlite = 0,
            msMysql = 1,
            msODBC = 2,
            msMsSQL = 3,
            msOracle = 4,
            msDB2 = 5,
            msPostgreSQL = 6,
            msMongoDB = 7
        };

        enum tagVTExt {
            vteNormal = 0,
            vteGuid = 1
        };

        /**
         * Async database client/server just requires the following request identification numbers 
         */
        static const unsigned short idOpen = 0x7E7F;
        static const unsigned short idClose = idOpen + 1;
        static const unsigned short idBeginTrans = idClose + 1;
        static const unsigned short idEndTrans = idBeginTrans + 1;
        static const unsigned short idExecute = idEndTrans + 1;
        static const unsigned short idPrepare = idExecute + 1;
        static const unsigned short idExecuteParameters = idPrepare + 1;

        /**
         * the request identification numbers used for message push from server to client
         */
        static const unsigned short idDBUpdate = idExecuteParameters + 1; //server ==> client only
        static const unsigned short idRowsetHeader = idDBUpdate + 1; //server ==> client only
        static const unsigned short idOutputParameter = idRowsetHeader + 1; //server ==> client only

        /**
         * Internal request/response identification numbers used for data communication between client and server
         */
        static const unsigned short idBeginRows = idOutputParameter + 1;
        static const unsigned short idTransferring = idBeginRows + 1;
        static const unsigned short idStartBLOB = idTransferring + 1;
        static const unsigned short idChunk = idStartBLOB + 1;
        static const unsigned short idEndBLOB = idChunk + 1;
        static const unsigned short idEndRows = idEndBLOB + 1;
        static const unsigned short idCallReturn = idEndRows + 1;

        static const unsigned short idGetCachedTables = idCallReturn + 1;

        static const unsigned short idSqlBatchHeader = idGetCachedTables + 1;
        static const unsigned short idExecuteBatch = idSqlBatchHeader + 1;
        static const unsigned short idParameterPosition = idExecuteBatch + 1;

        /**
         * Whenever a data size in bytes is about twice larger than the defined value,
         * the data will be treated in large object and transferred in chunks for reducing memory foot print
         */
        static const unsigned int DEFAULT_BIG_FIELD_CHUNK_SIZE = 16 * 1024;

        /**
         * A record data size in bytes is approximately equal to or slightly larger than the defined constant
         */
        static const unsigned int DEFAULT_RECORD_BATCH_SIZE = 16 * 1024;

        /**
         * A flag used with idOpen for tracing database table update events
         */
        static const unsigned int ENABLE_TABLE_UPDATE_MESSAGES = 0x1;

        /**
         * A chat group id used at SocketPro server side for notifying database events from server to connected clients
         */
        static const unsigned int STREAMING_SQL_CHAT_GROUP_ID = 0x1fffffff;

        static const unsigned int CACHE_UPDATE_CHAT_GROUP_ID = STREAMING_SQL_CHAT_GROUP_ID + 1;

        /**
         * VARINAT data types for OLEDB
         */
        static const unsigned short VT_BYTES = 128; //OLEDB data type for binary array
        static const unsigned short VT_STR = 129; //OLEDB data type for ASCII string
        static const unsigned short VT_WSTR = 130; //OLEDB data type for unicode string

#ifdef WIN32_64
        static const double MIN_WIN_DATETIME = 0.000001 / 3600 / 24;
#endif

        class CDBVariant : public CComVariant {
        public:
            tagVTExt VtExt;

            CDBVariant(CDBVariant &&vtData) {
                CDBVariant &me = *this;
                me = (tagVARIANT&&)vtData;
                VtExt = vtData.VtExt;
                vtData.VtExt = vteNormal;
            }

            CDBVariant(CComVariant &&vtData) {
                CDBVariant &me = *this;
                me = (tagVARIANT&&)vtData;
                VtExt = vteNormal;
            }

            CDBVariant(tagVARIANT &&vtData) {
                CDBVariant &me = *this;
                me = vtData;
                VtExt = vteNormal;
            }

            CDBVariant(const CDBVariant &vt) : CComVariant(vt), VtExt(vt.VtExt) {
            }

            CDBVariant(const CComVariant &vt) : CComVariant(vt), VtExt(vteNormal) {
            }

            CDBVariant(const tagVARIANT &vt) : CComVariant(vt), VtExt(vteNormal) {
            }

            CDBVariant() {
                vt = VT_NULL;
                VtExt = vteNormal;
            }

            CDBVariant(const GUID &uuid) {
                void *buffer;
                SAFEARRAYBOUND sab[1] = {sizeof (GUID), 0};
                parray = ::SafeArrayCreate(VT_UI1, 1, sab);
                ::SafeArrayAccessData(parray, &buffer);
                ::memcpy(buffer, &uuid, sizeof (uuid));
                ::SafeArrayUnaccessData(parray);
                vt = (VT_ARRAY | VT_UI1);
                VtExt = vteGuid;
            }

            template <typename type>
            CDBVariant(const type& src) : CComVariant(src) {
                VtExt = vteNormal;
            }

            CDBVariant(const DECIMAL& src) {
                decVal = src;
                vt = VT_DECIMAL;
                VtExt = vteNormal;
            }

            CDBVariant(const unsigned char *buffer, unsigned int bytes) {
                if (buffer) {
                    SAFEARRAYBOUND sab[1] = {bytes, 0};
                    parray = ::SafeArrayCreate(VT_UI1, 1, sab);
                    if (bytes) {
                        void *b;
                        ::SafeArrayAccessData(parray, &b);
                        ::memcpy(b, buffer, bytes);
                        ::SafeArrayUnaccessData(parray);
                    }
                    vt = (VT_UI1 | VT_ARRAY);
                } else {
                    vt = VT_NULL;
                }
                VtExt = vteNormal;
            }

            CDBVariant(const char* lpszSrc, unsigned int len = UQUEUE_NULL_LENGTH) {
                if (lpszSrc) {
                    if (len == UQUEUE_NULL_LENGTH) {
                        len = (unsigned int) ::strlen(lpszSrc);
                    }
                    SAFEARRAYBOUND sab[1] = {len, 0};
                    parray = ::SafeArrayCreate(VT_I1, 1, sab);
                    if (len) {
                        void *buffer;
                        ::SafeArrayAccessData(parray, &buffer);
                        ::memcpy(buffer, lpszSrc, len);
                        ::SafeArrayUnaccessData(parray);
                    }
                    vt = (VT_I1 | VT_ARRAY);
                } else {
                    vt = VT_NULL;
                }
                VtExt = vteNormal;
            }

            CDBVariant(const wchar_t* lpszSrc, unsigned int len = UQUEUE_NULL_LENGTH) {
                if (lpszSrc) {
                    if (len == (unsigned int) (~0)) {
                        len = (unsigned int) ::wcslen(lpszSrc);
                    }
                    bstrVal = ::SysAllocStringLen(lpszSrc, len);
                    vt = VT_BSTR;
                } else {
                    vt = VT_NULL;
                }
                VtExt = vteNormal;
            }

            CDBVariant(const UDateTime &dt) : CComVariant(dt.time) {
                vt = VT_DATE;
                VtExt = vteNormal;
            }

            CDBVariant(const std::tm &st, unsigned int us = 0) : CComVariant(UDateTime(st, us).time) {
                vt = VT_DATE;
                VtExt = vteNormal;
            }

#ifdef WIN32_64

            CDBVariant(CComBSTR &&bstr) {
                if (bstr.m_str) {
                    vt = VT_BSTR;
                    bstrVal = bstr.Detach();
                } else {
                    vt = VT_NULL;
                }
                VtExt = vteNormal;
            }

            CDBVariant(double dblSrc, VARTYPE vtSrc = VT_R8/* or VT_DATE*/, unsigned short us = 0) : CComVariant(dblSrc, vtSrc) {
                if (vtSrc == VT_DATE) {
                    //convert variant date to high precision time on window system
                    UDateTime udt(dblSrc, us);
                    this->ullVal = udt.time;
                }
                VtExt = vteNormal;
            }

            CDBVariant(const SYSTEMTIME &st, unsigned short us = 0) : CComVariant(UDateTime(st, us).time) {
                vt = VT_DATE;
                VtExt = vteNormal;
            }
#else

            CDBVariant(const SYSTEMTIME &st) {
                vt = VT_DATE;
                ullVal = UDateTime(st).time;
                VtExt = vteNormal;
            }
#endif
        public:

            unsigned short Type() const {
                return vt;
            }

            bool operator!=(const VARIANT &data) const {
                return (!(*this == data));
            }

            bool operator==(const VARIANT &data) const {
                if ((vt == VT_NULL || vt == VT_EMPTY) && (data.vt == VT_EMPTY || data.vt == VT_NULL)) {
                    return true;
                } else if (vt == VT_BSTR) {
                    int res;
                    //case-insensitive compare
                    if (data.vt == VT_BSTR) {
#ifdef WIN32_64
                        res = ::_wcsicmp(bstrVal, data.bstrVal);
#else
                        res = ::wcscasecmp(bstrVal, data.bstrVal);
#endif
                    } else if (data.vt == (VT_ARRAY | VT_I1)) {
                        SPA::CScopeUQueue sb;
                        const char *s0 = nullptr;
                        ::SafeArrayAccessData(data.parray, (void**) &s0);
                        SPA::Utilities::ToWide(s0, data.parray->rgsabound->cElements);
                        ::SafeArrayUnaccessData(data.parray);
                        const wchar_t *s = (const wchar_t*)sb->GetBuffer();
#ifdef WIN32_64
                        res = ::_wcsicmp(s, bstrVal);
#else
                        res = ::wcscasecmp(s, bstrVal);
#endif
                    } else {
                        res = -1;
                    }
                    return (res == 0);
                } else if (vt == (VT_ARRAY | VT_I1)) {
                    int res;
                    //case-insensitive compare
                    if (data.vt == (VT_ARRAY | VT_I1)) {
                        if (parray->rgsabound->cElements != data.parray->rgsabound->cElements)
                            return false;
                        const char *s0;
                        ::SafeArrayAccessData(data.parray, (void**) &s0);
                        const char *s;
                        ::SafeArrayAccessData(parray, (void**) &s);
#ifdef WIN32_64
                        res = ::_strnicmp(s0, s, parray->rgsabound->cElements);
#else
                        res = ::strncasecmp(s0, s, parray->rgsabound->cElements);
#endif
                        ::SafeArrayUnaccessData(parray);
                        ::SafeArrayUnaccessData(data.parray);
                    } else if (data.vt == VT_BSTR) {
                        SPA::CScopeUQueue sb;
                        SPA::Utilities::ToUTF8(data.bstrVal, ::wcslen(data.bstrVal), *sb);
                        if (sb->GetSize() != parray->rgsabound->cElements)
                            return false;
                        const char *s0 = (const char*) sb->GetBuffer();
                        const char *s;
                        ::SafeArrayAccessData(parray, (void**) &s);
#ifdef WIN32_64
                        res = ::_strnicmp(s0, s, parray->rgsabound->cElements);
#else
                        res = ::strncasecmp(s0, s, parray->rgsabound->cElements);
#endif
                        ::SafeArrayUnaccessData(parray);
                    } else {
                        res = -1;
                    }
                    return (res == 0);
                } else if (vt == data.vt && vt == (VT_UI1 | VT_ARRAY)) {
                    if (parray->rgsabound->cElements != data.parray->rgsabound->cElements)
                        return false;
                    const unsigned char *s;
                    const unsigned char *s0;
                    ::SafeArrayAccessData(data.parray, (void**) &s0);
                    ::SafeArrayAccessData(parray, (void**) &s);
                    int res = ::memcmp(s, s0, parray->rgsabound->cElements);
                    ::SafeArrayUnaccessData(parray);
                    ::SafeArrayUnaccessData(data.parray);
                    return (res == 0);
                } else if (vt == VT_DECIMAL && vt == data.vt) {
                    return (decVal.Lo64 == data.decVal.Lo64) &&
                            (decVal.Hi32 == data.decVal.Hi32) &&
                            (decVal.signscale == data.decVal.signscale);
                }
                UINT64 d, d0;
                switch (vt) {
                    case VT_BOOL:
                        d = boolVal ? 1 : 0;
                        break;
                    case VT_I1:
                        d = cVal;
                        break;
                    case VT_UI1:
                        d = bVal;
                        break;
                    case VT_I2:
                        d = iVal;
                        break;
                    case VT_UI2:
                        d = uiVal;
                        break;
                    case VT_I4:
                        d = lVal;
                        break;
                    case VT_INT:
                        d = intVal;
                        break;
                    case VT_UI4:
                        d = ulVal;
                        break;
                    case VT_UINT:
                        d = uintVal;
                        break;
                    case VT_I8:
                        d = (UINT64) llVal;
                        break;
                    case VT_UI8:
                        d = ullVal;
                        break;
                    default:
                        return false;
                        break;
                }
                switch (data.vt) {
                    case VT_BOOL:
                        d0 = data.boolVal ? 1 : 0;
                        break;
                    case VT_I1:
                        d0 = data.cVal;
                        break;
                    case VT_UI1:
                        d0 = data.bVal;
                        break;
                    case VT_I2:
                        d0 = data.iVal;
                        break;
                    case VT_UI2:
                        d0 = data.uiVal;
                        break;
                    case VT_I4:
                        d0 = data.lVal;
                        break;
                    case VT_INT:
                        d0 = data.intVal;
                        break;
                    case VT_UI4:
                        d0 = data.ulVal;
                        break;
                    case VT_UINT:
                        d0 = data.uintVal;
                        break;
                    case VT_I8:
                        d0 = (UINT64) data.llVal;
                        break;
                    case VT_UI8:
                        d0 = data.ullVal;
                        break;
                    default:
                        return false;
                        break;
                }
                return (d == d0);
            }

            CDBVariant& operator=(const CDBVariant &vtData) {
                CComVariant &me = *this;
                me = vtData;
                VtExt = vtData.VtExt;
                return *this;
            }

            CDBVariant& operator=(const CComVariant &vtData) {
                CComVariant &me = *this;
                me = vtData;
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(const tagVARIANT &vtData) {
                CComVariant &me = *this;
                me = vtData;
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(const char* lpszSrc) {
                ::VariantClear(this);
                if (lpszSrc) {
                    unsigned int len = (unsigned int) strlen(lpszSrc);
                    SAFEARRAYBOUND sab[1] = {len, 0};
                    parray = ::SafeArrayCreate(VT_I1, 1, sab);
                    if (len) {
                        void *buffer;
                        ::SafeArrayAccessData(parray, &buffer);
                        ::memcpy(buffer, lpszSrc, len);
                        ::SafeArrayUnaccessData(parray);
                    }
                    vt = (VT_I1 | VT_ARRAY);
                } else {
                    vt = VT_NULL;
                }
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(const wchar_t* lpszSrc) {
                CComVariant &me = *this;
                me = lpszSrc;
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(const GUID &uuid) {
                void *buffer;
                ::VariantClear(this);
                SAFEARRAYBOUND sab[1] = {sizeof (GUID), 0};
                parray = ::SafeArrayCreate(VT_UI1, 1, sab);
                ::SafeArrayAccessData(parray, &buffer);
                ::memcpy(buffer, &uuid, sizeof (uuid));
                ::SafeArrayUnaccessData(parray);
                vt = (VT_ARRAY | VT_UI1);
                VtExt = vteGuid;
                return *this;
            }

            CDBVariant& operator=(const DECIMAL& src) {
                ::VariantClear(this);
                decVal = src;
                vt = VT_DECIMAL;
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(const UDateTime &dt) {
                ::VariantClear(this);
                vt = VT_DATE;
                ullVal = dt.time;
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(const SYSTEMTIME &st) {
                ::VariantClear(this);
                vt = VT_DATE;
                ullVal = UDateTime(st).time;
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(const std::tm &st) {
                ::VariantClear(this);
                vt = VT_DATE;
                ullVal = UDateTime(st).time;
                VtExt = vteNormal;
                return *this;
            }

            template <typename type>
            CDBVariant& operator=(const type &src) {
                CComVariant &me = *this;
                me = src;
                if (vt == VT_EMPTY) {
                    vt = VT_NULL;
                }
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(CComVariant &&vtData) {
                *this = (tagVARIANT&&)vtData;
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(tagVARIANT &&vtData) {
                if (this == &vtData) {
                    return *this;
                }
                VARTYPE vtSrc = vtData.vt;
                if ((vtSrc & VT_ARRAY) == VT_ARRAY) {
                    if ((this->vt & VT_ARRAY) == VT_ARRAY) {
                        unsigned short vt = vtSrc;
                        SAFEARRAY *p = this->parray;
                        this->parray = vtData.parray;
                        vtData.parray = p;
                        vtData.vt = this->vt;
                        this->vt = vt;
                    } else if (this->vt == VT_BSTR) {
                        BSTR bstr = this->bstrVal;
                        this->parray = vtData.parray;
                        vtData.bstrVal = bstr;
                        this->vt = vtSrc;
                        vtData.vt = VT_BSTR;
                    } else {
                        this->vt = vtSrc;
                        this->parray = vtData.parray;
                        vtData.vt = VT_NULL;
                    }
                } else if (vtSrc == VT_BSTR) {
                    if (this->vt == VT_BSTR) {
                        BSTR bstr = this->bstrVal;
                        this->bstrVal = vtData.bstrVal;
                        vtData.bstrVal = bstr;
                    } else if ((this->vt & VT_ARRAY) == VT_ARRAY) {
                        SAFEARRAY *p = this->parray;
                        this->bstrVal = vtData.bstrVal;
                        vtData.parray = p;
                        vtData.vt = this->vt;
                        this->vt = VT_BSTR;
                    } else {
                        this->vt = vtSrc;
                        this->bstrVal = vtData.bstrVal;
                        vtData.vt = VT_NULL;
                    }
                } else {
                    ::memcpy(this, &vtData, sizeof (vtData));
                }
                VtExt = vteNormal;
                return *this;
            }

            CDBVariant& operator=(CDBVariant &&vtData) {
                tagVTExt ve = VtExt;
                *this = (tagVARIANT&&)vtData;
                VtExt = vtData.VtExt;
                vtData.VtExt = ve;
                return *this;
            }
        };

        static CUQueue& operator<<(CUQueue &q, const CDBVariant &vt) {
            if (vteGuid == vt.VtExt && (vt.vt == (VT_ARRAY | VT_UI1)) && vt.parray->rgsabound->cElements == sizeof (GUID)) {
                const VARTYPE type = VT_CLSID;
                q.Push((const unsigned char*) &type, sizeof (type));
                unsigned char *buffer;
                ::SafeArrayAccessData(vt.parray, (void**) &buffer);
                q.Push((const unsigned char *) buffer, sizeof (GUID));
                ::SafeArrayUnaccessData(vt.parray);
            }
#ifdef WIN32_64
            else if (vt.vt == VT_DATE && vt.date < MIN_WIN_DATETIME) {
                q << vt.vt << vt.ullVal;
            }
#endif
            else {
                q << (const tagVARIANT&) vt;
            }
            return q;
        }

        static CUQueue& operator>>(CUQueue &q, CDBVariant &vt) {
            tagVTExt vte = vteNormal;
            if (q.GetSize() >= (sizeof (VARTYPE) + sizeof (GUID))) {
                const VARTYPE *pvt = (const VARTYPE *) q.GetBuffer();
                if (VT_CLSID == *pvt) {
                    vte = vteGuid;
                }
            }
#ifdef WIN32_64
            const VARTYPE *pvt = (const VARTYPE *) q.GetBuffer();
            if (*pvt == VT_DATE) {
                const double *dbl = (const double *) q.GetBuffer(sizeof (VARTYPE));
                if (*dbl < MIN_WIN_DATETIME) {
                    //high precision time
                    q >> vt.vt >> vt.ullVal;
                    return q;
                }
            }
#endif
            q >> (tagVARIANT&) vt;
            vt.VtExt = vte;
            return q;
        }

        class CDBColumnInfo {
        public:
            static const unsigned int FLAG_NOT_NULL = 0x1;
            static const unsigned int FLAG_UNIQUE = 0x2;
            static const unsigned int FLAG_PRIMARY_KEY = 0x4;
            static const unsigned int FLAG_AUTOINCREMENT = 0x8;
            static const unsigned int FLAG_NOT_WRITABLE = 0x10;
            static const unsigned int FLAG_ROWID = 0x20;
            static const unsigned int FLAG_XML = 0x40;
            static const unsigned int FLAG_JSON = 0x80;
            static const unsigned int FLAG_CASE_SENSITIVE = 0x100;
            static const unsigned int FLAG_IS_ENUM = 0x200;
            static const unsigned int FLAG_IS_SET = 0x400;
            static const unsigned int FLAG_IS_UNSIGNED = 0x800;
            static const unsigned int FLAG_IS_BIT = 0x1000;

            CDBColumnInfo() : ColumnSize(0), Flags(0), DataType(VT_EMPTY), Precision(0), Scale(0) {
            }

            CDBColumnInfo(const CDBColumnInfo &info)
            : DBPath(info.DBPath),
            TablePath(info.TablePath),
            DisplayName(info.DisplayName),
            OriginalName(info.OriginalName),
            DeclaredType(info.DeclaredType),
            Collation(info.Collation),
            ColumnSize(info.ColumnSize),
            Flags(info.Flags),
            DataType(info.DataType),
            Precision(info.Precision),
            Scale(info.Scale) {

            }

            CDBColumnInfo(CDBColumnInfo &&info)
            : DBPath(std::move(info.DBPath)),
            TablePath(std::move(info.TablePath)),
            DisplayName(std::move(info.DisplayName)),
            OriginalName(std::move(info.OriginalName)),
            DeclaredType(std::move(info.DeclaredType)),
            Collation(std::move(info.Collation)),
            ColumnSize(info.ColumnSize),
            Flags(info.Flags),
            DataType(info.DataType),
            Precision(info.Precision),
            Scale(info.Scale) {
                info.ColumnSize = 0;
                info.Flags = 0;
                info.DataType = VT_EMPTY;
                info.Precision = 0;
                info.Scale = 0;
            }

            CDBColumnInfo& operator=(const CDBColumnInfo &info) {
                if (this == &info) {
                    return *this;
                }
                DBPath = info.DBPath;
                TablePath = info.TablePath;
                DisplayName = info.DisplayName;
                OriginalName = info.OriginalName;
                DeclaredType = info.DeclaredType;
                Collation = info.Collation;
                ColumnSize = info.ColumnSize;
                Flags = info.Flags;
                DataType = info.DataType;
                Precision = info.Precision;
                Scale = info.Scale;
                return *this;
            }

            CDBColumnInfo& operator=(CDBColumnInfo &&info) {
                if (this == &info) {
                    return *this;
                }
                DBPath = std::move(info.DBPath);
                TablePath = std::move(info.TablePath);
                DisplayName = std::move(info.DisplayName);
                OriginalName = std::move(info.OriginalName);
                DeclaredType = std::move(info.DeclaredType);
                Collation = std::move(info.Collation);
                ColumnSize = std::move(info.ColumnSize);
                Flags = std::move(info.Flags);
                DataType = std::move(info.DataType);
                Precision = std::move(info.Precision);
                Scale = std::move(info.Scale);
                return *this;
            }

            bool operator==(const CDBColumnInfo &info) const {
                return (DBPath == info.DBPath &&
                        TablePath == info.TablePath &&
                        DisplayName == info.DisplayName &&
                        OriginalName == info.OriginalName &&
                        DeclaredType == info.DeclaredType &&
                        Collation == info.Collation &&
                        ColumnSize == info.ColumnSize &&
                        Flags == info.Flags &&
                        DataType == info.DataType &&
                        Precision == info.Precision &&
                        Scale == info.Scale);
            }

            bool operator!=(const CDBColumnInfo &info) const {
                return (!(*this == info));
            }

        public:
            std::wstring DBPath;
            std::wstring TablePath;
            std::wstring DisplayName;
            std::wstring OriginalName;
            std::wstring DeclaredType;
            std::wstring Collation;
            unsigned int ColumnSize;
            unsigned int Flags;
            unsigned short DataType;
            unsigned char Precision;
            unsigned char Scale;
        };

        static CUQueue& operator<<(CUQueue &q, const CDBColumnInfo &dbinfo) {
            q << dbinfo.DBPath << dbinfo.TablePath << dbinfo.DisplayName << dbinfo.OriginalName << dbinfo.DeclaredType << dbinfo.Collation <<
                    dbinfo.ColumnSize << dbinfo.Flags << dbinfo.DataType << dbinfo.Precision << dbinfo.Scale;
            return q;
        }

        static CUQueue& operator>>(CUQueue &q, CDBColumnInfo &dbinfo) {
            q >> dbinfo.DBPath >> dbinfo.TablePath >> dbinfo.DisplayName >> dbinfo.OriginalName >> dbinfo.DeclaredType >> dbinfo.Collation >>
                    dbinfo.ColumnSize >> dbinfo.Flags >> dbinfo.DataType >> dbinfo.Precision >> dbinfo.Scale;
            return q;
        }

        typedef std::vector<CDBColumnInfo> CDBColumnInfoArray;

        static CUQueue& operator<<(CUQueue &q, const CDBColumnInfoArray &arr) {
            unsigned int size = (unsigned int) arr.size();
            q << size;
            for (unsigned int n = 0; n < size; ++n) {
                q << arr[n];
            }
            return q;
        }

        static CUQueue& operator>>(CUQueue &q, CDBColumnInfoArray &arr) {
            unsigned int size;
            arr.clear();
            q >> size;
            if (size) {
                CDBColumnInfo c;
                for (unsigned int n = 0; n < size; ++n) {
                    arr.push_back(c);
                    CDBColumnInfo &info = arr.back();
                    q >> info;
                }
            }
            return q;
        }

        typedef std::vector<CDBVariant> CDBVariantArray;

        static CUQueue& operator<<(CUQueue &q, const CDBVariantArray &arr) {
            unsigned int size = (unsigned int) arr.size();
            q << size;
            for (unsigned int n = 0; n < size; ++n) {
                q << arr[n];
            }
            return q;
        }

        static CUQueue& operator>>(CUQueue &q, CDBVariantArray &arr) {
            unsigned int size;
            arr.clear();
            q >> size;
            if (size) {
                CDBVariant v;
                for (unsigned int n = 0; n < size; ++n) {
                    arr.push_back(v);
                    CDBVariant &data = arr.back();
                    q >> data;
                }
            }
            return q;
        }

        enum tagParameterDirection {
            pdUnknown = 0,
            pdInput = 1,
            pdOutput = 2,
            pdInputOutput = 3,
            pdReturnValue = 4
        };

        class CParameterInfo {
        public:

            CParameterInfo()
            : Direction(pdInput), DataType(VT_EMPTY), ColumnSize(0), Precision(0), Scale(0) {
            }

            CParameterInfo(const CParameterInfo &info)
            : Direction(info.Direction),
            DataType(info.DataType),
            ColumnSize(info.ColumnSize),
            Precision(info.Precision),
            Scale(info.Scale),
            ParameterName(info.ParameterName) {

            }

            CParameterInfo& operator=(const CParameterInfo &info) {
                if (this == &info) {
                    return *this;
                }
                Direction = info.Direction;
                DataType = info.DataType;
                ColumnSize = info.ColumnSize;
                Precision = info.Precision;
                Scale = info.Scale;
                ParameterName = info.ParameterName;
                return *this;
            }

            CParameterInfo(CParameterInfo &&info)
            : Direction(info.Direction),
            DataType(info.DataType),
            ColumnSize(info.ColumnSize),
            Precision(info.Precision),
            Scale(info.Scale),
            ParameterName(std::move(info.ParameterName)) {
                info.Direction = pdInput;
                info.DataType = VT_EMPTY;
                info.ColumnSize = 0;
                info.Precision = 0;
                info.Scale = 0;
            }

            CParameterInfo& operator=(CParameterInfo &&info) {
                if (this == &info) {
                    return *this;
                }
                Direction = std::move(info.Direction);
                DataType = std::move(info.DataType);
                ColumnSize = std::move(info.ColumnSize);
                Precision = std::move(info.Precision);
                Scale = std::move(info.Scale);
                ParameterName = std::move(info.ParameterName);
                return *this;
            }

            tagParameterDirection Direction; //required
            VARTYPE DataType; //required! for example, VT_I4, VT_BSTR, VT_I1|VT_ARRAY (UTF8 string), ....
            unsigned int ColumnSize; //-1 BLOB, string length or binary bytes; ignored for other data types
            unsigned char Precision; //datetime, decimal or numeric only
            unsigned char Scale; //datetime, decimal or numeric only
            std::wstring ParameterName; //may be optional, which depends on remote database system
        };

        static CUQueue& operator<<(CUQueue &q, const CParameterInfo &info) {
            q << (int) info.Direction << info.DataType << info.ColumnSize <<
                    info.Precision << info.Scale << info.ParameterName;
            return q;
        }

        static CUQueue& operator>>(CUQueue &q, CParameterInfo &info) {
            int direction;
            q >> direction;
            info.Direction = (tagParameterDirection) direction;
            q >> info.DataType >> info.ColumnSize >> info.Precision >>
                    info.Scale >> info.ParameterName;
            return q;
        }

        typedef std::vector<CParameterInfo> CParameterInfoArray;

        static CUQueue& operator<<(CUQueue &q, const CParameterInfoArray &arr) {
            unsigned int size = (unsigned int) arr.size();
            q << size;
            for (unsigned int n = 0; n < size; ++n) {
                q << arr[n];
            }
            return q;
        }

        static CUQueue& operator>>(CUQueue &q, CParameterInfoArray &arr) {
            unsigned int size;
            arr.clear();
            q >> size;
            if (size) {
                CParameterInfo p;
                for (unsigned int n = 0; n < size; ++n) {
                    arr.push_back(p);
                    CParameterInfo &data = arr.back();
                    q >> data;
                }
            }
            return q;
        }
    } //UDB
} //namespace SPA

#endif //__UDAPARTS_DATABASE_COMM_H___
