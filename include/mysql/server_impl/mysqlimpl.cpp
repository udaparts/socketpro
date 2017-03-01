
#include "mysqlimpl.h"
#include <algorithm>
#include <sstream>
#ifndef NDEBUG
#include <iostream>
#endif

namespace SPA
{
    namespace ServerSide{

        CUCriticalSection CMysqlImpl::m_csPeer;
        my_bool CMysqlImpl::B_IS_NULL = 1;
        std::wstring CMysqlImpl::m_strGlobalConnection;
        std::wstring CMysqlImpl::m_strGlobalDB;
        std::string CMysqlImpl::m_strOptions = "basedir=.;\ndatadir=data/;\nplugin_dir=./plugin;\ndefault-storage-engine=MyISAM;\ndefault-tmp-storage-engine=MyISAM;\ncharacter-set-server=utf8;\ninnodb=OFF;\nconsole;\n"
        "key_buffer_size=64M;\n"
        "read_buffer_size=128K;\n"
        "loose-innodb-trx=0;\n"
        "loose-innodb-locks=0;\n"
        "loose-innodb-lock-waits=0;\n"
        "loose-innodb-cmp=0;\n"
        "loose-innodb-cmp-per-index=0;\n"
        "loose-innodb-cmp-per-index-reset=0;\n"
        "loose-innodb-cmp-reset=0;\n"
        "loose-innodb-cmpmem=0;\n"
        "loose-innodb-cmpmem-reset=0;\n"
        "loose-innodb-buffer-page=0;\n"
        "loose-innodb-buffer-page-lru=0;\n"
        "loose-innodb-buffer-pool-stats=0;\n"
        "loose-innodb-metrics=0;\n"
        "loose-innodb-ft-default-stopword=0;\n"
        "loose-innodb-ft-inserted=0;\n"
        "loose-innodb-ft-deleted=0;\n"
        "loose-innodb-ft-being-deleted=0;\n"
        "loose-innodb-ft-config=0;\n"
        "loose-innodb-ft-index-cache=0;\n"
        "loose-innodb-ft-index-table=0;\n"
        "loose-innodb-sys-tables=0;\n"
        "loose-innodb-sys-tablestats=0;\n"
        "loose-innodb-sys-indexes=0;\n"
        "loose-innodb-sys-columns=0;\n"
        "loose-innodb-sys-fields=0;\n"
        "loose-innodb-sys-foreign=0;\n"
        "loose-innodb-sys-foreign-cols=0";

        bool CMysqlImpl::m_bInitMysql = false;
        bool CMysqlImpl::m_bInitEmbeddedMysql = false;

        const wchar_t * CMysqlImpl::NO_DB_OPENED_YET = L"No mysql database opened yet";
        const wchar_t * CMysqlImpl::BAD_END_TRANSTACTION_PLAN = L"Bad end transaction plan";
        const wchar_t * CMysqlImpl::NO_PARAMETER_SPECIFIED = L"No parameter specified";
        const wchar_t * CMysqlImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = L"Bad parameter data array length";
        const wchar_t * CMysqlImpl::BAD_PARAMETER_COLUMN_SIZE = L"Bad parameter column size";
        const wchar_t * CMysqlImpl::DATA_TYPE_NOT_SUPPORTED = L"Data type not supported";
        const wchar_t * CMysqlImpl::NO_DB_NAME_SPECIFIED = L"No mysql database name specified";
        const wchar_t * CMysqlImpl::MYSQL_LIBRARY_NOT_INITIALIZED = L"Mysql library not initialized";
        const wchar_t * CMysqlImpl::BAD_MANUAL_TRANSACTION_STATE = L"Bad manual transaction state";

        const wchar_t * CMysqlImpl::MYSQL_GLOBAL_CONNECTION_STRING = L"MYSQL_GLOBAL_CONNECTION_STRING";

        unsigned int CMysqlImpl::m_nParam = 0;

        CMysqlLoader CMysqlImpl::m_remMysql;
        CMysqlLoader CMysqlImpl::m_embMysql;

        void CMysqlImpl::MYSQL_CONNECTION_STRING::Init() {
            timeout = 10;
            port = 3306;
            database.clear();
            host.clear();
            password.clear();
            ssl_ca.clear();
            ssl_capath.clear();
            ssl_cert.clear();
            ssl_cipher.clear();
            ssl_key.clear();
            user.clear();
        }

        std::vector<std::string> CMysqlImpl::ParseOptions() {
            using namespace std;
            m_csPeer.lock();
            stringstream ss(m_strOptions);
            m_csPeer.unlock();
            string item;
            vector<string> tokens;
            while (getline(ss, item, ';')) {
                CMysqlImpl::MYSQL_CONNECTION_STRING::Trim(item);
                if (!item.size()) {
                    continue;
                }
                item.insert(0, "--");
                tokens.push_back(item);
            }
            return tokens;
        }

        void CMysqlImpl::MYSQL_CONNECTION_STRING::Parse(const char *s) {
            using namespace std;
            Init();
            if (!strstr(s, "="))
                return;
            stringstream ss(s ? s : "");
            string item;
            vector<string> tokens;
            while (getline(ss, item, ';')) {
                tokens.push_back(item);
            }
            for (auto it = tokens.begin(), end = tokens.end(); it != end; ++it) {
                auto pos = it->find('=');
                if (pos == string::npos)
                    continue;
                string left = it->substr(0, pos);
                string right = it->substr(pos + 1);
                Trim(left);
                Trim(right);
                transform(left.begin(), left.end(), left.begin(), ::tolower);
                if (left == "connect-timeout" || left == "timeout" || left == "connection-timeout")
                    timeout = (unsigned int) std::atoi(right.c_str());
                else if (left == "database" || left == "db")
                    database = right;
                else if (left == "port")
                    port = (unsigned int) std::atoi(right.c_str());
                else if (left == "pwd" || left == "password")
                    password = right;
                else if (left == "host" || left == "server")
                    host = right;
                else if (left == "user" || left == "uid")
                    user = right;
                else if (left == "ssl-ca")
                    ssl_ca = right;
                else if (left == "ssl-capath")
                    ssl_capath = right;
                else if (left == "ssl-cert")
                    ssl_cert = right;
                else if (left == "ssl-cipher")
                    ssl_cipher = right;
                else if (left == "ssl-key")
                    ssl_key = right;
                else {
                    //!!! not implemented
                    assert(false);
                }
            }
        }

        void CMysqlImpl::MYSQL_CONNECTION_STRING::Trim(std::string & s) {
            static const char *WHITESPACE = " \r\n\t\v\f\v";
            auto pos = s.find_first_of(WHITESPACE);
            while (pos == 0) {
                s.erase(s.begin());
                pos = s.find_first_of(WHITESPACE);
            }
            pos = s.find_last_of(WHITESPACE);
            while (s.size() && pos == s.size() - 1) {
                s.pop_back();
                pos = s.find_last_of(WHITESPACE);
            }
        }

        CMysqlImpl::CMysqlImpl() : m_oks(0), m_fails(0), m_ti(tiUnspecified), m_pLib(nullptr), m_global(true), m_Blob(*m_sb), m_parameters(0), m_bCall(false) {
            m_Blob.ToUtf8(true);
#ifdef WIN32_64
            m_UQueue.TimeEx(true); //use high-precision datetime
#endif
            m_UQueue.ToUtf8(true);
        }

        unsigned int CMysqlImpl::GetParameters() const {
            return (unsigned int) m_parameters;
        }

        MYSQL * CMysqlImpl::GetDBHandle() const {
            return m_pMysql.get();
        }

        MYSQL_STMT * CMysqlImpl::GetPreparedStatement() const {
            return m_pPrepare.get();
        }

        bool CMysqlImpl::IsGloballyConnected() const {
            return m_global;
        }

        bool CMysqlImpl::IsStoredProcedure() const {
            return m_bCall;
        }

        const std::string & CMysqlImpl::GetProcedureName() const {
            return m_procName;
        }

        CMysqlLoader * CMysqlImpl::GetLib() const {
            return m_pLib;
        }

        void CMysqlImpl::UnloadMysql() {
            SPA::CAutoLock al(m_csPeer);
            m_remMysql.Unload();
            m_embMysql.Unload();
        }

        bool CMysqlImpl::InitEmbeddedMySql() {
            if ((m_nParam & Mysql::DISABLE_EMBEDDED_MYSQL) == Mysql::DISABLE_EMBEDDED_MYSQL) {
                return false;
            }
            SPA::CAutoLock al(m_csPeer);
            if (m_bInitEmbeddedMysql) {
                return true;
            }
            if (m_embMysql.LoadMysql(false)) {
                std::vector<std::string> options = ParseOptions();
                std::shared_ptr<char*> pOptions(new char*[options.size() + 1], [](char **p) {
                    delete []p;
                });
                char **pp = pOptions.get();
                pp[0] = (char*) "umysql";
                int index = 1;
                for (auto it = options.begin(), end = options.end(); it != end; ++it, ++index) {
                    pp[index] = (char*) it->c_str();
                }
                static char *groups[] = {
                    (char*) "umysql",
                    nullptr
                };
                m_bInitEmbeddedMysql = (m_embMysql.mysql_server_init((int) (options.size() + 1), pp, (char **) groups) == 0);
                if (m_bInitEmbeddedMysql) {
                    ServerCoreLoader.SetThreadEvent(OnThreadEventEmbedded);
                }
            }
            return m_bInitEmbeddedMysql;
        }

        void CALLBACK CMysqlImpl::OnThreadEventEmbedded(SPA::ServerSide::tagThreadEvent te) {
            if (te == SPA::ServerSide::teStarted) {
                my_bool fail = m_embMysql.mysql_thread_init();
                assert(!fail);
            } else {
                m_embMysql.mysql_thread_end();
            }
        }

        void CALLBACK CMysqlImpl::OnThreadEvent(SPA::ServerSide::tagThreadEvent te) {
            if (te == SPA::ServerSide::teStarted) {
                my_bool fail = m_remMysql.mysql_thread_init();
                assert(!fail);
            } else {
                m_remMysql.mysql_thread_end();
            }
        }

        bool CMysqlImpl::InitMySql() {
            if ((m_nParam & Mysql::DISABLE_REMOTE_MYSQL) == Mysql::DISABLE_REMOTE_MYSQL) {
                return false;
            }
            SPA::CAutoLock al(m_csPeer);
            if (m_bInitMysql) {
                return true;
            }
            if (m_remMysql.LoadMysql(true)) {
                m_bInitMysql = (m_remMysql.mysql_server_init(0, nullptr, nullptr) == 0);
                /*
                if (m_bInitMysql) {
                    ServerCoreLoader.SetThreadEvent(OnThreadEvent);
                }
                 */
            }
            return m_bInitMysql;
        }

        void CMysqlImpl::ReleaseArray() {
            for (auto it = m_vArray.begin(), end = m_vArray.end(); it != end; ++it) {
                SAFEARRAY *arr = *it;
                ::SafeArrayUnaccessData(arr);
            }
            m_vArray.clear();
        }

        void CMysqlImpl::SetDBGlobalConnectionString(const wchar_t *dbConnection, bool remote) {
            m_csPeer.lock();
            if (remote) {
                if (dbConnection) {
                    m_strGlobalConnection = dbConnection;
                } else {
                    m_strGlobalConnection.clear();
                }
            } else {
                if (dbConnection) {
                    m_strGlobalDB = dbConnection;
                } else {
                    m_strGlobalDB.clear();
                }
            }
            m_csPeer.unlock();
        }

        const char* CMysqlImpl::SetEmbeddedOptions(const wchar_t * options) {
            std::string s = SPA::Utilities::ToUTF8(options);
            SPA::CAutoLock al(m_csPeer);
            if (s.size()) {
                m_strOptions = s;
            }
            return m_strOptions.c_str();
        }

        void CMysqlImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            CleanDBObjects();
            m_Blob.SetSize(0);
            if (m_Blob.GetMaxSize() > 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                m_Blob.ReallocBuffer(2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
            }
            m_global = true;
            m_pLib = nullptr;
            MYSQL_BIND_RESULT_FIELD::ShrinkMemoryPool();
        }

        void CMysqlImpl::OnSwitchFrom(unsigned int nOldServiceId) {
            m_oks = 0;
            m_fails = 0;
            m_ti = tiUnspecified;
        }

        void CMysqlImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I1_R0(idStartBLOB, StartBLOB, unsigned int)
            M_I0_R0(idChunk, Chunk)
            M_I0_R0(idEndBLOB, EndBLOB)
            M_I0_R0(idBeginRows, BeginRows)
            M_I0_R0(idTransferring, Transferring)
            M_I0_R0(idEndRows, EndRows)
            M_I0_R2(idClose, CloseDb, int, std::wstring)
            M_I2_R3(idOpen, Open, std::wstring, unsigned int, int, std::wstring, int)
            M_I3_R3(idBeginTrans, BeginTrans, int, std::wstring, unsigned int, int, std::wstring, int)
            M_I1_R2(idEndTrans, EndTrans, int, int, std::wstring)
            M_I5_R5(idExecute, Execute, std::wstring, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            M_I2_R3(idPrepare, Prepare, std::wstring, CParameterInfoArray, int, std::wstring, unsigned int)
            M_I4_R5(idExecuteParameters, ExecuteParameters, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            END_SWITCH
            if (reqId == idExecuteParameters) {
                ReleaseArray();
                m_vParam.clear();
            }
        }

        int CMysqlImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I0_R2(idClose, CloseDb, int, std::wstring)
            M_I2_R3(idOpen, Open, std::wstring, unsigned int, int, std::wstring, int)
            M_I3_R3(idBeginTrans, BeginTrans, int, std::wstring, unsigned int, int, std::wstring, int)
            M_I1_R2(idEndTrans, EndTrans, int, int, std::wstring)
            M_I5_R5(idExecute, Execute, std::wstring, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            M_I2_R3(idPrepare, Prepare, std::wstring, CParameterInfoArray, int, std::wstring, unsigned int)
            M_I4_R5(idExecuteParameters, ExecuteParameters, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            END_SWITCH
            if (reqId == idExecuteParameters) {
                ReleaseArray();
                m_vParam.clear();
            }
            return 0;
        }

        void CMysqlImpl::Open(const std::wstring &strConnection, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            ms = msMysql;
            CleanDBObjects();
            bool remote = (SPA::Mysql::USE_REMOTE_MYSQL == (flags & SPA::Mysql::USE_REMOTE_MYSQL));
            if (remote) {
                if (!InitMySql()) {
                    res = SPA::Mysql::ER_MYSQL_LIBRARY_NOT_INITIALIZED;
                    errMsg = MYSQL_LIBRARY_NOT_INITIALIZED;
                    return;
                }
                m_pLib = &m_remMysql;
            } else {
                if (!InitEmbeddedMySql()) {
                    res = SPA::Mysql::ER_MYSQL_LIBRARY_NOT_INITIALIZED;
                    errMsg = MYSQL_LIBRARY_NOT_INITIALIZED;
                    return;
                }
                m_pLib = &m_embMysql;
            }
            MYSQL *mysql = m_pLib->mysql_init(nullptr);
            do {
                std::wstring db(strConnection);
                if (!db.size() || db == MYSQL_GLOBAL_CONNECTION_STRING) {
                    m_csPeer.lock();
                    if (remote) {
                        db = m_strGlobalConnection;
                    } else {
                        db = m_strGlobalDB;
                    }
                    m_csPeer.unlock();
                    m_global = true;
                } else {
                    m_global = false;
                }
                m_pLib->mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
                if (remote) {
                    MYSQL_CONNECTION_STRING conn;
                    conn.Parse(Utilities::ToUTF8(db.c_str()).c_str());
                    m_pLib->mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &conn.timeout);
                    if (conn.IsSSL()) {
#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID < 50700
#define MYSQL_OPT_SSL_KEY ((mysql_option) 25)
#define MYSQL_OPT_SSL_CERT ((mysql_option) 26)
#define MYSQL_OPT_SSL_CA ((mysql_option) 27)
#define MYSQL_OPT_SSL_CAPATH ((mysql_option) 28)
#define MYSQL_OPT_SSL_CIPHER ((mysql_option) 29)
#endif
                        if (m_pLib->mysql_get_client_version() > 50700) {
                            if (conn.ssl_ca.size())
                                m_pLib->mysql_options(mysql, MYSQL_OPT_SSL_CA, conn.ssl_ca.c_str());
                            if (conn.ssl_capath.size())
                                m_pLib->mysql_options(mysql, MYSQL_OPT_SSL_CAPATH, conn.ssl_capath.c_str());
                            if (conn.ssl_cert.size())
                                m_pLib->mysql_options(mysql, MYSQL_OPT_SSL_CERT, conn.ssl_cert.c_str());
                            if (conn.ssl_cipher.size())
                                m_pLib->mysql_options(mysql, MYSQL_OPT_SSL_CIPHER, conn.ssl_cipher.c_str());
                            if (conn.ssl_key.size())
                                m_pLib->mysql_options(mysql, MYSQL_OPT_SSL_KEY, conn.ssl_key.c_str());
                        } else {
                            my_bool ssl_enabled = 1;
                            m_pLib->mysql_options(mysql, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_enabled);
                        }
                    }
                    MYSQL *ret = m_pLib->mysql_real_connect(mysql, conn.host.c_str(), conn.user.c_str(),
                            conn.password.c_str(), conn.database.c_str(),
                            conn.port, nullptr, CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS | CLIENT_LOCAL_FILES | CLIENT_IGNORE_SIGPIPE);
                    if (!ret) {
                        res = m_pLib->mysql_errno(mysql);
                        errMsg = Utilities::ToWide(m_pLib->mysql_error(mysql));
                        break;
                    } else {
                        res = 0;
                    }
                } else {
                    res = m_pLib->mysql_options(mysql, MYSQL_OPT_USE_EMBEDDED_CONNECTION, nullptr);
                    if (res) {
                        res = m_pLib->mysql_errno(mysql);
                        errMsg = SPA::Utilities::ToWide(m_pLib->mysql_error(mysql));
                        break;
                    }
                    MYSQL *ret = m_pLib->mysql_real_connect(mysql, nullptr, nullptr, nullptr,
                            SPA::Utilities::ToUTF8(db.c_str()).c_str(), 0, nullptr,
                            CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS | CLIENT_LOCAL_FILES | CLIENT_IGNORE_SIGPIPE);
                    if (!ret) {
                        res = m_pLib->mysql_errno(mysql);
                        errMsg = Utilities::ToWide(m_pLib->mysql_error(mysql));
                        break;
                    }
                }
                if (!m_global) {
                    errMsg = db;
                } else {
                    errMsg = MYSQL_GLOBAL_CONNECTION_STRING;
                }
            } while (false);
            if (!res) {
                m_pMysql.reset(mysql, [this](MYSQL * mysql) {
                    if (mysql) {
                        this->m_pLib->mysql_close(mysql);
                    }
                });
            } else if (mysql) {
                m_pLib->mysql_close(mysql);
            }
        }

        void CMysqlImpl::CloseDb(int &res, std::wstring & errMsg) {
            CleanDBObjects();
            res = 0;
            m_pLib = nullptr;
        }

        void CMysqlImpl::CleanDBObjects() {
            m_pPrepare.reset();
            m_pMysql.reset();
            m_vParam.clear();
            m_parameters = 0;
        }

		void CMysqlImpl::OnBaseRequestArrive(unsigned short requestId) {
			switch(requestId) {
			case idCancel:
#ifndef NDEBUG
				std::cout << "Cancel called" << std::endl;
#endif
				{
					MYSQL *mysql = m_pMysql.get();
					if (mysql) {
						unsigned long id = m_pLib->mysql_thread_id(mysql);
						std::string sqlKill = "KILL QUERY " + std::to_string((UINT64)id);
						int status = m_pLib->mysql_real_query(mysql, sqlKill.c_str(), (unsigned long)sqlKill.size());
						status = m_pLib->mysql_rollback(mysql);
					}
				}
				break;
			default:
				break;
			}
		}

        void CMysqlImpl::BeginTrans(int isolation, const std::wstring &dbConn, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            ms = msMysql;
            if (m_ti != tiUnspecified || isolation == (int) tiUnspecified) {
                errMsg = BAD_MANUAL_TRANSACTION_STATE;
                res = SPA::Mysql::ER_BAD_MANUAL_TRANSACTION_STATE;
                return;
            }
            if (!m_pMysql) {
                Open(dbConn, flags, res, errMsg, ms);
                if (!m_pMysql) {
                    return;
                }
            } else {
                std::string sql;
                switch ((tagTransactionIsolation) isolation) {
                    case tiReadUncommited:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL READ UNCOMMITTED";
                        break;
                    case tiRepeatableRead:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ";
                        break;
                    case tiReadCommited:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED";
                        break;
                    case tiSerializable:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE";
                        break;
                    default:
                        break;
                }
                if (sql.size()) {
                    int status = m_pLib->mysql_real_query(m_pMysql.get(), sql.c_str(), (unsigned long) sql.size());
                    if (status) {
                        res = m_pLib->mysql_errno(m_pMysql.get());
                        errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
                        return;
                    }
                } else {
                    //ignored for unsupported isolation level
                }
                my_bool fail = m_pLib->mysql_autocommit(m_pMysql.get(), 0);
                if (!fail) {
                    res = 0;
                    m_fails = 0;
                    m_oks = 0;
                    m_ti = (tagTransactionIsolation) isolation;
                    if (!m_global) {
                        errMsg = dbConn;
                    } else {
                        errMsg = MYSQL_GLOBAL_CONNECTION_STRING;
                    }
                } else {
                    res = m_pLib->mysql_errno(m_pMysql.get());
                    errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
                }
            }
        }

        void CMysqlImpl::EndTrans(int plan, int &res, std::wstring & errMsg) {
            if (m_ti == tiUnspecified) {
                errMsg = BAD_MANUAL_TRANSACTION_STATE;
                res = SPA::Mysql::ER_BAD_MANUAL_TRANSACTION_STATE;
                return;
            }
            if (plan < 0 || plan > rpRollbackAlways) {
                res = SPA::Mysql::ER_BAD_END_TRANSTACTION_PLAN;
                errMsg = BAD_END_TRANSTACTION_PLAN;
                return;
            }
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            }
            bool rollback = false;
            tagRollbackPlan rp = (tagRollbackPlan) plan;
            switch (rp) {
                case rpRollbackErrorAny:
                    rollback = m_fails ? true : false;
                    break;
                case rpRollbackErrorLess:
                    rollback = (m_fails < m_oks && m_fails) ? true : false;
                    break;
                case rpRollbackErrorEqual:
                    rollback = (m_fails >= m_oks) ? true : false;
                    break;
                case rpRollbackErrorMore:
                    rollback = (m_fails > m_oks) ? true : false;
                    break;
                case rpRollbackErrorAll:
                    rollback = (m_oks) ? false : true;
                    break;
                case rpRollbackAlways:
                    rollback = true;
                    break;
                default:
                    assert(false); //shouldn't come here
                    break;
            }
            my_bool fail;
            if (rollback) {
                fail = m_pLib->mysql_rollback(m_pMysql.get());
            } else {
                fail = m_pLib->mysql_commit(m_pMysql.get());
            }
            if (fail) {
                res = m_pLib->mysql_errno(m_pMysql.get());
                errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
            } else {
                res = 0;
                m_ti = tiUnspecified;
                m_fails = 0;
                m_oks = 0;
            }
        }

        void CMysqlImpl::ExecuteSqlWithoutRowset(int &res, std::wstring &errMsg, INT64 & affected) {
            do {
                MYSQL_RES *result = m_pLib->mysql_use_result(m_pMysql.get());
                if (result) {
                    m_pLib->mysql_free_result(result);
                    ++m_oks;

                    //For SELECT statements, mysql_affected_rows() works like mysql_num_rows().
                    //affected += (INT64)m_pLib->mysql_affected_rows(m_pMysql.get());
                } else {
                    int errCode = m_pLib->mysql_errno(m_pMysql.get());
                    if (!errCode) {
                        ++m_oks;
                        affected += (INT64) m_pLib->mysql_affected_rows(m_pMysql.get());
                    } else {
                        ++m_fails;
                        if (!res) {
                            res = errCode;
                            errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
                        }
                    }
                }
                int status = m_pLib->mysql_next_result(m_pMysql.get());
                if (status == -1) {
                    break; //Successful and there are no more results
                } else if (status > 0) {
                    ++m_fails;
                    if (!res) {
                        res = m_pLib->mysql_errno(m_pMysql.get());
                        errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
                    }
                    break;
                } else if (status == 0) {
                    //Successful and there are more results
                } else {
                    assert(false); //never come here
                }
            } while (true);
        }

        CDBColumnInfoArray CMysqlImpl::GetColInfo(MYSQL_RES *result, unsigned int cols, bool prepare) {
            CDBColumnInfoArray vCols;
            MYSQL_FIELD *field = m_pLib->mysql_fetch_fields(result);
            for (unsigned int n = 0; n < cols; ++n) {
                vCols.push_back(CDBColumnInfo());
                CDBColumnInfo &info = vCols.back();
                MYSQL_FIELD &f = field[n];
                info.DisplayName.assign(f.name, f.name + f.name_length);
                if (f.org_name && f.org_name_length) {
                    info.OriginalName.assign(f.org_name, f.org_name + f.org_name_length);
                } else {
                    info.OriginalName = info.DisplayName;
                }

                if (f.table && f.table_length) {
                    info.TablePath.assign(f.table, f.table + f.table_length);
                } else if (f.org_table && f.org_table_length) {
                    info.TablePath.assign(f.org_table, f.org_table + f.org_table_length);
                } else {
                    info.TablePath.clear();
                }
                if (f.db && f.db_length) {
                    info.DBPath.assign(f.db, f.db + f.db_length);
                } else {
                    info.DBPath.clear();
                }

                if (f.org_table && f.org_table_length && (f.org_table != f.table)) {
                    info.Collation.assign(f.org_table, f.org_table + f.org_table_length);
                } else if (f.catalog && f.catalog_length) {
                    info.Collation.assign(f.catalog, f.catalog + f.catalog_length);
                } else if (f.def && f.def_length) {
                    info.Collation.assign(f.def, f.def + f.def_length);
                } else {
                    info.Collation.clear();
                }
                if ((f.flags & NOT_NULL_FLAG) == NOT_NULL_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                }
                if ((f.flags & PRI_KEY_FLAG) == PRI_KEY_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
                }
                if ((f.flags & UNIQUE_KEY_FLAG) == UNIQUE_KEY_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_UNIQUE;
                }
                if ((f.flags & AUTO_INCREMENT_FLAG) == AUTO_INCREMENT_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
                    info.Flags |= CDBColumnInfo::FLAG_NOT_WRITABLE;
#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID > 50700
                } else if (f.type == MYSQL_TYPE_JSON) {
                    info.Flags |= CDBColumnInfo::FLAG_JSON;
#endif
                }
                if ((f.flags & ENUM_FLAG) == ENUM_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_IS_ENUM;
                } else if ((f.flags & SET_FLAG) == SET_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_IS_SET;
                }

                if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_IS_UNSIGNED;
                }

                switch (f.type) {
                    case MYSQL_TYPE_NEWDECIMAL:
                    case MYSQL_TYPE_DECIMAL:
                        info.DeclaredType = L"DECIMAL"; //The maximum number of digits for DECIMAL is 65, but the actual range for a given DECIMAL column can be constrained by the precision or scale for a given column.
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        info.Scale = (unsigned char) f.decimals;
                        info.Precision = (unsigned char) f.length;
                        break;
                    case MYSQL_TYPE_TINY:
                        info.DeclaredType = L"TINYINT";
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI1;
                        else
                            info.DataType = VT_I1;
                        break;
                    case MYSQL_TYPE_SHORT:
                        info.DeclaredType = L"SMALLINT";
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI2;
                        else
                            info.DataType = VT_I2;
                        break;
                    case MYSQL_TYPE_LONG:
                        info.DeclaredType = L"INT";
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI4;
                        else
                            info.DataType = VT_I4;
                        break;
                    case MYSQL_TYPE_FLOAT:
                        info.DeclaredType = L"FLOAT";
                        info.DataType = VT_R4;
                        break;
                    case MYSQL_TYPE_DOUBLE:
                        info.DeclaredType = L"DOUBLE";
                        info.DataType = VT_R8;
                        break;
                    case MYSQL_TYPE_TIMESTAMP:
                        info.DeclaredType = L"TIMESTAMP";
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_LONGLONG:
                        info.DeclaredType = L"BIGINT";
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI8;
                        else
                            info.DataType = VT_I8;
                        break;
                    case MYSQL_TYPE_INT24:
                        info.DeclaredType = L"MEDIUMINT";
                        info.DataType = VT_I4;
                        break;
                    case MYSQL_TYPE_DATE:
                        info.DeclaredType = L"DATE";
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_TIME:
                        info.DeclaredType = L"TIME";
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_DATETIME: //#define DATETIME_MAX_DECIMALS 6
                        info.DeclaredType = L"DATETIME";
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_YEAR:
                        info.DeclaredType = L"YEAR";
                        info.DataType = VT_I2;
                        break;
                    case MYSQL_TYPE_NEWDATE:
                        info.DeclaredType = L"NEWDATE";
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_VARCHAR:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            info.DeclaredType = L"VARBINARY";
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            info.DeclaredType = L"VARCHAR";
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        }
                        break;
                    case MYSQL_TYPE_BIT:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"BIT";
                        info.Flags |= CDBColumnInfo::FLAG_IS_BIT;
                        if (f.length == 1)
                            info.DataType = VT_BOOL;
                        else if (f.length <= 8)
                            info.DataType = VT_UI1;
                        else if (f.length <= 16)
                            info.DataType = VT_UI2;
                        else if (f.length <= 32)
                            info.DataType = VT_UI4;
                        else if (f.length <= 64)
                            info.DataType = VT_UI8;
                        else {
                            assert(false); //not implemented
                        }
                        break;
#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID > 50700
                    case MYSQL_TYPE_TIMESTAMP2:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"TIMESTAMP2";
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_DATETIME2:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"DATETIME2";
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_TIME2:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"TIME2";
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_JSON:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"JSON";
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        break;
#endif
                    case MYSQL_TYPE_ENUM:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"ENUM";
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        break;
                    case MYSQL_TYPE_SET:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"SET";
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        break;
                    case MYSQL_TYPE_TINY_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            info.DeclaredType = L"TINYBLOB";
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            info.DeclaredType = L"TINYTEXT";
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_MEDIUM_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            info.DeclaredType = L"MEDIUMBLOB";
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            info.DeclaredType = L"MEDIUMTEXT";
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_LONG_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            info.DeclaredType = L"LONGBLOB";
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            info.DeclaredType = L"LONGTEXT";
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            if (f.length == MYSQL_TINYBLOB) {
                                info.DeclaredType = L"TINYBLOB";
                            } else if (f.length == MYSQL_MIDBLOB) {
                                info.DeclaredType = L"MEDIUMBLOB";
                            } else if (f.length == MYSQL_BLOB) {
                                info.DeclaredType = L"BLOB";
                            } else {
                                info.DeclaredType = L"LONGBLOB";
                            }
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            if (f.length == MYSQL_TINYBLOB) {
                                info.DeclaredType = L"TINYTEXT";
                            } else if (f.length == MYSQL_MIDBLOB) {
                                info.DeclaredType = L"MEDIUMTEXT";
                            } else if (f.length == MYSQL_BLOB) {
                                info.DeclaredType = L"TEXT";
                            } else {
                                info.DeclaredType = L"LONGTEXT";
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_VAR_STRING:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            info.DeclaredType = L"VARBINARY";
                            info.DataType = (VT_UI1 | VT_ARRAY);
                        } else {
                            info.DeclaredType = L"VARCHAR";
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        }
                        break;
                    case MYSQL_TYPE_STRING:
                        info.ColumnSize = f.length;
                        if ((f.flags & ENUM_FLAG) == ENUM_FLAG) {
                            info.DeclaredType = L"ENUM";
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        } else if ((f.flags & SET_FLAG) == SET_FLAG) {
                            info.DeclaredType = L"SET";
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        } else {
                            if (f.charsetnr == IS_BINARY) {
                                info.DeclaredType = L"BINARY";
                                info.DataType = (VT_UI1 | VT_ARRAY);
                            } else {
                                info.DeclaredType = L"CHAR";
                                info.DataType = (VT_I1 | VT_ARRAY); //string
                            }
                        }
                        info.ColumnSize = f.length;
                        break;
                    case MYSQL_TYPE_GEOMETRY:
                        info.ColumnSize = f.length;
                        info.DeclaredType = L"GEOMETRY";
                        info.DataType = (VT_UI1 | VT_ARRAY); //binary array
                        break;
                    default:
                        assert(false); //not implemented
                        break;
                }
            }
            return vCols;
        }

        bool CMysqlImpl::SendRows(CScopeUQueue& sb, bool transferring) {
            bool batching = (GetBytesBatched() >= DEFAULT_RECORD_BATCH_SIZE);
            if (batching) {
                CommitBatching();
            }
            unsigned int ret = SendResult(transferring ? idTransferring : idEndRows, sb->GetBuffer(), sb->GetSize());
            sb->SetSize(0);
            if (batching) {
                StartBatching();
            }
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        bool CMysqlImpl::SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes) {
            unsigned int ret = SendResult(idStartBLOB,
            (unsigned int) (bytes + sizeof (unsigned short) + sizeof (unsigned int) + sizeof (unsigned int))/* extra 4 bytes for string null termination*/,
            data_type, bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            while (bytes > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                ret = SendResult(idChunk, buffer, DEFAULT_BIG_FIELD_CHUNK_SIZE);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                assert(ret == DEFAULT_BIG_FIELD_CHUNK_SIZE);
                buffer += DEFAULT_BIG_FIELD_CHUNK_SIZE;
                bytes -= DEFAULT_BIG_FIELD_CHUNK_SIZE;
            }
            ret = SendResult(idEndBLOB, buffer, bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        void CMysqlImpl::ConvertToUTF8OrDouble(CDBVariant & vt) {
            switch (vt.Type()) {
                case VT_DATE:
                {
                    char str[32] = {0};
                    UDateTime d(vt.ullVal);
                    d.ToDBString(str, sizeof (str));
                    vt = (const char*) str;
                }
                    break;
                case VT_CY:
                {
                    double d = (double) vt.cyVal.int64;
                    d /= 10000.0;
                    vt = d;
                }
                    break;
                case VT_DECIMAL:
                {
                    const DECIMAL &decVal = vt.decVal;
                    std::string s = std::to_string(decVal.Lo64);
                    unsigned char len = (unsigned char) s.size();
                    if (len <= decVal.scale) {
                        s.insert(0, (decVal.scale - len) + 1, '0');
                    }
                    if (decVal.sign) {
                        s.insert(0, 1, '-');
                    }
                    if (decVal.scale) {
                        size_t pos = s.length() - decVal.scale;
                        s.insert(pos, 1, '.');
                    }
                    vt = s.c_str();
                }
                    break;
                default:
                    break;
            }
        }

        UINT64 CMysqlImpl::ConvertBitsToInt(const unsigned char *s, unsigned int bytes) {
            UINT64 n = 0;
            for (unsigned int i = 0; i < bytes; ++i) {
                if (i) {
                    n <<= 8;
                }
                n += s[i];
            }
            return n;
        }

        bool CMysqlImpl::PushRecords(MYSQL_RES *result, const CDBColumnInfoArray &vColInfo, int &res, std::wstring & errMsg) {
            VARTYPE vt;
            CScopeUQueue sb;
            size_t fields = vColInfo.size();
            MYSQL_ROW ppData = m_pLib->mysql_fetch_row(result);
            while (ppData) {
                bool blob = false;
                unsigned long *lengths = m_pLib->mysql_fetch_lengths(result);
                for (size_t i = 0; i < fields; ++i) {
                    const CDBColumnInfo &colInfo = vColInfo[i];
                    vt = colInfo.DataType;
                    unsigned int len = (unsigned int) (lengths[i]);
                    char *data = ppData[i];
                    if (data) {
                        if ((vt & VT_ARRAY) != VT_ARRAY) {
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                        }
                        switch (vt) {
                            case (VT_I1 | VT_ARRAY):
                            case (VT_UI1 | VT_ARRAY):
                            {
                                if (colInfo.Precision && colInfo.Precision <= 19) {
                                    DECIMAL dec;
                                    ParseDec(data, dec);
                                    vt = VT_DECIMAL;
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb << dec;
                                } else if (len > (2 * DEFAULT_BIG_FIELD_CHUNK_SIZE)) {
                                    if (sb->GetSize() && !SendRows(sb, true)) {
                                        return false;
                                    }
                                    bool batching = IsBatching();
                                    if (batching) {
                                        CommitBatching();
                                    }

                                    if (!SendBlob(vt, (const unsigned char *) data, len)) {
                                        return false;
                                    }
                                    if (batching) {
                                        StartBatching();
                                    }
                                    blob = true;
                                } else {
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb->Push((const unsigned char*) &len, sizeof (len));
                                    sb->Push((const unsigned char*) data, len);
                                }
                            }
                                break;
                            case VT_BOOL:
                            {
                                unsigned short c = *data ? (~0) : 0;
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I1:
                            {
                                const char *end;
                                char c = (char) SPA::atoi(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_UI1:
                            {
                                unsigned char c;
                                if (colInfo.ColumnSize) {
                                    //bit
                                    c = (unsigned char) (data[0]);
                                } else {
                                    const char *end;
                                    c = (unsigned char) SPA::atoui(data, end);
                                }
                                sb->Push(&c, sizeof (c));
                            }
                                break;
                            case VT_UI2:
                            {
                                unsigned short c;
                                if (colInfo.ColumnSize)
                                    //bit
                                    c = (unsigned short) ConvertBitsToInt((unsigned char*) data, len);
                                else {
                                    const char *end;
                                    c = (unsigned short) SPA::atoui(data, end);
                                }
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I2:
                            {
                                const char *end;
                                short c = (short) SPA::atoi(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_UI4:
                            {
                                unsigned int c;
                                if (colInfo.ColumnSize)
                                    //bit
                                    c = (unsigned int) ConvertBitsToInt((unsigned char*) data, len);
                                else {
                                    const char *end;
                                    c = SPA::atoui(data, end);
                                }
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I4:
                            {
                                const char *end;
                                int c = SPA::atoi(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_UI8:
                            {
                                UINT64 c;
                                if (colInfo.ColumnSize)
                                    //bit
                                    c = ConvertBitsToInt((unsigned char*) data, len);
                                else {
                                    const char *end;
                                    c = SPA::atoull(data, end);
                                }
                                sb->Push((const unsigned char*) &c, sizeof (UINT64));
                            }
                                break;
                            case VT_I8:
                            {
                                const char *end;
                                INT64 c = SPA::atoll(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (INT64));
                            }
                                break;
                            case VT_R4:
                            {
                                const char *end;
                                float c = (float) SPA::atof(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_R8:
                            {
                                const char *end;
                                double c = SPA::atof(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_DATE:
                            {
                                UDateTime udt(data);
                                sb << udt.time;
                            }
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    } else {
                        vt = VT_NULL;
                        sb->Push((const unsigned char*) &vt, sizeof (vt));
                    }
                }
                if ((sb->GetSize() >= DEFAULT_RECORD_BATCH_SIZE || blob) && !SendRows(sb)) {
                    return false;
                }
                ppData = m_pLib->mysql_fetch_row(result);
            }
            if (sb->GetSize()) {
                return SendRows(sb);
            }
            return true;
        }

        void CMysqlImpl::ExecuteSqlWithRowset(bool meta, UINT64 index, int &res, std::wstring &errMsg, INT64 & affected) {
            unsigned int ret;
            CScopeUQueue sbRowset;
            do {
                MYSQL_RES *result = m_pLib->mysql_use_result(m_pMysql.get());
                if (result) {
                    unsigned int cols = m_pLib->mysql_num_fields(result);
                    CDBColumnInfoArray vInfo = GetColInfo(result, cols, false);
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                        m_pLib->mysql_free_result(result);
                        return;
                    }
                    bool ok = PushRecords(result, vInfo, res, errMsg);
                    m_pLib->mysql_free_result(result);
                    ++m_oks;

                    //For SELECT statements, mysql_affected_rows() works like mysql_num_rows().
                    //affected += (INT64)m_pLib->mysql_affected_rows(m_pMysql.get());

                    if (!ok) {
                        return;
                    }
                } else {
                    CDBColumnInfoArray vInfo;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                        return;
                    }
                    int errCode = m_pLib->mysql_errno(m_pMysql.get());
                    if (!errCode) {
                        ++m_oks;
                        affected += (INT64) m_pLib->mysql_affected_rows(m_pMysql.get());
                    } else {
                        ++m_fails;
                        if (!res) {
                            res = errCode;
                            errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
                        }
                    }
                }
                int status = m_pLib->mysql_next_result(m_pMysql.get());
                if (status == -1) {
                    break; //Successful and there are no more results
                } else if (status > 0) {
                    ++m_fails;
                    if (!res) {
                        res = m_pLib->mysql_errno(m_pMysql.get());
                        errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
                    }
                    break;
                } else if (status == 0) {
                    //Successful and there are more results
                } else {
                    assert(false); //never come here
                }
                sbRowset->SetSize(0);
            } while (true);
        }

        void CMysqlImpl::Execute(const std::wstring& wsql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            fail_ok = 0;
            affected = 0;
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
            }
            vtId = (INT64) 0;
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            CScopeUQueue sb;
            Utilities::ToUTF8(wsql.c_str(), wsql.size(), *sb);
            const char *sqlUtf8 = (const char*) sb->GetBuffer();
            int status = m_pLib->mysql_real_query(m_pMysql.get(), sqlUtf8, sb->GetSize());
            if (status) {
                res = m_pLib->mysql_errno(m_pMysql.get());
                errMsg = Utilities::ToWide(m_pLib->mysql_error(m_pMysql.get()));
                ++m_fails;
            } else {
                if (rowset) {
                    ExecuteSqlWithRowset(meta, index, res, errMsg, affected);
                } else {
                    ExecuteSqlWithoutRowset(res, errMsg, affected);
                }
                if (lastInsertId) {
                    if (affected) {
                        vtId = (INT64) m_pLib->mysql_insert_id(m_pMysql.get());
                    }
                }
            }
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void CMysqlImpl::PreprocessPreparedStatement() {
            std::string s = m_sqlPrepare;
            transform(s.begin(), s.end(), s.begin(), ::tolower);
            m_bCall = (s.find("call ") == 0);
            if (m_bCall) {
                auto pos = m_sqlPrepare.find('(');
                if (pos != std::string::npos) {
                    m_procName.assign(m_sqlPrepare.begin() + 5, m_sqlPrepare.begin() + pos);
                } else {
                    m_procName = m_sqlPrepare.substr(5);
                }
                auto dot = m_procName.rfind('.');
                if (dot != std::string::npos) {
                    m_procName = m_procName.substr(dot + 1);
                }
                if (m_procName.back() == '`') {
                    m_procName.pop_back();
                    m_procName.erase(m_procName.begin());
                }
                CMysqlImpl::MYSQL_CONNECTION_STRING::Trim(m_procName);
            } else {
                m_procName.clear();
            }
        }

        void CMysqlImpl::Prepare(const std::wstring& wsql, CParameterInfoArray& params, int &res, std::wstring &errMsg, unsigned int &parameters) {
            parameters = 0;
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            }
            m_pPrepare.reset();
            m_vParam.clear();
            m_parameters = 0;
            m_sqlPrepare = Utilities::ToUTF8(wsql.c_str(), wsql.size());
            CMysqlImpl::MYSQL_CONNECTION_STRING::Trim(m_sqlPrepare);
            MYSQL_STMT *stmt = m_pLib->mysql_stmt_init(m_pMysql.get());
            PreprocessPreparedStatement();
            my_bool fail;
            do {
                if (!m_pLib->m_bRemote && m_bCall) {
                    //this is hack for embedded mysql store procedure to prevent the async mysql server library from crash
                    unsigned long type = (unsigned long) CURSOR_TYPE_READ_ONLY;
                    fail = m_pLib->mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void *) &type);
                    if (fail) {
                        break;
                    }
                }
                fail = m_pLib->mysql_stmt_prepare(stmt, m_sqlPrepare.c_str(), (unsigned long) m_sqlPrepare.size());
                if (fail) {
                    break;
                }
            } while (false);
            if (fail) {
                res = m_pLib->mysql_stmt_errno(stmt);
                errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(stmt));
                m_pLib->mysql_stmt_close(stmt);
            } else {
                res = 0;
                m_parameters = m_pLib->mysql_stmt_param_count(stmt);
                m_pPrepare.reset(stmt, [this](MYSQL_STMT * stmt) {
                    if (stmt) {
                        this->m_pLib->mysql_stmt_close(stmt);
                    }
                });
                parameters = (unsigned int) m_parameters;
            }
        }

        int CMysqlImpl::Bind(CUQueue &qBufferSize, int row, std::wstring & errMsg) {
            int res = 0;
            if (!m_parameters) {
                return res;
            }
            unsigned int size = sizeof (MYSQL_BIND);
            size *= (unsigned int) m_parameters;
            SPA::CScopeUQueue sb;
            if (size > sb->GetMaxSize()) {
                sb->ReallocBuffer(size);
            }
            sb->CleanTrack();
            qBufferSize.SetSize(0);
            if ((m_parameters + 1) * sizeof (unsigned long) > qBufferSize.GetMaxSize()) {
                qBufferSize.ReallocBuffer((unsigned int) ((m_parameters + 1) * sizeof (unsigned long)));
            }
            unsigned int indexBS = 0;
            MYSQL_BIND *pBind = (MYSQL_BIND*) sb->GetBuffer();
            for (size_t n = 0; n < m_parameters; ++n) {
                CDBVariant &data = m_vParam[row * m_parameters + n];
                unsigned short vt = data.Type();
                MYSQL_BIND &bind = pBind[n];
                switch (vt) {
                    case VT_NULL:
                    case VT_EMPTY:
                        bind.buffer_type = MYSQL_TYPE_NULL;
                        bind.is_null = (my_bool*) & CMysqlImpl::B_IS_NULL;
                        break;
                    case VT_I1:
                        bind.buffer_type = MYSQL_TYPE_TINY;
                        bind.buffer = (void*) &data.cVal;
                        bind.buffer_length = sizeof (char);
                        break;
                    case VT_UI1:
                        bind.buffer_type = MYSQL_TYPE_TINY;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.bVal;
                        bind.buffer_length = sizeof (unsigned char);
                        break;
                    case VT_UI2:
                        bind.buffer_type = MYSQL_TYPE_SHORT;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.uiVal;
                        bind.buffer_length = sizeof (unsigned short);
                        break;
                    case VT_I2:
                        bind.buffer_type = MYSQL_TYPE_SHORT;
                        bind.buffer = (void*) &data.iVal;
                        bind.buffer_length = sizeof (short);
                        break;
                    case VT_UINT:
                    case VT_UI4:
                        bind.buffer_type = MYSQL_TYPE_LONG;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.ulVal;
                        bind.buffer_length = sizeof (unsigned int);
                        break;
                    case VT_INT:
                    case VT_I4:
                        bind.buffer_type = MYSQL_TYPE_LONG;
                        bind.buffer = (void*) &data.lVal;
                        bind.buffer_length = sizeof (int);
                        break;
                    case VT_UI8:
                        bind.buffer_type = MYSQL_TYPE_LONGLONG;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.ullVal;
                        bind.buffer_length = sizeof (UINT64);
                        break;
                    case VT_I8:
                        bind.buffer_type = MYSQL_TYPE_LONGLONG;
                        bind.buffer = (void*) &data.llVal;
                        bind.buffer_length = sizeof (INT64);
                        break;
                    case VT_BOOL:
                        bind.buffer_type = MYSQL_TYPE_BIT;
                        bind.buffer_length = sizeof (bool);
                        data.cVal = data.boolVal ? 1 : 0;
                        bind.buffer = (void*) &data.cVal;
                        break;
                    case VT_R4:
                        bind.buffer_type = MYSQL_TYPE_FLOAT;
                        bind.buffer = (void*) &data.fltVal;
                        bind.buffer_length = sizeof (float);
                        break;
                    case VT_R8:
                        bind.buffer_type = MYSQL_TYPE_DOUBLE;
                        bind.buffer = (void*) &data.dblVal;
                        bind.buffer_length = sizeof (double);
                        break;
                    case VT_STR:
                    case (VT_ARRAY | VT_I1):
                        bind.buffer_type = MYSQL_TYPE_STRING;
                        bind.buffer_length = data.parray->rgsabound->cElements;
                        ::SafeArrayAccessData(data.parray, &bind.buffer);
                        m_vArray.push_back(data.parray);
                        qBufferSize << bind.buffer_length;
                        bind.length = (unsigned long*) qBufferSize.GetBuffer(indexBS * sizeof (unsigned long));
                        ++indexBS;
                        break;
                    case VT_BYTES:
                    case (VT_ARRAY | VT_UI1):
                        bind.buffer_type = MYSQL_TYPE_BLOB;
                        bind.buffer_length = data.parray->rgsabound->cElements;
                        ::SafeArrayAccessData(data.parray, &bind.buffer);
                        m_vArray.push_back(data.parray);
                        qBufferSize << bind.buffer_length;
                        bind.length = (unsigned long*) qBufferSize.GetBuffer(indexBS * sizeof (unsigned long));
                        ++indexBS;
                        break;
                    default:
                        assert(false); //not implemented
                        if (!res) {
                            res = SPA::Mysql::ER_DATA_TYPE_NOT_SUPPORTED;
                            errMsg = DATA_TYPE_NOT_SUPPORTED;
                        }
                        break;
                }
            }
            if (!res && m_pLib->mysql_stmt_bind_param(m_pPrepare.get(), pBind)) {
                res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                if (!errMsg.size()) {
                    errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                }
            }
            return res;
        }

        std::shared_ptr<MYSQL_BIND> CMysqlImpl::PrepareBindResultBuffer(MYSQL_RES *result, const CDBColumnInfoArray &vColInfo, int &res, std::wstring &errMsg, std::shared_ptr<MYSQL_BIND_RESULT_FIELD> &field) {
            std::shared_ptr<MYSQL_BIND> p(new MYSQL_BIND[vColInfo.size()], [](MYSQL_BIND * b) {
                if (b) {
                    delete []b;
                }
            });
            field.reset(new MYSQL_BIND_RESULT_FIELD[vColInfo.size()], [](MYSQL_BIND_RESULT_FIELD * f) {
                if (f) {
                    delete []f;
                }
            });
            MYSQL_BIND *ps_params = p.get();
            MYSQL_BIND_RESULT_FIELD *ps_fields = field.get();
            size_t index = 0;
            for (auto it = vColInfo.begin(), end = vColInfo.end(); it != end; ++it, ++index) {
                MYSQL_BIND &b = ps_params[index];
                ::memset(&b, 0, sizeof (b));
                MYSQL_BIND_RESULT_FIELD &f = ps_fields[index];
                VARTYPE vt = it->DataType;
                switch (vt) {
                    case VT_ARRAY | VT_I1:
                    case VT_ARRAY | VT_UI1:
                        if (it->ColumnSize > 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                            if (f.buffer_length < 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                f.ResetMaxBuffer(2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
                            }
                        } else {
                            if (it->ColumnSize > f.buffer_length) {
                                f.ResetMaxBuffer((unsigned int) it->ColumnSize);
                            }
                        }
                        if (vt == (VT_ARRAY | VT_I1)) {
                            b.buffer_type = MYSQL_TYPE_STRING;
                        } else {
                            b.buffer_type = MYSQL_TYPE_BLOB;
                        }
                        break;
                    case VT_UI1:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_I1:
                        b.buffer_type = MYSQL_TYPE_TINY;
                        break;
                    case VT_UI2:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_I2:
                        b.buffer_type = MYSQL_TYPE_SHORT;
                        break;
                    case VT_BOOL:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_UINT:
                    case VT_UI4:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_INT:
                    case VT_I4:
                        b.buffer_type = MYSQL_TYPE_LONG;
                        break;
                    case VT_UI8:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_I8:
                        b.buffer_type = MYSQL_TYPE_LONGLONG;
                        break;
                    case VT_R4:
                        b.buffer_type = MYSQL_TYPE_FLOAT;
                        break;
                    case VT_R8:
                        b.buffer_type = MYSQL_TYPE_DOUBLE;
                        break;
                    case VT_DATE:
                        b.buffer_type = MYSQL_TYPE_TIMESTAMP;
                        break;
                    default:
                        assert(false);
                        break;
                }
                b.buffer = f.GetBuffer();
                b.buffer_length = f.buffer_length;
                b.length = &f.length;
                b.is_null = &f.is_null;
            }
            my_bool fail = m_pLib->mysql_stmt_bind_result(m_pPrepare.get(), ps_params);
            if (fail) {
                if (!res) {
                    res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                    errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                }
                p.reset();
                field.reset();
            }
            return p;
        }

        bool CMysqlImpl::PushRecords(UINT64 index, MYSQL_BIND *binds, MYSQL_BIND_RESULT_FIELD *fields, const CDBColumnInfoArray &vColInfo, bool rowset, bool output, int &res, std::wstring & errMsg) {
            unsigned int sent;
            size_t cols = vColInfo.size();
            if (output) {
                sent = SendResult(idBeginRows, index);
                if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                    return false;
                }
            }
            CScopeUQueue sb;
            //successful binding
            int ret = m_pLib->mysql_stmt_fetch(m_pPrepare.get());
            while (ret != MYSQL_NO_DATA && ret != 1) {
                if (output || rowset) {
                    bool blob = false;
                    VARTYPE vt;
                    for (size_t i = 0; i < cols; ++i) {
                        MYSQL_BIND_RESULT_FIELD &f = fields[i];
                        if (f.is_null) {
                            vt = VT_NULL;
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                            continue;
                        }
                        const CDBColumnInfo &colInfo = vColInfo[i];
                        MYSQL_BIND &b = binds[i];
                        vt = colInfo.DataType;
                        if ((vt & VT_ARRAY) != VT_ARRAY) {
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                        }
                        switch (vt) {
                            case (VT_I1 | VT_ARRAY):
                            case (VT_UI1 | VT_ARRAY):
                            {
                                unsigned int len = (unsigned int) *b.length;
                                if (len >= b.buffer_length) {
                                    if (sb->GetSize() && !SendRows(sb, true)) {
                                        return false;
                                    }
                                    sent = SendResult(idStartBLOB,
                                            (unsigned int) (len + sizeof (unsigned short) + sizeof (unsigned int) + sizeof (unsigned int))/* extra 4 bytes for string null termination*/,
                                            vt, len);
                                    if (ret == (int) REQUEST_CANCELED || ret == (int) SOCKET_NOT_FOUND) {
                                        return false;
                                    }
                                    bool batching = IsBatching();
                                    if (batching) {
                                        CommitBatching();
                                    }
                                    unsigned long offset = 0;
                                    unsigned int remain = len;
                                    unsigned int obtain = b.buffer_length;
                                    while (remain) {
                                        ret = SendResult((obtain >= b.buffer_length) ? idChunk : idEndBLOB, (const unsigned char*) b.buffer, obtain);
                                        if (ret == (int) REQUEST_CANCELED || ret == (int) SOCKET_NOT_FOUND) {
                                            return false;
                                        }
                                        remain -= obtain;
                                        offset += obtain;
                                        if (!remain) {
                                            break;
                                        }
                                        ret = m_pLib->mysql_stmt_fetch_column(m_pPrepare.get(), &b, (unsigned int) i, offset);
                                        if (ret) {
                                            if (!res) {
                                                res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                                                errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                                            }
                                            return false;
                                        }
                                        if (remain < b.buffer_length) {
                                            obtain = remain;
                                        }
                                    }
                                    if (batching) {
                                        StartBatching();
                                    }
                                } else if (colInfo.Precision && colInfo.Precision <= 19) {
                                    DECIMAL dec;
                                    ParseDec((const char*) b.buffer, dec);
                                    vt = VT_DECIMAL;
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb << dec;
                                } else {
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb->Push((const unsigned char*) &len, sizeof (len));
                                    sb->Push((const unsigned char*) b.buffer, len);
                                }
                            }
                                break;
                            case VT_I1:
                                assert(*b.length == sizeof (char));
                                sb->Push((const unsigned char*) b.buffer, sizeof (char));
                                break;
                            case VT_UI1:
                            {
                                sb->Push((const unsigned char*) b.buffer, sizeof (unsigned char));
                            }
                                break;
                            case VT_I2:
                                assert(*b.length == sizeof (short));
                                sb->Push((const unsigned char*) b.buffer, sizeof (short));
                                break;
                            case VT_UI2:
                            {
                                unsigned int len = *b.length;
                                unsigned short c = (unsigned short) ConvertBitsToInt((unsigned char*) b.buffer, len);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_BOOL:
                                assert(*b.length == sizeof (char));
                            {
                                unsigned short s = *((char*) b.buffer) ? (~0) : 0;
                                sb->Push((const unsigned char*) &s, sizeof (unsigned short));
                            }
                                break;
                            case VT_UI4:
                            case VT_UINT:
                            {
                                unsigned int len = *b.length;
                                unsigned int c = (unsigned int) ConvertBitsToInt((unsigned char*) b.buffer, len);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I4:
                            case VT_INT:
                            case VT_R4:
                                assert(*b.length == sizeof (int));
                                sb->Push((const unsigned char*) b.buffer, sizeof (int));
                                break;
                            case VT_UI8:
                            {
                                unsigned int len = *b.length;
                                UINT64 c = ConvertBitsToInt((unsigned char*) b.buffer, len);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_R8:
                            case VT_I8:
                                assert(*b.length == sizeof (INT64));
                                sb->Push((const unsigned char*) b.buffer, sizeof (INT64));
                                break;
                            case VT_DATE:
                            {
                                const MYSQL_TIME *date = (const MYSQL_TIME*) b.buffer;
                                sb << ToUDateTime(*date);
                            }
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    }
                    if (rowset) {
                        if ((sb->GetSize() >= DEFAULT_RECORD_BATCH_SIZE || blob) && !SendRows(sb)) {
                            return false;
                        }
                    }
                }
                ret = m_pLib->mysql_stmt_fetch(m_pPrepare.get());
            }
            assert(ret != MYSQL_DATA_TRUNCATED);
            if (ret == 1 && !res) {
                res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
            }
            if (output) {
                //tell output parameter data
                sent = SendResult(idOutputParameter, sb->GetBuffer(), sb->GetSize());
                if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                    return false;
                }
            } else if (rowset) {
                if (sb->GetSize()) {
                    return SendRows(sb);
                }
            }
            return true;
        }

        UINT64 CMysqlImpl::ToUDateTime(const MYSQL_TIME & td) {
            std::tm date;
            if (td.time_type == MYSQL_TIMESTAMP_TIME) {
                date.tm_year = 0;
                date.tm_mon = 0;
                date.tm_mday = 0;
            } else {
                date.tm_year = td.year - 1900;
                date.tm_mon = td.month - 1;
                date.tm_mday = td.day;
            }
            date.tm_hour = td.hour;
            date.tm_min = td.minute;
            date.tm_sec = td.second;
            return SPA::UDateTime(date, td.second_part).time;
        }

#ifdef NO_PARAMETER_PREPARED_STATEMENT_SUPPORTED

        void CMysqlImpl::ExecuteNoParameter(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            int ret = m_pLib->mysql_stmt_execute(m_pPrepare.get());
            if (ret) {
                res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                fail_ok = 1;
                fail_ok <<= 32;
                ++m_fails;
                return;
            }
            res = 0;
            bool header_sent = false;
            //For SELECT statements, mysql_stmt_affected_rows() works like mysql_num_rows().
            my_ulonglong affected_rows = m_pLib->mysql_stmt_affected_rows(m_pPrepare.get());
            if (affected_rows != (~0) && affected_rows) {
                affected += affected_rows;
            }
            unsigned int cols = m_pLib->mysql_stmt_field_count(m_pPrepare.get());
            bool output = (m_pMysql.get()->server_status & SERVER_PS_OUT_PARAMS) ? true : false;
            MYSQL_RES *result = m_pLib->mysql_stmt_result_metadata(m_pPrepare.get());
            while (result && cols) {
                CDBColumnInfoArray vInfo = GetColInfo(result, cols, true);
                if (!output && m_pLib->m_bRemote) {
                    //Mysql server_status & SERVER_PS_OUT_PARAMS does NOT work correctly for an unknown reason
                    //This is a hack solution for detecting output result, which may be wrong if a table name is EXACTLY the same as stored procedure name
                    output = (m_bCall && (vInfo[0].TablePath == Utilities::ToWide(m_procName.c_str())));
                }

                //we push stored procedure output parameter meta data onto client to follow common approach for output parameter data
                if (output || rowset) {
                    CScopeUQueue sb;
                    unsigned int outputs = 0;
                    if (output) {
                        outputs = (unsigned int) vInfo.size();
                    }
                    sb << vInfo << index << outputs;
                    unsigned int sent = SendResult(idRowsetHeader, sb->GetBuffer(), sb->GetSize());
                    header_sent = true;
                    if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                        m_pLib->mysql_free_result(result);
                        m_pLib->mysql_stmt_free_result(m_pPrepare.get());
                        return;
                    }
                }
                std::shared_ptr<MYSQL_BIND_RESULT_FIELD> fields;
                std::shared_ptr<MYSQL_BIND> pBinds = PrepareBindResultBuffer(result, vInfo, res, errMsg, fields);
                MYSQL_BIND *mybind = pBinds.get();
                MYSQL_BIND_RESULT_FIELD *myfield = fields.get();
                if (pBinds && (output || rowset)) {
                    if (!PushRecords(index, mybind, myfield, vInfo, rowset, output, res, errMsg)) {
                        m_pLib->mysql_free_result(result);
                        m_pLib->mysql_stmt_free_result(m_pPrepare.get());
                        return;
                    }
                }
                m_pLib->mysql_free_result(result);
                pBinds.reset();
                fields.reset();
                if (res) {
                    break;
                }
                ret = m_pLib->mysql_stmt_next_result(m_pPrepare.get());
                if (output) {
                    break;
                } else if (ret == 0) {
                    //continue for the next set
                } else if (ret == -1) {
                    //no more result
                    break;
                } else if (ret > 0) {
                    //error
                    if (!res) {
                        res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                        errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                    }
                    break;
                } else {
                    //should never come here
                    assert(false);
                }
                cols = m_pLib->mysql_stmt_field_count(m_pPrepare.get());
                output = (m_pMysql.get()->server_status & SERVER_PS_OUT_PARAMS) ? true : false;
                result = m_pLib->mysql_stmt_result_metadata(m_pPrepare.get());
            }
            if (m_pLib->mysql_stmt_free_result(m_pPrepare.get())) {
                if (!res) {
                    res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                    errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                }
            }
            if (!header_sent && rowset) {
                SPA::CScopeUQueue sbRowset;
                CDBColumnInfoArray vInfo;
                sbRowset << vInfo << index;
                SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
            }
            if (res) {
                fail_ok = 1;
                fail_ok <<= 32;
                ++m_fails;
            } else {
                fail_ok = 1;
                ++m_oks;
            }
            if (lastInsertId) {
                vtId = (INT64) m_pLib->mysql_stmt_insert_id(m_pPrepare.get());
            }
        }
#endif

        void CMysqlImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            fail_ok = 0;
            affected = 0;
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }

            if (!m_pPrepare) {
                res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }

            if (!m_parameters) {
#ifdef NO_PARAMETER_PREPARED_STATEMENT_SUPPORTED
                //Executing a prepared statement may crash if no parameter is provided!
                if (m_vParam.size()) {
                    res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                    errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                    ++m_fails;
                    fail_ok = 1;
                    fail_ok <<= 32;
                    return;
                }
                ExecuteNoParameter(rowset, meta, lastInsertId, index, affected, res, errMsg, vtId, fail_ok);
#else
                res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
#endif
                return;
            } else if ((m_vParam.size() % m_parameters) || (m_vParam.size() == 0)) {
                res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }

            res = 0;
            CScopeUQueue sb;
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            bool header_sent = false;
            int rows = (int) (m_vParam.size() / m_parameters);
            for (int row = 0; row < rows; ++row) {
                int ret = Bind(*sb, row, errMsg);
                if (ret) {
                    if (!res) {
                        res = ret;
                    }
                    ++m_fails;
                    continue;
                }
                ret = m_pLib->mysql_stmt_execute(m_pPrepare.get());
                if (ret) {
                    if (!res) {
                        res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                        errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                    }
                    ++m_fails;
                    continue;
                }

                //For SELECT statements, mysql_stmt_affected_rows() works like mysql_num_rows().
                my_ulonglong affected_rows = m_pLib->mysql_stmt_affected_rows(m_pPrepare.get());
                if (affected_rows != (my_ulonglong) (~0) && affected_rows) {
                    affected += affected_rows;
                }

                unsigned int cols = m_pLib->mysql_stmt_field_count(m_pPrepare.get());
                bool output = (m_pMysql.get()->server_status & SERVER_PS_OUT_PARAMS) ? true : false;
                MYSQL_RES *result = m_pLib->mysql_stmt_result_metadata(m_pPrepare.get());
#ifndef NDEBUG
                if (cols) {
                    assert(result);
                }
#endif
                while (result && cols) {
                    CDBColumnInfoArray vInfo = GetColInfo(result, cols, true);

                    if (!output/* && m_pLib->m_bRemote*/) {
                        //Mysql + Mariadb server_status & SERVER_PS_OUT_PARAMS does NOT work correctly for an unknown reason
                        //This is a hack solution for detecting output result, which may be wrong if a table name is EXACTLY the same as stored procedure name
                        output = (m_bCall && (vInfo[0].TablePath == Utilities::ToWide(m_procName.c_str())));
                    }

                    //we push stored procedure output parameter meta data onto client to follow common approach for output parameter data
                    if (output || rowset) {
                        CScopeUQueue sb;
                        unsigned int outputs = 0;
                        if (output) {
                            outputs = (unsigned int) vInfo.size();
                        }
                        sb << vInfo << index << outputs;
                        unsigned int sent = SendResult(idRowsetHeader, sb->GetBuffer(), sb->GetSize());
                        header_sent = true;
                        if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                            m_pLib->mysql_free_result(result);
                            m_pLib->mysql_stmt_free_result(m_pPrepare.get());
                            return;
                        }
                    }
                    std::shared_ptr<MYSQL_BIND_RESULT_FIELD> fields;
                    std::shared_ptr<MYSQL_BIND> pBinds = PrepareBindResultBuffer(result, vInfo, res, errMsg, fields);
                    MYSQL_BIND *mybind = pBinds.get();
                    MYSQL_BIND_RESULT_FIELD *myfield = fields.get();
                    if (pBinds && (output || rowset)) {
                        if (!PushRecords(index, mybind, myfield, vInfo, rowset, output, res, errMsg)) {
                            m_pLib->mysql_free_result(result);
                            m_pLib->mysql_stmt_free_result(m_pPrepare.get());
                            return;
                        }
                    }
                    m_pLib->mysql_free_result(result);
                    pBinds.reset();
                    fields.reset();
                    ret = m_pLib->mysql_stmt_next_result(m_pPrepare.get());
                    if (output) {
                        ret = 0;
                        break;
                    } else if (ret == 0) {
                        //continue for the next set
                    } else if (ret == -1) {
                        //no more result
                        ret = 0;
                        break;
                    } else if (ret > 0) {
                        //error
                        if (!res) {
                            res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                            errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                        }
                        break;
                    } else {
                        //should never come here
                        assert(false);
                    }
                    cols = m_pLib->mysql_stmt_field_count(m_pPrepare.get());
                    output = (m_pMysql.get()->server_status & SERVER_PS_OUT_PARAMS) ? true : false;
                    result = m_pLib->mysql_stmt_result_metadata(m_pPrepare.get());
                }
                if (m_pLib->mysql_stmt_free_result(m_pPrepare.get())) {
                    ret = 1;
                    if (!res) {
                        res = m_pLib->mysql_stmt_errno(m_pPrepare.get());
                        errMsg = Utilities::ToWide(m_pLib->mysql_stmt_error(m_pPrepare.get()));
                    }
                }
                if (ret)
                    ++m_fails;
                else
                    ++m_oks;
            }
            if (!header_sent && rowset) {
                SPA::CScopeUQueue sbRowset;
                CDBColumnInfoArray vInfo;
                sbRowset << vInfo << index;
                SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
            }
            if (lastInsertId) {
                vtId = (INT64) m_pLib->mysql_stmt_insert_id(m_pPrepare.get());
            }
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void CMysqlImpl::StartBLOB(unsigned int lenExpected) {
            m_Blob.SetSize(0);
            if (lenExpected > m_Blob.GetMaxSize()) {
                m_Blob.ReallocBuffer(lenExpected);
            }
            CUQueue &q = m_UQueue;
            m_Blob.Push(q.GetBuffer(), q.GetSize());
            assert(q.GetSize() > sizeof (unsigned short) + sizeof (unsigned int));
            q.SetSize(0);
        }

        void CMysqlImpl::Chunk() {
            CUQueue &q = m_UQueue;
            if (q.GetSize()) {
                m_Blob.Push(q.GetBuffer(), q.GetSize());
                q.SetSize(0);
            }
        }

        void CMysqlImpl::EndBLOB() {
            Chunk();
            m_vParam.push_back(CDBVariant());
            CDBVariant &vt = m_vParam.back();
            m_Blob >> vt;
            assert(m_Blob.GetSize() == 0);
        }

        void CMysqlImpl::BeginRows() {
            Transferring();
        }

        void CMysqlImpl::EndRows() {
            Transferring();
        }

        void CMysqlImpl::Transferring() {
            CUQueue &q = m_UQueue;
            while (q.GetSize()) {
                m_vParam.push_back(CDBVariant());
                CDBVariant &vt = m_vParam.back();
                q >> vt;
                ConvertToUTF8OrDouble(vt);
            }
            assert(q.GetSize() == 0);
        }

    } //namespace ServerSide
} //namespace SPA