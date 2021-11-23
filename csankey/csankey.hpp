/* csankey.cpp | MIT License | https://github.com/kirin123kirin/csankey/raw/main/LICENSE */
/* parser.hpp | MIT License | https://github.com/AriaFallah/csv-parser/blob/master/LICENSE */

#pragma once
#ifndef CSANKEY_HPP
#define CSANKEY_HPP

#include <Python.h>
#include <fstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include "../extern/csv-parser/parser.hpp"

#if !(_WIN32 || _WIN64)
#include <ext/string_conversions.h>
#include <limits>
#endif

static constexpr const wchar_t BEFORE_TEXT[] =
#include "bf.cc"
;

static constexpr const wchar_t AFTER_TEXT[] =
#include "af.cc"
;

template <class T>
struct PyMallocator {
    typedef T value_type;

    PyMallocator() = default;
    template <class U>
    constexpr PyMallocator(const PyMallocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        if(n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();
        if(auto p = PyMem_New(T, n)) {
            // report(p, n);
            return p;
        }
        throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t n) noexcept {
        PyMem_Del(p);
        ;
    }

    bool operator==(const PyMallocator<T>&) { return true; }

    bool operator!=(const PyMallocator<T>&) { return false; }

    //    private:
    //     void report(T* p, std::size_t n, bool alloc = true) const {
    //         std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n << " bytes at " << std::hex <<
    //         std::showbase
    //                   << reinterpret_cast<void*>(p) << std::dec << '\n';
    //     }
};

template <typename CharT, typename Container = std::vector<std::vector<std::basic_string<CharT>>>>
class SankeyData {
   public:
    using row_type = typename Container::value_type;

    Container data;
    std::unordered_set<std::basic_string<CharT>> nodes;
    std::unordered_map<std::basic_string<CharT>, int> links;
    CharT delimiter = TYPED_LITERAL(CharT, ',');
    CharT quote = TYPED_LITERAL(CharT, '"');

   private:
    bool parsed;

    const char* errmsg = "argument is 2d list or tuple object?";

   public:
    // SankeyData() : data(), nodes({}), links({}), parsed(false) {}
    // SankeyData(std::nullptr_t) : data(), nodes({}), links({}), parsed(false) {}
    SankeyData(Container _data) : data(_data), nodes({}), links({}), parsed(false) {}
    ~SankeyData() {}

   private:
    void _table_parse(bool header, std::size_t needcolsize, std::size_t srcidx, std::size_t taridx) {
        std::size_t _nlen;

        int i = 0;
        for(auto&& row : data) {
            if(i++ == 0 && header)
                continue;

            int val = 1;
            if(row.empty())
                throw std::runtime_error("UnExpected Error. Empty row Found.");

            if((_nlen = row.size()) != needcolsize)
                throw std::runtime_error("UnExpected column size Error. expected columns num.\n");

            if(_nlen > 2) {
                auto&& c = row[taridx + 1];
                if(c.size() &&
                   c.find_first_not_of(TYPED_LITERAL(CharT*, "0123456789")) == std::basic_string<CharT>::npos)
                    val = std::stoi(row[taridx + 1]);
            }
            mapper(row[srcidx], row[taridx], val);
        }
    }

    void mapper(const std::basic_string<CharT>& src,
                const std::basic_string<CharT>& tar,
                int value = 1) {  //_append_data
        if(src.empty() && tar.empty()) {
            throw std::runtime_error("Parse string Error.");
        } else if(src.empty()) {
            if(nodes.find(tar) == nodes.end())
                nodes.emplace(tar);
        } else if(tar.empty()) {
            if(nodes.find(src) == nodes.end())
                nodes.emplace(src);
        } else {
            std::basic_string<CharT> key(TYPED_LITERAL(CharT*, R"(,"source":")"));
            key += src;
            key += TYPED_LITERAL(CharT*, R"(","target":")");
            key += tar;
            key += TYPED_LITERAL(CharT*, R"(","value":)");

            links[key] += value;
            if(nodes.find(src) == nodes.end())
                nodes.emplace(src);
            if(nodes.find(tar) == nodes.end())
                nodes.emplace(tar);
        }
    }

   public:
    std::basic_string<CharT> to_json() {
        if(parsed == false)
            throw std::runtime_error("yet parse data....");

        std::basic_string<CharT> res(TYPED_LITERAL(CharT*, "{\n\"nodes\":[\n"));
        std::size_t i = 0, j = 0;

        for(auto&& node : nodes) {
            res += TYPED_LITERAL(CharT*, R"({"ID":)");
#if _WIN32 || _WIN64
            res += std::_UIntegral_to_string<CharT>(++i);
#else
            // res += __gnu_cxx::__to_xstring<CharT>(++i); //@todo error: no matching function for call to '__gnu_cxx::__to_xstring'
            res += std::to_wstring(++i);
#endif
            res += TYPED_LITERAL(CharT*, R"(,"name":")") + node + TYPED_LITERAL(CharT*, "\"},\n");
        }
        res += TYPED_LITERAL(CharT*, "],\n\"links\":[\n");

        for(auto&& link : links) {
            res += TYPED_LITERAL(CharT*, R"({"ID":)");
#if _WIN32 || _WIN64
            res += std::_UIntegral_to_string<CharT>(++j);
#else
            // res += __gnu_cxx::__to_xstring<CharT>(++j); //@todo error: no matching function for call to '__gnu_cxx::__to_xstring'
            res += std::to_wstring(++j);
#endif
            res += link.first;
#if _WIN32 || _WIN64
            res += std::_Integral_to_string<CharT>(link.second);
#else
            // res += __gnu_cxx::__to_xstring<CharT>(link.second); //@todo error: no matching function for call to '__gnu_cxx::__to_xstring'
            res += std::to_wstring(link.second);
#endif
            res += TYPED_LITERAL(CharT*, "},\n");
        }
        return res + TYPED_LITERAL(CharT*, "]}\n");
    }

    void parse() {
        std::size_t i, j, _nlen, c = 0;
        parsed = true;

        for(auto&& row : data) {
            _nlen = row.size();

            for(i = 0; i < _nlen; ++i) {
                j = i;
                std::basic_string<CharT> src, tar, vs;

                if((_nlen = row.size()) == 0)
                    continue;

                while(j < _nlen) {
                    vs = row[j];
                    if(!vs.empty())
                        src.swap(vs);

                    if(++j == _nlen)
                        break;

                    tar = row[j];
                    if(!tar.empty()) {
                        ++j;
                        break;
                    }
                }
                if(!src.empty()) {
                    mapper(src, tar);
                    ++c;
                }
            }
        }

        if (c == 0)
            throw std::runtime_error("Cannot Parse Empty Data.\n");
    }

    void parse(bool header) {
        std::size_t nlen = std::size_t(-1);
        parsed = true;

        if((nlen = (*data.begin()).size()) == 0)
            return;

        if(nlen == 2 || nlen == 3) {
            return _table_parse(header, nlen, 0, 1);
        } else if(nlen == 4) {
            return _table_parse(header, nlen, 1, 2);
        } else {
            throw std::runtime_error("If you want to use this feature, at least 2 - 4 columns are needed.\n");
        }
    }

    void _locale(const char* codepage = "") {
        std::ios_base::sync_with_stdio(false);
        std::locale default_loc(codepage);
        std::locale::global(default_loc);
        std::locale ctype_default(std::locale::classic(), default_loc, std::locale::ctype);
        std::cout.imbue(ctype_default);
        std::wcout.imbue(ctype_default);
        std::cin.imbue(ctype_default);
        std::wcin.imbue(ctype_default);
    }

    int to_html(const std::string& filename, const char* codepage = "Japanese_Japan.65001") {
        _locale(codepage);
        std::basic_ofstream<CharT> wf(filename, std::basic_ios<CharT>::out);
        if(!wf.good())
            throw std::runtime_error("Fail Writing File Ready.");

        wf << BEFORE_TEXT;
        wf << this->to_json();
        wf << AFTER_TEXT << std::endl;

        wf.close();
        return 0;
    }

    int to_html(const char* codepage = "Japanese_Japan.932") {
        _locale(codepage);
        std::wcout << BEFORE_TEXT;
        std::wcout << this->to_json();
        std::wcout << AFTER_TEXT << std::endl;
        return 0;
    }
};

template <>
class SankeyData<wchar_t, PyObject*> {
   public:
    PyObject* data;
    Py_ssize_t len;
    std::unordered_set<std::wstring> nodes;
    std::unordered_map<std::wstring, int> links;

   private:
    bool parsed = false;

   public:
    SankeyData() : data(NULL), len(-1), nodes({}), links({}) {}
    SankeyData(std::nullptr_t) : data(NULL), len(-1), nodes({}), links({}) {}
    SankeyData(PyObject*& _py2darraydata)
        : data(_py2darraydata), len(PyObject_Length(_py2darraydata)), nodes({}), links({}) {}
    ~SankeyData() {}

    std::wstring to_json() {
        if(!parsed) {
            PyErr_Format(PyExc_IndexError, "yet Parsed. Please to_json function Call After Parsed");
            return L"";
        }

        std::wstring res(L"{\n\"nodes\":[\n");
        std::size_t i = 0, j = 0;

        for(auto&& node : nodes) {
            res += LR"({"ID":)";
            res += std::to_wstring(++i);
            res += LR"(,"name":")" + node + L"\"},\n";
        }
        res += L"],\n\"links\":[\n";

        for(auto&& link : links) {
            res += LR"({"ID":)";
            res += std::to_wstring(++j);
            res += link.first;
            res += std::to_wstring(link.second);
            res += L"},\n";
        }
        return res + L"]}\n";
    }

    bool parse() {
        Py_ssize_t _nlen, c = 0;
        const char* errmsg = "argument is 2d list or tuple object?";
        parsed = true;

        if(len == -1) {
            PyErr_Format(PyExc_IndexError, errmsg);
            return false;
        } else if(len == 0) {
            PyErr_Format(PyExc_ValueError, "argument list is empty data.");
            return false;
        }

        for(Py_ssize_t n = 0; n < len; ++n) {
            PyObject* row = PySequence_GetItem(data, n);
            Py_ssize_t i, j;

            if(row == NULL) {
                return false;
            }

            if((_nlen = PyObject_Length(row)) == -1) {
                Py_DECREF(row);
                return false;
            }

            for(i = 0; i < _nlen; ++i) {
                j = i;
                PyObject *src = NULL, *tar = NULL, *vs = NULL;

                while(j < _nlen) {
                    vs = PySequence_GetItem(row, j);
                    if(vs && (PyObject_IsTrue(vs) || PyObject_RichCompareBool(vs, Py_False, Py_EQ)))
                        std::swap(src, vs);

                    if(++j == _nlen)
                        break;

                    tar = PySequence_GetItem(row, j);
                    if(tar && (PyObject_IsTrue(tar) || PyObject_RichCompareBool(tar, Py_False, Py_EQ))) {
                        ++j;
                        break;
                    }
                }

                if(src && (PyObject_IsTrue(src) || PyObject_RichCompareBool(src, Py_False, Py_EQ))) {
                    mapper(src, tar);
                    ++c;
                }

                if(src)
                    Py_CLEAR(src);
                if(tar)
                    Py_CLEAR(tar);
                if(vs)
                    Py_CLEAR(vs);
            }

            Py_DECREF(row);
        }

        if(c == 0) {
            PyErr_Format(PyExc_ValueError, "argument list is empty data.");
            return false;
        }

        return true;
    }

    bool parse(bool header) {
        PyObject* row;
        const char* errmsg = "argument is 2d list or tuple object?";
        parsed = true;
        Py_ssize_t nlen = -1;

        if(len == -1) {
            PyErr_Format(PyExc_IndexError, errmsg);
            return false;
        } else if(len == 0) {
            PyErr_Format(PyExc_ValueError, "argument list is empty data.");
            return false;
        }

        if((row = PySequence_GetItem(data, 0)) == NULL) {
            Py_DECREF(row);
            return false;
        }
        if((nlen = PyObject_Length(row)) == -1) {
            Py_DECREF(row);
            return false;
        }
        Py_DECREF(row);

        if(nlen == 2 || nlen == 3) {
            return _table_parse(header, nlen, 0, 1);
        } else if(nlen == 4) {
            return _table_parse(header, nlen, 1, 2);
        } else {
            PyErr_Format(PyExc_ValueError,
                         "If you want to use this feature, at least 2 - 4 columns are needed.\n"
                         "But you input %d columns",
                         nlen);
            return false;
        }
    }

    PyObject* to_html() {
        PyObject* res = NULL;
        wchar_t *ret, *p;
        std::size_t json_len, bf_len, af_len, wsize, datasize;

        std::wstring jsondata = to_json();

        if((json_len = jsondata.size()) == 0)
            return NULL;

        wsize = sizeof(wchar_t);
        bf_len = (sizeof(BEFORE_TEXT) / wsize) - 1;
        af_len = (sizeof(AFTER_TEXT) / wsize) - 1;
        datasize = json_len + bf_len + af_len;

        {
            /* Make Faster PyUnicode Object Make. */
            if((res = PyUnicode_New((Py_ssize_t)datasize, (Py_UCS4)(wsize == 2 ? 65535 : 1114111))) == NULL)
                return PyErr_Format(PyExc_MemoryError, "Unknow Error.");

            if((ret = (wchar_t*)PyUnicode_DATA(res)) == NULL) {
                Py_CLEAR(res);
                return PyErr_Format(PyExc_MemoryError, "Unknow Error.");
            }
            p = ret;
        }

        {
            /* Befor + json + After Writing */
            if(wsize * datasize < wsize * bf_len) {
                Py_CLEAR(res);
                return PyErr_Format(PyExc_MemoryError, "Error. before_text data memory writing");
            }
            memcpy(p, BEFORE_TEXT, wsize * bf_len);
            p += bf_len;

            if(wsize * (datasize - bf_len) < wsize * json_len) {
                Py_CLEAR(res);
                return PyErr_Format(PyExc_MemoryError, "Error. after_text data memory writing");
            }
            memcpy(p, jsondata.data(), wsize * json_len);
            p += json_len;

            if(wsize * af_len < wsize * af_len){
                Py_CLEAR(res);
                return PyErr_Format(PyExc_MemoryError, "Error. after_text data memory writing");
            }
            memcpy(p, AFTER_TEXT, wsize * af_len);
        }

        return res;
    }

   private:
    std::wstring pyto_wstring(PyObject* o) {
        wchar_t* ws;
        Py_ssize_t _len = -1;

        if(o == NULL || PyObject_Not(o))
            return L"";

        if(PyUnicode_Check(o)) {
            if((ws = PyUnicode_AsWideCharString(o, &_len)) == NULL) {
                PyErr_Format(PyExc_ValueError, "Failed Parse unicode Object.");
                return L"";
            }
        } else {
            PyObject* str = PyObject_Str(o);
            if(str == NULL) {
                PyErr_Format(PyExc_ValueError, "Failed Parse unicode Object.");
                return L"";
            }
            ws = PyUnicode_AsWideCharString(str, &_len);
            Py_DECREF(str);
            if(ws == NULL) {
                PyErr_Format(PyExc_ValueError, "Failed Parse unicode Object.");
                return L"";
            }
        }
        std::wstring res(ws);
        PyMem_Free(ws);
        return res;
    }

    bool mapper(PyObject* pysrc, PyObject* pytar, int value = 1) {
        std::wstring src = pyto_wstring(pysrc);
        std::wstring tar = pyto_wstring(pytar);

        if(src.empty() && tar.empty()) {
            PyErr_Format(PyExc_ValueError, "Parse string Error.");
            return false;
        } else if(src.empty()) {
            if(nodes.find(tar) == nodes.end())
                nodes.emplace(tar);
        } else if(tar.empty()) {
            if(nodes.find(src) == nodes.end())
                nodes.emplace(src);
        } else {
            std::wstring key(LR"(,"source":")");
            key += src;
            key += LR"(","target":")";
            key += tar;
            key += LR"(","value":)";

            links[key] += value;
            if(nodes.find(src) == nodes.end())
                nodes.emplace(src);
            if(nodes.find(tar) == nodes.end())
                nodes.emplace(tar);
        }
        return true;
    }

    bool _table_parse(bool header, Py_ssize_t needcolsize, Py_ssize_t srcidx, Py_ssize_t taridx) {
        Py_ssize_t _nlen;
        const char* errmsg = "argument is 2d list or tuple object?";

        if(len == -1) {
            PyErr_Format(PyExc_IndexError, errmsg);
            return false;
        }

        for(Py_ssize_t n = (int)header; n < len; ++n) {
            PyObject* row = PySequence_GetItem(data, n);
            PyObject *pysrc, *pytar, *pyval;
            int val = 1;

            if(row == NULL)
                return false;

            if((_nlen = PyObject_Length(row)) != needcolsize) {
                PyErr_Format(PyExc_ValueError,
                             "UnExpected column size Error. expected %d columns.\n"
                             "but you input %d columns",
                             needcolsize, _nlen);
                Py_DECREF(row);
                return false;
            }

            pysrc = PySequence_GetItem(row, srcidx);
            pytar = PySequence_GetItem(row, taridx);
            if(_nlen > 2) {
                pyval = PySequence_GetItem(row, taridx + 1);
                val = PyLong_AsLong(pyval);
            }

            if((mapper(pysrc, pytar, val)) == false) {
                Py_DECREF(row);
                return false;
            }

            Py_DECREF(row);

            if(PyErr_Occurred())
                return false;
        }
        return true;
    }
};

#endif
