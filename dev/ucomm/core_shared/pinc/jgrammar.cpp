
#include "jgrammar.h"

namespace UJSON
{
  std::vector<CJsonGrammar*> CJsonGrammar::m_vGrammar;
  MB::CUCriticalSection CJsonGrammar::m_cs;

  unsigned int Count(const char *str, char c)
  {
	  if(str == NULL)
		  return 0;
	  unsigned int num = 0;
	  const char *pos = ::strchr(str, c);
	  while(pos)
	  {
		  ++num;
		  str = pos + 2;
		  pos = ::strchr(str, c);
	  }
	  return num;
  }

  parse_info<> CJsonGrammar::ParseJson(const char *json, JObject &jo, MB::CUQueue &qStr, MB::CUQueue &qObject, MB::CUQueue &qPair, MB::CUQueue &qArray, MB::CUQueue &qElement)
  {
    parse_info<> pi;
	if(json == NULL)
		return pi;

    jo.Count = 0;
	jo.Pairs = NULL;
	size_t len = ::strlen(json);

	unsigned int objCount = Count(json, '{');
	unsigned int arrayCount = Count(json, '[');
	unsigned int eleCount = Count(json, ',');
	unsigned int pairCount = Count(json, ':');

	qObject.SetSize(0);
	qPair.SetSize(0);
	qArray.SetSize(0);
	qElement.SetSize(0);
	qStr.SetSize(0);

	qStr.Push(json, (unsigned int)len + 1);
	unsigned int BufferSize = qObject.GetMaxSize();
	unsigned int expectedSize = (objCount + 2)*sizeof(JObject);
	if(BufferSize < expectedSize)
		qObject.ReallocBuffer(expectedSize);

	BufferSize = qPair.GetMaxSize();
	expectedSize = (pairCount + 2)*sizeof(JPair);
	if(BufferSize < expectedSize)
		qPair.ReallocBuffer(expectedSize);

	BufferSize = qArray.GetMaxSize();
	expectedSize = (arrayCount + 2)*sizeof(JArray);
	if(BufferSize < expectedSize)
		qArray.ReallocBuffer(expectedSize);

	BufferSize = qElement.GetMaxSize();
	expectedSize = (eleCount + objCount + arrayCount + pairCount + 2)*sizeof(JElement);
	if(BufferSize < expectedSize)
		qElement.ReallocBuffer(expectedSize);

    CJsonGrammar *p = NULL;
    m_cs.lock();
    if (m_vGrammar.size()) {
      p = m_vGrammar.back();
      m_vGrammar.pop_back();
    }
    m_cs.unlock();
    if(!p)
      p = new CJsonGrammar;

    p->m_pRoot = &jo;
	p->m_pArray = &qArray;
	p->m_pObject = &qObject;
	p->m_pPair = &qPair;
	p->m_pStr = &qStr;
	p->m_pElement = &qElement;
	p->m_start = json;

	//pi = boost::spirit::classic::parse(json, *p, space_p|comment_p("//")|comment_p("/*", "*/")); //support of comments will make parsing slower
    pi = boost::spirit::classic::parse(json, json + len, *p, space_p);
    m_cs.lock();
    m_vGrammar.push_back(p);
    m_cs.unlock();
    return pi;
  }
};

