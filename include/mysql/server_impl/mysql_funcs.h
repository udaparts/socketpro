
#ifndef _UMYSQL_SOCKETPRO_FUNCTIONS_H_
#define _UMYSQL_SOCKETPRO_FUNCTIONS_H_

#include "../../userver.h"
#include "../include/mysql.h"
#include <algorithm>

namespace SPA {
    namespace ServerSide {
        namespace Mysql {

            typedef int (STDCALL *Pmysql_server_init)(int argc, char **argv, char **groups);
            typedef void (STDCALL *Pmysql_server_end)(void);
            typedef my_bool(STDCALL *Pmysql_thread_init)(void);
            typedef void (STDCALL *Pmysql_thread_end)(void);
            typedef my_ulonglong(STDCALL *Pmysql_num_rows)(MYSQL_RES *res);
            typedef unsigned int (STDCALL *Pmysql_num_fields)(MYSQL_RES *res);
            typedef my_bool(STDCALL *Pmysql_eof)(MYSQL_RES *res);
            typedef MYSQL_FIELD* (STDCALL *Pmysql_fetch_field_direct)(MYSQL_RES *res, unsigned int fieldnr);
            typedef MYSQL_FIELD* (STDCALL *Pmysql_fetch_fields)(MYSQL_RES *res);
            typedef MYSQL_ROW_OFFSET(STDCALL *Pmysql_row_tell)(MYSQL_RES *res);
            typedef MYSQL_FIELD_OFFSET(STDCALL *Pmysql_field_tell)(MYSQL_RES *res);
            typedef unsigned int (STDCALL *Pmysql_field_count)(MYSQL *mysql);
            typedef my_ulonglong(STDCALL *Pmysql_affected_rows)(MYSQL *mysql);
            typedef my_ulonglong(STDCALL *Pmysql_insert_id)(MYSQL *mysql);
            typedef unsigned int (STDCALL *Pmysql_errno)(MYSQL *mysql);
            typedef const char* (STDCALL *Pmysql_error)(MYSQL *mysql);
            typedef const char* (STDCALL *Pmysql_sqlstate)(MYSQL *mysql);
            typedef unsigned int (STDCALL *Pmysql_warning_count)(MYSQL *mysql);
            typedef const char* (STDCALL *Pmysql_info)(MYSQL *mysql);
            typedef unsigned long (STDCALL *Pmysql_thread_id)(MYSQL *mysql);
            typedef const char* (STDCALL *Pmysql_character_set_name)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_set_character_set)(MYSQL *mysql, const char *csname);
            typedef MYSQL* (STDCALL *Pmysql_init)(MYSQL *mysql);
            typedef my_bool(STDCALL *Pmysql_ssl_set)(MYSQL *mysql, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher);
            typedef const char* (STDCALL *Pmysql_get_ssl_cipher)(MYSQL *mysql);
            typedef my_bool(STDCALL *Pmysql_change_user)(MYSQL *mysql, const char *user, const char *passwd, const char *db);
            typedef MYSQL* (STDCALL *Pmysql_real_connect)(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag);
            typedef int (STDCALL *Pmysql_select_db)(MYSQL *mysql, const char *db);
            typedef int (STDCALL *Pmysql_query)(MYSQL *mysql, const char *q);
            typedef int (STDCALL *Pmysql_send_query)(MYSQL *mysql, const char *q, unsigned long length);
            typedef int (STDCALL *Pmysql_real_query)(MYSQL *mysql, const char *q, unsigned long length);
            typedef MYSQL_RES* (STDCALL *Pmysql_store_result)(MYSQL *mysql);
            typedef MYSQL_RES* (STDCALL *Pmysql_use_result)(MYSQL *mysql);
            typedef void (STDCALL *Pmysql_get_character_set_info)(MYSQL *mysql, MY_CHARSET_INFO *charset);
            typedef void (*Pmysql_set_local_infile_handler)(MYSQL *mysql, int (*local_infile_init)(void **, const char *, void *), int (*local_infile_read)(void *, char *, unsigned int), void (*local_infile_end)(void *), int (*local_infile_error)(void *, char*, unsigned int), void *);
            typedef void (*Pmysql_set_local_infile_default)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_shutdown)(MYSQL *mysql, enum mysql_enum_shutdown_level shutdown_level);
            typedef int (STDCALL *Pmysql_dump_debug_info)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_refresh)(MYSQL *mysql, unsigned int refresh_options);
            typedef int (STDCALL *Pmysql_kill)(MYSQL *mysql, unsigned long pid);
            typedef int (STDCALL *Pmysql_set_server_option)(MYSQL *mysql, enum enum_mysql_set_option option);
            typedef int (STDCALL *Pmysql_ping)(MYSQL *mysql);
            typedef const char* (STDCALL *Pmysql_stat)(MYSQL *mysql);
            typedef const char* (STDCALL *Pmysql_get_server_info)(MYSQL *mysql);
            typedef const char* (STDCALL *Pmysql_get_client_info)(void);
            typedef unsigned long (STDCALL *Pmysql_get_client_version)(void);
            typedef const char* (STDCALL *Pmysql_get_host_info)(MYSQL *mysql);
            typedef unsigned long (STDCALL *Pmysql_get_server_version)(MYSQL *mysql);
            typedef unsigned int (STDCALL *Pmysql_get_proto_info)(MYSQL *mysql);
            typedef MYSQL_RES* (STDCALL *Pmysql_list_dbs)(MYSQL *mysql, const char *wild);
            typedef MYSQL_RES* (STDCALL *Pmysql_list_tables)(MYSQL *mysql, const char *wild);
            typedef MYSQL_RES* (STDCALL *Pmysql_list_processes)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_options)(MYSQL *mysql, enum mysql_option option, const void *arg);
            typedef int (STDCALL *Pmysql_options4)(MYSQL *mysql, enum mysql_option option, const void *arg1, const void *arg2);
            typedef int (STDCALL *Pmysql_get_option)(MYSQL *mysql, enum mysql_option option, const void *arg);
            typedef void (STDCALL *Pmysql_free_result)(MYSQL_RES *result);
            typedef void (STDCALL *Pmysql_data_seek)(MYSQL_RES *result, my_ulonglong offset);
            typedef MYSQL_ROW_OFFSET(STDCALL *Pmysql_row_seek)(MYSQL_RES *result, MYSQL_ROW_OFFSET offset);
            typedef MYSQL_FIELD_OFFSET(STDCALL *Pmysql_field_seek)(MYSQL_RES *result, MYSQL_FIELD_OFFSET offset);
            typedef MYSQL_ROW(STDCALL *Pmysql_fetch_row)(MYSQL_RES *result);
            typedef unsigned long* (STDCALL *Pmysql_fetch_lengths)(MYSQL_RES *result);
            typedef MYSQL_FIELD* (STDCALL *Pmysql_fetch_field)(MYSQL_RES *result);
            typedef MYSQL_RES* (STDCALL *Pmysql_list_fields)(MYSQL *mysql, const char *table, const char *wild);
            typedef unsigned long (STDCALL *Pmysql_escape_string)(char *to, const char *from, unsigned long from_length);
            typedef unsigned long (STDCALL *Pmysql_hex_string)(char *to, const char *from, unsigned long from_length);
            typedef unsigned long (STDCALL *Pmysql_real_escape_string)(MYSQL *mysql, char *to, const char *from, unsigned long length);
            typedef unsigned long (STDCALL *Pmysql_real_escape_string_quote)(MYSQL *mysql, char *to, const char *from, unsigned long length, char quote);
            typedef void (STDCALL *Pmysql_debug)(const char *debug);
            typedef void (STDCALL *Pmyodbc_remove_escape)(MYSQL *mysql, char *name);
            typedef unsigned int (STDCALL *Pmysql_thread_safe)(void);
            typedef my_bool(STDCALL *Pmysql_embedded)(void);
            typedef my_bool(STDCALL *Pmysql_read_query_result)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_reset_connection)(MYSQL *mysql);
            typedef MYSQL_STMT* (STDCALL *Pmysql_stmt_init)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_stmt_prepare)(MYSQL_STMT *stmt, const char *query, unsigned long length);
            typedef int (STDCALL *Pmysql_stmt_execute)(MYSQL_STMT *stmt);
            typedef int (STDCALL *Pmysql_stmt_fetch)(MYSQL_STMT *stmt);
            typedef int (STDCALL *Pmysql_stmt_fetch_column)(MYSQL_STMT *stmt, MYSQL_BIND *bind_arg, unsigned int column, unsigned long offset);
            typedef int (STDCALL *Pmysql_stmt_store_result)(MYSQL_STMT *stmt);
            typedef unsigned long (STDCALL *Pmysql_stmt_param_count)(MYSQL_STMT * stmt);
            typedef my_bool(STDCALL *Pmysql_stmt_attr_set)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, const void *attr);
            typedef my_bool(STDCALL *Pmysql_stmt_attr_get)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, void *attr);
            typedef my_bool(STDCALL *Pmysql_stmt_bind_param)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
            typedef my_bool(STDCALL *Pmysql_stmt_bind_result)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
            typedef my_bool(STDCALL *Pmysql_stmt_close)(MYSQL_STMT * stmt);
            typedef my_bool(STDCALL *Pmysql_stmt_reset)(MYSQL_STMT * stmt);
            typedef my_bool(STDCALL *Pmysql_stmt_free_result)(MYSQL_STMT *stmt);
            typedef my_bool(STDCALL *Pmysql_stmt_send_long_data)(MYSQL_STMT *stmt, unsigned int param_number, const char *data, unsigned long length);
            typedef MYSQL_RES* (STDCALL *Pmysql_stmt_result_metadata)(MYSQL_STMT *stmt);
            typedef MYSQL_RES* (STDCALL *Pmysql_stmt_param_metadata)(MYSQL_STMT *stmt);
            typedef unsigned int (STDCALL *Pmysql_stmt_errno)(MYSQL_STMT * stmt);
            typedef const char* (STDCALL *Pmysql_stmt_error)(MYSQL_STMT * stmt);
            typedef const char* (STDCALL *Pmysql_stmt_sqlstate)(MYSQL_STMT * stmt);
            typedef MYSQL_ROW_OFFSET(STDCALL *Pmysql_stmt_row_seek)(MYSQL_STMT *stmt, MYSQL_ROW_OFFSET offset);
            typedef MYSQL_ROW_OFFSET(STDCALL *Pmysql_stmt_row_tell)(MYSQL_STMT *stmt);
            typedef void (STDCALL *Pmysql_stmt_data_seek)(MYSQL_STMT *stmt, my_ulonglong offset);
            typedef my_ulonglong(STDCALL *Pmysql_stmt_num_rows)(MYSQL_STMT *stmt);
            typedef my_ulonglong(STDCALL *Pmysql_stmt_affected_rows)(MYSQL_STMT *stmt);
            typedef my_ulonglong(STDCALL *Pmysql_stmt_insert_id)(MYSQL_STMT *stmt);
            typedef unsigned int (STDCALL *Pmysql_stmt_field_count)(MYSQL_STMT *stmt);
            typedef my_bool(STDCALL *Pmysql_commit)(MYSQL * mysql);
            typedef my_bool(STDCALL *Pmysql_rollback)(MYSQL * mysql);
            typedef my_bool(STDCALL *Pmysql_autocommit)(MYSQL * mysql, my_bool auto_mode);
            typedef my_bool(STDCALL *Pmysql_more_results)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_next_result)(MYSQL *mysql);
            typedef int (STDCALL *Pmysql_stmt_next_result)(MYSQL_STMT *stmt);
            typedef my_bool(STDCALL *Pmysql_stmt_reset)(MYSQL_STMT * stmt);
            typedef void (STDCALL *Pmysql_close)(MYSQL *sock);

            struct CMysqlLoader {
            private:
                HINSTANCE m_hMysql;
                CMysqlLoader(const CMysqlLoader&);
                CMysqlLoader& operator=(const CMysqlLoader&);

            public:

                CMysqlLoader() : m_hMysql(nullptr) {

                }

                virtual ~CMysqlLoader() {
                    if (m_hMysql) {
                        ::FreeLibrary(m_hMysql);
                        m_hMysql = nullptr;
                    }
                }

                void Unload() {
                    if (m_hMysql) {
                        ::FreeLibrary(m_hMysql);
                        m_hMysql = nullptr;
                    }
                }

                Pmysql_stmt_affected_rows mysql_stmt_affected_rows;
                Pmysql_stmt_insert_id mysql_stmt_insert_id;
                Pmysql_stmt_field_count mysql_stmt_field_count;
                Pmysql_commit mysql_commit;
                Pmysql_rollback mysql_rollback;
                Pmysql_autocommit mysql_autocommit;
                Pmysql_more_results mysql_more_results;
                Pmysql_next_result mysql_next_result;
                Pmysql_stmt_next_result mysql_stmt_next_result;
                Pmysql_close mysql_close;
                Pmysql_server_init mysql_server_init;
                Pmysql_server_end mysql_server_end;
                Pmysql_num_rows mysql_num_rows;
                Pmysql_num_fields mysql_num_fields;
                Pmysql_fetch_fields mysql_fetch_fields;
                Pmysql_field_count mysql_field_count;
                Pmysql_affected_rows mysql_affected_rows;
                Pmysql_insert_id mysql_insert_id;
                Pmysql_errno mysql_errno;
                Pmysql_error mysql_error;
                Pmysql_init mysql_init;
                Pmysql_real_connect mysql_real_connect;
                Pmysql_real_query mysql_real_query;
                Pmysql_use_result mysql_use_result;
                Pmysql_options mysql_options;
                Pmysql_free_result mysql_free_result;
                Pmysql_fetch_row mysql_fetch_row;
                Pmysql_fetch_lengths mysql_fetch_lengths;
                Pmysql_stmt_init mysql_stmt_init;
                Pmysql_stmt_prepare mysql_stmt_prepare;
                Pmysql_stmt_execute mysql_stmt_execute;
                Pmysql_stmt_fetch mysql_stmt_fetch;
                Pmysql_stmt_fetch_column mysql_stmt_fetch_column;
                Pmysql_stmt_param_count mysql_stmt_param_count;
                Pmysql_stmt_bind_param mysql_stmt_bind_param;
                Pmysql_stmt_bind_result mysql_stmt_bind_result;
                Pmysql_stmt_close mysql_stmt_close;
                Pmysql_stmt_result_metadata mysql_stmt_result_metadata;
                Pmysql_stmt_errno mysql_stmt_errno;
                Pmysql_stmt_error mysql_stmt_error;
                Pmysql_thread_init mysql_thread_init;
                Pmysql_thread_safe mysql_thread_safe;
                Pmysql_thread_end mysql_thread_end;
                Pmysql_debug mysql_debug;
                Pmysql_stmt_store_result mysql_stmt_store_result;
                Pmysql_thread_id mysql_thread_id;
                Pmysql_stmt_free_result mysql_stmt_free_result;
                Pmysql_stmt_attr_set mysql_stmt_attr_set;
                Pmysql_get_client_version mysql_get_client_version;
                Pmysql_query mysql_query;
                Pmysql_stmt_reset mysql_stmt_reset;

                bool LoadMysql() {
                    if (m_hMysql) {
                        return true;
                    }
#ifdef WIN32_64
                    m_hMysql = ::LoadLibraryW(L"libmysql.dll");
                    if (!m_hMysql)
                        m_hMysql = ::LoadLibraryW(L"libmariadb.dll");
#else
                    m_hMysql = ::dlopen("libmysqlclient.so", RTLD_LAZY);
                    if (!m_hMysql)
                        m_hMysql = ::dlopen("libmariadb.so", RTLD_LAZY);
#endif
                    if (!m_hMysql) {
                        return false;
                    }
                    mysql_server_init = (Pmysql_server_init)::GetProcAddress(m_hMysql, "mysql_server_init");
                    mysql_server_end = (Pmysql_server_end)::GetProcAddress(m_hMysql, "mysql_server_end");
                    mysql_num_rows = (Pmysql_num_rows)::GetProcAddress(m_hMysql, "mysql_num_rows");
                    mysql_num_fields = (Pmysql_num_fields)::GetProcAddress(m_hMysql, "mysql_num_fields");
                    mysql_fetch_fields = (Pmysql_fetch_fields)::GetProcAddress(m_hMysql, "mysql_fetch_fields");
                    mysql_field_count = (Pmysql_field_count)::GetProcAddress(m_hMysql, "mysql_field_count");
                    mysql_affected_rows = (Pmysql_affected_rows)::GetProcAddress(m_hMysql, "mysql_affected_rows");
                    mysql_insert_id = (Pmysql_insert_id)::GetProcAddress(m_hMysql, "mysql_insert_id");
                    mysql_errno = (Pmysql_errno)::GetProcAddress(m_hMysql, "mysql_errno");
                    mysql_error = (Pmysql_error)::GetProcAddress(m_hMysql, "mysql_error");
                    mysql_init = (Pmysql_init)::GetProcAddress(m_hMysql, "mysql_init");
                    mysql_stmt_fetch_column = (Pmysql_stmt_fetch_column)::GetProcAddress(m_hMysql, "mysql_stmt_fetch_column");
                    mysql_stmt_field_count = (Pmysql_stmt_field_count)::GetProcAddress(m_hMysql, "mysql_stmt_field_count");
                    mysql_stmt_result_metadata = (Pmysql_stmt_result_metadata)::GetProcAddress(m_hMysql, "mysql_stmt_result_metadata");
                    mysql_fetch_lengths = (Pmysql_fetch_lengths)::GetProcAddress(m_hMysql, "mysql_fetch_lengths");
                    mysql_fetch_row = (Pmysql_fetch_row)::GetProcAddress(m_hMysql, "mysql_fetch_row");
                    mysql_next_result = (Pmysql_next_result)::GetProcAddress(m_hMysql, "mysql_next_result");
                    mysql_free_result = (Pmysql_free_result)::GetProcAddress(m_hMysql, "mysql_free_result");
                    mysql_use_result = (Pmysql_use_result)::GetProcAddress(m_hMysql, "mysql_use_result");
                    mysql_real_query = (Pmysql_real_query)::GetProcAddress(m_hMysql, "mysql_real_query");
                    mysql_more_results = (Pmysql_more_results)::GetProcAddress(m_hMysql, "mysql_more_results");
                    mysql_stmt_next_result = (Pmysql_stmt_next_result)::GetProcAddress(m_hMysql, "mysql_stmt_next_result");
                    mysql_stmt_fetch = (Pmysql_stmt_fetch)::GetProcAddress(m_hMysql, "mysql_stmt_fetch");
                    mysql_stmt_execute = (Pmysql_stmt_execute)::GetProcAddress(m_hMysql, "mysql_stmt_execute");
                    mysql_stmt_bind_result = (Pmysql_stmt_bind_result)::GetProcAddress(m_hMysql, "mysql_stmt_bind_result");
                    mysql_stmt_bind_param = (Pmysql_stmt_bind_param)::GetProcAddress(m_hMysql, "mysql_stmt_bind_param");
                    mysql_stmt_affected_rows = (Pmysql_stmt_affected_rows)::GetProcAddress(m_hMysql, "mysql_stmt_affected_rows");
                    mysql_stmt_insert_id = (Pmysql_stmt_insert_id)::GetProcAddress(m_hMysql, "mysql_stmt_insert_id");
                    mysql_stmt_param_count = (Pmysql_stmt_param_count)::GetProcAddress(m_hMysql, "mysql_stmt_param_count");
                    mysql_stmt_close = (Pmysql_stmt_close)::GetProcAddress(m_hMysql, "mysql_stmt_close");
                    mysql_stmt_errno = (Pmysql_stmt_errno)::GetProcAddress(m_hMysql, "mysql_stmt_errno");
                    mysql_stmt_error = (Pmysql_stmt_error)::GetProcAddress(m_hMysql, "mysql_stmt_error");
                    mysql_stmt_prepare = (Pmysql_stmt_prepare)::GetProcAddress(m_hMysql, "mysql_stmt_prepare");
                    mysql_stmt_init = (Pmysql_stmt_init)::GetProcAddress(m_hMysql, "mysql_stmt_init");
                    mysql_autocommit = (Pmysql_autocommit)::GetProcAddress(m_hMysql, "mysql_autocommit");
                    mysql_rollback = (Pmysql_rollback)::GetProcAddress(m_hMysql, "mysql_rollback");
                    mysql_commit = (Pmysql_commit)::GetProcAddress(m_hMysql, "mysql_commit");
                    mysql_real_connect = (Pmysql_real_connect)::GetProcAddress(m_hMysql, "mysql_real_connect");
                    mysql_options = (Pmysql_options)::GetProcAddress(m_hMysql, "mysql_options");
                    mysql_close = (Pmysql_close)::GetProcAddress(m_hMysql, "mysql_close");
                    mysql_thread_init = (Pmysql_thread_init)::GetProcAddress(m_hMysql, "mysql_thread_init");
                    mysql_thread_safe = (Pmysql_thread_safe)::GetProcAddress(m_hMysql, "mysql_thread_safe");
                    mysql_thread_end = (Pmysql_thread_end)::GetProcAddress(m_hMysql, "mysql_thread_end");
                    mysql_debug = (Pmysql_debug)::GetProcAddress(m_hMysql, "mysql_debug");
                    mysql_stmt_store_result = (Pmysql_stmt_store_result)::GetProcAddress(m_hMysql, "mysql_stmt_store_result");
                    mysql_thread_id = (Pmysql_thread_id)::GetProcAddress(m_hMysql, "mysql_thread_id");
                    mysql_stmt_free_result = (Pmysql_stmt_free_result)::GetProcAddress(m_hMysql, "mysql_stmt_free_result");
                    mysql_stmt_attr_set = (Pmysql_stmt_attr_set)::GetProcAddress(m_hMysql, "mysql_stmt_attr_set");
                    mysql_get_client_version = (Pmysql_get_client_version)::GetProcAddress(m_hMysql, "mysql_get_client_version");
                    mysql_query = (Pmysql_query)::GetProcAddress(m_hMysql, "mysql_query");
                    mysql_stmt_reset = (Pmysql_stmt_reset)::GetProcAddress(m_hMysql, "mysql_stmt_reset");
                    return true;
                }

                bool IsLoaded() const {
                    return (m_hMysql != nullptr);
                }
            };

        } //namespace Mysql
    } //namespace ServerSide
} //namespace SPA

#endif