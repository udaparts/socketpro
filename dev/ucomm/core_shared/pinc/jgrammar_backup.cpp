
#include "jgrammar.h"

namespace UJSON
{
  vector<CJsonGrammar*> CJsonGrammar::m_vGrammar;
  MB::CUCriticalSection CJsonGrammar::m_cs;

  ValueType GetType(const JValue &jv)
  {
    return (ValueType)jv.which();   
  }

  parse_info<> CJsonGrammar::ParseJson(const char *json, JObject &jo)
  {
    parse_info<> pi;
    jo.clear();
    CJsonGrammar *p = NULL;
    m_cs.lock();
    if (m_vGrammar.size()) {
      p = m_vGrammar.back();
      m_vGrammar.pop_back();
    }
    m_cs.unlock();
    if(!p)
      p = new CJsonGrammar;
    p->m_pJSONObject = &jo;
	  //pi = boost::spirit::classic::parse(json, *p, space_p|comment_p("//")|comment_p("/*", "*/")); //support of comments will make parsing slower
    pi = boost::spirit::classic::parse(json, *p, space_p);
    m_cs.lock();
    m_vGrammar.push_back(p);
    m_cs.unlock();
    return pi;
  }
};