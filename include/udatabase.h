

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
         * VARINAT data types for OLEDB
         */
        static const unsigned short VT_BYTES = 128; //OLEDB data type for binary array
        static const unsigned short VT_STR = 129; //OLEDB data type for ASCII string
        static const unsigned short VT_WSTR = 130; //OLEDB data type for unicode string

        class CDBVariant : public CComVariant {
        public:

            CDBVariant(CDBVariant &&vtData) {
                CDBVariant &me = *this;
                me = (tagVARIANT&&)vtData;
            }

            CDBVariant(CComVariant &&vtData) {
                CDBVariant &me = *this;
                me = (tagVARIANT&&)vtData;
            }

            CDBVariant(tagVARIANT &&vtData) {
                CDBVariant &me = *this;
                me = vtData;
            }

            CDBVariant(const CDBVariant &vt) : CComVariant(vt) {
            }

            CDBVariant(const CComVariant &vt) : CComVariant(vt) {
            }

            CDBVariant(const tagVARIANT &vt) : CComVariant(vt) {
            }

            CDBVariant() {
                vt = VT_NULL;
            }

            CDBVariant(const GUID &uuid) {
                ::memcpy(&decVal, &uuid, sizeof (uuid));
                vt = VT_CLSID;
            }

            template <typename type>
            CDBVariant(const type& src) : CComVariant(src) {
            }

            CDBVariant(const DECIMAL& src) {
                decVal = src;
                vt = VT_DECIMAL;
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
            }

            CDBVariant(const UDateTime &dt) : CComVariant(dt.time) {
                vt = VT_DATE;
            }

            CDBVariant(const std::tm &st, unsigned int us = 0) : CComVariant(UDateTime(st, us).time) {
                vt = VT_DATE;
            }

#ifdef WIN32_64

            CDBVariant(CComBSTR &&bstr) {
                if (bstr.m_str) {
                    vt = VT_BSTR;
                    bstrVal = bstr.Detach();
                } else {
                    vt = VT_NULL;
                }
            }

            CDBVariant(double dblSrc, VARTYPE vtSrc = VT_R8/* or VT_DATE*/, unsigned short us = 0) : CComVariant(dblSrc, vtSrc) {
                if (vtSrc == VT_DATE) {
                    //convert variant date to high precision time on window system
                    UDateTime udt(dblSrc, us);
                    this->ullVal = udt.time;
                }
            }

            CDBVariant(const SYSTEMTIME &st, unsigned short us = 0) : CComVariant(UDateTime(st, us).time) {
                vt = VT_DATE;
            }
#else

            CDBVariant(const SYSTEMTIME &st) {
                vt = VT_DATE;
                ullVal = UDateTime(st).time;
            }
#endif
        public:

            unsigned short Type() const {
                return vt;
            }

            CDBVariant& operator=(const CDBVariant &vtData) {
                CComVariant &me = *this;
                me = vtData;
                return *this;
            }

            CDBVariant& operator=(const CComVariant &vtData) {
                CComVariant &me = *this;
                me = vtData;
                return *this;
            }

            CDBVariant& operator=(const tagVARIANT &vtData) {
                CComVariant &me = *this;
                me = vtData;
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
                return *this;
            }

            CDBVariant& operator=(const wchar_t* lpszSrc) {
                CComVariant &me = *this;
                me = lpszSrc;
                return *this;
            }

            CDBVariant& operator=(const GUID &uuid) {
                ::VariantClear(this);
                ::memcpy(&decVal, &uuid, sizeof (uuid));
                vt = VT_CLSID;
                return *this;
            }

            CDBVariant& operator=(const DECIMAL& src) {
                ::VariantClear(this);
                decVal = src;
                vt = VT_DECIMAL;
            }

            CDBVariant& operator=(const UDateTime &dt) {
                ::VariantClear(this);
                vt = VT_DATE;
                this->ullVal = dt.time;
                return *this;
            }

            CDBVariant& operator=(const SYSTEMTIME &st) {
                ::VariantClear(this);
                vt = VT_DATE;
                ullVal = UDateTime(st).time;
                return *this;
            }

            CDBVariant& operator=(const std::tm &st) {
                ::VariantClear(this);
                vt = VT_DATE;
                ullVal = UDateTime(st).time;
                return *this;
            }

            template <typename type>
            CDBVariant& operator=(const type &src) {
                CComVariant &me = *this;
                me = src;
                if (this->vt == VT_EMPTY) {
                    this->vt = VT_NULL;
                }
                return *this;
            }

            CDBVariant& operator=(CComVariant &&vtData) {
                *this = (tagVARIANT&&)vtData;
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
                return *this;
            }

            CDBVariant& operator=(CDBVariant &&vtData) {
                *this = (tagVARIANT&&)vtData;
                return *this;
            }
        };

        static CUQueue& operator<<(CUQueue &q, const CDBVariant &vt) {
            q << (const tagVARIANT&) vt;
            return q;
        }

        static CUQueue& operator>>(CUQueue &q, CDBVariant &vt) {
            q >> (tagVARIANT&) vt;
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
            unsigned int ColumnSize; //-1 BLOB, string len or binary bytes; ignored for other data types
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
