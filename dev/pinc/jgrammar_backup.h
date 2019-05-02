
#pragma once

#include "commutil.h"
#include <boost/variant.hpp>
#include <unordered_map>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <vector>

/*
  Performance: it is about 50% faster than .NET System.Web.Script.Serialization.JavaScriptSerializer

*/

namespace UJSON
{
  using namespace std;
  using namespace boost::spirit::classic;
  const int_parser < boost::int64_t >  int64_p  = int_parser < boost::int64_t  >();
  const uint_parser< boost::uint64_t > uint64_p = uint_parser< boost::uint64_t >();
  
  typedef boost::function<void(const char*, const char*) > CStrAction;
  typedef boost::function<void(boost::int64_t) > CInt64Action;
  typedef boost::function<void(boost::uint64_t) > CUInt64Action;
  typedef boost::function<void(int) > CIntAction;
  typedef boost::function<void(double) > CDoubleAction;
  typedef boost::function<void(char)> CCharAction;

  enum ValueType{ vtNull = 0, vtBool, vtInt64, vtDouble, vtString, vtObject, vtArray };

  struct JNull{};
  struct JObject;

  typedef boost::make_recursive_variant<JNull, bool, boost::int64_t, double, string, JObject, vector<boost::recursive_variant_> >::type JValue;

  struct JObject : public unordered_map<string, JValue>
  {
    JObject() : ParentObject(NULL), ParentArray(NULL) {}
    JObject *ParentObject;
    vector<JValue> *ParentArray;
  };

  ValueType GetType(const JValue &v);
  
  struct CJsonGrammar : public grammar<CJsonGrammar>, private boost::noncopyable {
    JObject *m_pJSONObject;
    template <typename ScannerT>
    class definition {
      typedef rule<ScannerT> CRule;
    public:
      definition(CJsonGrammar const& jg)
        : m_jg(jg), 
          m_pArray(NULL), 
          m_pJSONObject(NULL), 
          m_pJSon(NULL)
      {
        CCharAction beginObject(boost::bind(&definition::NewObject, this, _1));
        CCharAction endObject(boost::bind(&definition::EndObject, this, _1));
        CInt64Action newInt64(boost::bind(&definition::NewInt64, this, _1));
        CDoubleAction newDouble(boost::bind(&definition::NewDouble, this, _1));
        CCharAction beginArray(boost::bind(&definition::NewArray, this, _1));
        CCharAction endArray(boost::bind(&definition::EndArray, this, _1));
        CStrAction newNull(boost::bind(&definition::NewNull, this, _1, _2));
        CStrAction newTrue(boost::bind(&definition::NewTrue, this, _1, _2));
        CStrAction newFalse(boost::bind(&definition::NewFalse, this, _1, _2));
        CStrAction newName(boost::bind(&definition::NewName, this, _1, _2));
        CStrAction newString(boost::bind(&definition::NewString, this, _1, _2));

        json_ = ch_p('{')[beginObject] >> !(pair_ % ch_p(',')) >> ch_p('}')[endObject];
        pair_ = string_[newName] >> ':' >> value_;
        value_ = string_[newString]
				| number_ 
				| json_ 
				| array_
				| str_p("true")[newTrue] 
				| str_p("false")[newFalse]
				| str_p("null" )[newNull]
				;

        array_ = ch_p('[')[beginArray] >> !((value_ - array_) % ch_p(',')) >> ch_p(']')[endArray];

        //((value_ - array_) % ch_p(',')); //not support recursive array -- [1, 2, []]

        string_ = lexeme_d // this causes white space inside a string to be retained
					[
						confix_p
							( 
							'"', 
							*lex_escape_ch_p,
							'"'
							)  
					]
					;

		    //number_ = (strict_real_p[newDouble]|int_p[newInt]|int64_p[newInt64]|uint64_p[newUInt64]); //adding extra rules will slow parsing
        number_ = (strict_real_p[newDouble]|int64_p[newInt64]);
      }

    private:
      inline void NewObject(char c)
      {
        if(m_pJSon)
        {
          JObject jo;
          jo.ParentArray = m_pArray;
          jo.ParentObject = m_pJSon;
          if(m_pArray)
          {
            m_pArray->push_back(jo);
            JObject &json = boost::get<JObject>(m_pArray->back());
            m_pJSon = &json;
            m_pArray = NULL;
          }
          else
          {
            m_pair->second = jo;
            JObject &json = boost::get<JObject>(m_pair->second);
            m_pJSon = &json;
          }
        }
        else
        {
          m_pJSon = m_pJSONObject;
        }
      }

      inline void EndObject(char c)
      {
        m_pArray = m_pJSon->ParentArray;
        m_pJSon = m_pJSon->ParentObject;  
      }

      inline void NewArray(char c)
      {
        vector<JValue> a;
        m_pair->second = a;  
        vector<JValue> &myArray = boost::get < vector<JValue> > (m_pair->second);
        m_pArray = &myArray;
      }

      inline void EndArray(char c)
      {
        m_pArray = NULL;
      }

      inline void NewInt64(boost::int64_t data)
      {
        if(m_pArray)
          m_pArray->push_back(data);
        else
          m_pair->second = data;
      }

      inline void NewDouble(double data)
      {
         if(m_pArray)
          m_pArray->push_back(data);
        else
          m_pair->second = data;
      }

      inline void NewTrue(const char *s, const char *e)
      {
         if(m_pArray)
          m_pArray->push_back(true);
        else
          m_pair->second = true;
      }

      inline void NewFalse(const char *s, const char *e)
      {
        if(m_pArray)
          m_pArray->push_back(false);
        else
          m_pair->second = false;
      }

      inline void NewNull(const char *s, const char *e)
      {
        if(m_pArray)
        {
          JNull null;
          m_pArray->push_back(null);
        }
      }

      inline void NewName(const char *s, const char *e)
      {
        string name(s + 1, e - 1);
        JNull null;
        (*m_pJSon)[name] = null;
        m_pair = m_pJSon->find(name);
      }

      inline void NewString(const char *s, const char *e)
      {
        string str(s + 1, e - 1);
        if(m_pArray)
          m_pArray->push_back(str);
        else
          m_pair->second = str;
      }

    public:
      CRule const& start() {
          m_pJSONObject = m_jg.m_pJSONObject;
          return json_;
      }
    private:
      CRule json_, pair_, array_, value_, string_, number_;
	    //CRule members_, elements_;

    private:
      unordered_map<string, JValue>::iterator m_pair;
      vector<JValue> *m_pArray;
      const CJsonGrammar& m_jg;
      JObject *m_pJSONObject;
      JObject *m_pJSon;
    };

  public:
    static parse_info<> ParseJson(const char *json, JObject &jo);

  private:
    CJsonGrammar() : m_pJSONObject(NULL){}

  private:
    static MB::CUCriticalSection  m_cs;
    static vector<CJsonGrammar*>  m_vGrammar;
  };
};





