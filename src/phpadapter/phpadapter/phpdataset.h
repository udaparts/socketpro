
#ifndef SPA_PHP_DATA_SET_H
#define SPA_PHP_DATA_SET_H

namespace PA {

    class CPhpDataSet : public Php::Base {
    public:
        CPhpDataSet(SPA::CDataSet &ds);
        CPhpDataSet(const CPhpDataSet &ds) = delete;

    public:
        CPhpDataSet& operator=(const CPhpDataSet &ds) = delete;
        static void RegisterInto(Php::Namespace &spa);
        Php::Value __get(const Php::Value &name);
        static void ToArray(const Php::Value &v, std::vector<CComVariant> &vData);
        static void ToArray(const Php::Value &v, SPA::UDB::CDBVariantArray &vData);
        static void CheckResult(size_t res);
        void __destruct();

    private:
        void __construct(Php::Parameters &params);
        void AddEmptyRowset(Php::Parameters &params);
        void AddRows(Php::Parameters &params);
        Php::Value GetColumMeta(Php::Parameters &params);
        Php::Value GetRowCount(Php::Parameters &params);
        Php::Value GetColumnCount(Php::Parameters &params);
        Php::Value FindKeys(Php::Parameters &params);
        Php::Value FindOrdinal(Php::Parameters &params);
        Php::Value UpdateARow(Php::Parameters &params);
        Php::Value DeleteARow(Php::Parameters &params);
        Php::Value Between(Php::Parameters &params);
        Php::Value FindNull(Php::Parameters &params);
        Php::Value In(Php::Parameters &params);
        Php::Value NotIn(Php::Parameters &params);
        Php::Value Find(Php::Parameters &params);
        void Empty();

    private:
        SPA::CDataSet &m_ds;
    };

} //namespace PA

#endif