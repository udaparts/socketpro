#ifndef SPA_PHP_DATA_TABLE_H
#define SPA_PHP_DATA_TABLE_H

namespace PA {

    class CPhpTable : public Php::Base {
    public:
        CPhpTable(std::shared_ptr<SPA::CTable> table);

    public:
        static void RegisterInto(Php::Namespace &spa);
        Php::Value __get(const Php::Value &name);
        void __destruct();

    private:
        void __construct(Php::Parameters &params);
        Php::Value FindOrdinal(Php::Parameters &params);
        Php::Value Between(Php::Parameters &params);
        Php::Value FindNull(Php::Parameters &params);
        Php::Value In(Php::Parameters &params);
        Php::Value NotIn(Php::Parameters &params);
        Php::Value Find(Php::Parameters &params);
        Php::Value Sort(Php::Parameters &params);
        Php::Value Append(Php::Parameters &params);

    private:
        std::shared_ptr<SPA::CTable> m_table;
    };

} //namespace PA 

#endif