

#pragma once

#include "membuffer.h"
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
	enum ValueType{vtNull = 0, vtBool, vtInt64, vtDouble, vtString, vtArray, vtObject};
	struct JObject;
	struct JArray;

	union UElement
	{
		bool b;
		const char *str;
		MB::U_INT64 number;
		double d;
		JObject *obj;
		JArray *arr;
	};

	struct JElement
	{
		JElement() : Type(vtNull) {U.number = 0;}
		ValueType Type;
		UElement U;
	};

	typedef JElement *PJElement;

	struct JPair
	{
		JPair() : Name(NULL), Value(NULL) {}
		const char *Name;
		JElement *Value;
	};
	typedef JPair *PJPair;

	struct JObject
	{
		JObject() : Count(0), Pairs(NULL) {}
		unsigned int Count;
		PJPair *Pairs;
	};

	struct JArray
	{
		JArray() : Count(0), Elements(NULL) {}
		unsigned int Count;
		PJElement *Elements;
	};

	using namespace boost::spirit::classic;
	const int_parser < boost::int64_t >  int64_p  = int_parser < boost::int64_t  >();
  
	typedef boost::function<void(const char*, const char*) > CStrAction;
	typedef boost::function<void(boost::int64_t) > CInt64Action;
	typedef boost::function<void(boost::uint64_t) > CUInt64Action;
	typedef boost::function<void(int) > CIntAction;
	typedef boost::function<void(double) > CDoubleAction;
	typedef boost::function<void(char)> CCharAction;

struct CJsonGrammar : public grammar<CJsonGrammar>, private boost::noncopyable {
	
	JObject *m_pRoot;
	MB::CUQueue *m_pStr;
	MB::CUQueue	*m_pObject;
	MB::CUQueue	*m_pPair;
	MB::CUQueue	*m_pArray;
	MB::CUQueue *m_pElement;
	const char *m_start;

	template <typename ScannerT>
	class definition 
	{
		typedef rule<ScannerT> CRule;

	public:
		definition(CJsonGrammar const& jg)
			: m_pRoot(NULL), m_pStr(NULL), m_pObject(NULL), 
			m_pPair(NULL), m_pArray(NULL), m_pElement(NULL), 
			m_bRoot(true), m_curObj(NULL), m_curPair(NULL), 
			m_start(NULL), m_jg(jg)
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
			if(m_bRoot)
			{
				m_curObj = m_pRoot;
				m_bRoot = false;
			}
			else
			{
				JObject jo;
				unsigned int len = m_pObject->GetSize();
				*m_pObject << jo;
				m_curObj = (JObject*)m_pObject->GetBuffer(len);
			}
		}

		inline void EndObject(char c)
		{
        
		}

		inline void NewArray(char c)
		{
       
		}

		inline void EndArray(char c)
		{
        
		}

		inline void NewInt64(boost::int64_t data)
		{
			JElement int64;
			int64.Type = vtInt64;
			int64.U.number = data;
		}

		inline void NewDouble(double data)
		{
			JElement d;
			d.Type = vtDouble;
			d.U.d = data;
		}

		inline void NewTrue(const char *s, const char *e)
		{
			JElement jTrue;
			jTrue.Type = vtBool;
			jTrue.U.b = true;
		}

		inline void NewFalse(const char *s, const char *e)
		{
			JElement jFalse;
			jFalse.Type = vtBool;
		}

		inline void NewNull(const char *s, const char *e)
		{
			JElement null;
		
		}

		inline void NewName(const char *s, const char *e)
		{
			unsigned int begin = (unsigned int)(s - m_start) + 1;
			char *start = (char*)m_pStr->GetBuffer(begin);
			JPair jp;
			unsigned int len = m_pPair->GetSize();
			*m_pPair << jp;
			m_curPair = (JPair*)m_pPair->GetBuffer(len);
			len = (unsigned int)(e - s) - 2;
			start[len] = 0;
			m_curPair->Name = start;
		}

		inline void NewString(const char *s, const char *e)
		{
			unsigned int begin = (unsigned int)(s - m_start) + 1;
			char *start = (char*)m_pStr->GetBuffer(begin);
			unsigned int len = (unsigned int)(e - s) - 2;
			start[len] = 0;
			JElement str;
			str.Type = vtString;
			str.U.str = start;
		}

	public:
		CRule const& start() {
			m_pRoot = m_jg.m_pRoot;
			m_pStr = m_jg.m_pStr;
			m_pObject = m_jg.m_pObject;
			m_pPair = m_jg.m_pPair;
			m_pArray = m_jg.m_pArray;
			m_pElement = m_jg.m_pElement;
			m_start = m_jg.m_start;
			m_bRoot = true;
			m_curObj = NULL;
			m_curPair = NULL;
			return json_;
		}
	private:
		CRule json_, pair_, array_, value_, string_, number_;
		//CRule members_, elements_;
	private:
		JObject *m_pRoot;
		MB::CUQueue *m_pStr;
		MB::CUQueue	*m_pObject;
		MB::CUQueue	*m_pPair;
		MB::CUQueue	*m_pArray;
		MB::CUQueue	*m_pElement;
		bool m_bRoot;
		JObject *m_curObj;
		JPair *m_curPair;
		const char *m_start;
		const CJsonGrammar& m_jg;
	};

	public:
		static parse_info<> ParseJson(const char *json, JObject &jo, MB::CUQueue &qStr, MB::CUQueue &qObject, MB::CUQueue &qPair, MB::CUQueue &qArray, MB::CUQueue &qElement);

	private:
		CJsonGrammar() : m_pRoot(NULL), m_pStr(NULL), m_pObject(NULL), m_pPair(NULL), m_pArray(NULL), m_pElement(NULL), m_start(NULL) {}

	private:
		static MB::CUCriticalSection  m_cs;
		static std::vector<CJsonGrammar*>  m_vGrammar;
	};
};







