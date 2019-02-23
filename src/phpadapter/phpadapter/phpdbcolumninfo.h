#ifndef _PHP_DB_COLUMN_INFO_H_
#define _PHP_DB_COLUMN_INFO_H_

namespace PA {

    class CPhpDBColumnInfo : public Php::Base {
    public:
        CPhpDBColumnInfo();
        CPhpDBColumnInfo(const SPA::UDB::CDBColumnInfo &ColInfo);
        CPhpDBColumnInfo(const CPhpDBColumnInfo &ColInfo) = delete;

    public:
        CPhpDBColumnInfo& operator=(const CPhpDBColumnInfo &ColInfo) = delete;
        static void RegisterInto(Php::Namespace &cs);
        Php::Value __get(const Php::Value &name);
        void __destruct();

    private:
        void __construct(Php::Parameters &params);

    private:
        SPA::UDB::CDBColumnInfo m_ColInfo;
    };

} //namespace PA

#endif