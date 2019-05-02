// ujson.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../../include/membuffer.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include "../../pinc/document.h"		// rapidjson's DOM-style API
#include <cstdio>

#ifdef MB32_64
#include "../../pinc/yajl_tree.h"
#endif


using namespace std;

//max unsigned long long = 18446744073709551615
const char *str0 = "{\"name\":\"john\", \"age\":41, \"MyArray\":[1, 2, 3.5, true, false, {\"a\":[1, 2, 3.5, null, false]}, {}, null, \"test\"], \"testobj\":{\"myarray\":[]}}";

void DoRapidParse() {
    SPA::CScopeUQueue su;
    int n, count = 2000000;

    try {
        for (n = 0; n < count; ++n) {
            su->Push(str0);
            su->SetNull();
            {
                rapidjson::Document doc;
                const rapidjson::Document::ValueType &v = doc.ParseInsitu < 0 > ((char*) su->GetBuffer());
                rapidjson::SizeType size = doc.MemberSize();
                size = v.MemberSize();
                bool err = doc.HasParseError();
                assert(doc["age"].GetInt() == 41);
                const rapidjson::Value& a = doc["MyArray"];
                size = a.Size();
                assert(a[1].GetInt() == 2);
                assert(a[2].GetDouble() == 3.5);
                assert(a[3].GetBool());
                const rapidjson::Value &myarray = doc["testobj"]["myarray"];
                size = myarray.Size();
                const rapidjson::Value &ak = a[5]["a"];
                size = ak.Size();
                assert(ak[1].GetInt() == 2);
                assert(ak[2].GetDouble() == 3.5);
                assert(ak[3].IsNull());
                assert(!ak[4].GetBool());

                auto end = v.MemberEnd();
                for (auto it = v.MemberBegin(); it != end; ++it) {
                    const char *mem = it->name.GetString();
                    auto &val = it->value;
                    switch (val.GetType()) {
                        case rapidjson::kNullType:
                            break;
                        case rapidjson::kArrayType:
                            break;
                        case rapidjson::kFalseType:
                            break;
                        case rapidjson::kNumberType:
                            break;
                        case rapidjson::kObjectType:
                            break;
                        case rapidjson::kStringType:
                            break;
                        case rapidjson::kTrueType:
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    mem = NULL;
                }
            }
            su->SetSize(0);
        }
    } catch (std::exception &ex) {
        cout << "error = " << ex.what() << endl;
    } catch (...) {
        cout << "Unknown error = " << endl;
    }
}

#ifdef MB32_64

void DoParseYajl() {
    int n, count = 2000000;
    yajl_val jv;
    char ErrorBuffer[1024] = {0};
    try {
        for (n = 0; n < count; ++n) {
            jv = yajl_tree_parse(str0, ErrorBuffer, sizeof (ErrorBuffer));
            YJObject *jo = YAJL_GET_OBJECT(jv);
            /*
            size_t m, len = jo->len;
            for(m=0; m<len; ++m)
            {
                    const char *name = jo->keys[m];
                    yajl_val val = jo->values[m];
            }
             */
            yajl_tree_free(jv);
        }
    } catch (std::exception &ex) {
        cout << "error = " << ex.what() << endl;
    } catch (...) {
        cout << "Unknown error = " << endl;
    }
}

#endif

int main(int argc, char* argv[]) {
    boost::thread_group parse_threads;
    unsigned int n = 1;

    unsigned int cpu_count = boost::thread::hardware_concurrency();
    boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();


    /*for (n = 0; n < cpu_count; ++n) {
            parse_threads.create_thread(&DoRapidParse);
    }
    parse_threads.join_all();*/


    //DoParseYajl();

    DoRapidParse();


    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = t1 - t0;
    cout << "Time required = " << diff.total_milliseconds() << ", count = " << n << endl;

    cin >> n;
    return 0;
}



