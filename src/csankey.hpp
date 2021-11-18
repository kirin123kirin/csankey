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
#include "csv-parser/parser.hpp"

static constexpr const wchar_t BEFORE_TEXT[] =
#include "bf.cc"
    ;
static constexpr const wchar_t AFTER_TEXT[] =
#include "af.cc"
    ;

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
    // SankeyData(nullptr_t) : data(), nodes({}), links({}), parsed(false) {}
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
            res += std::_UIntegral_to_string<CharT>(++i);
            res += TYPED_LITERAL(CharT*, R"(,"name":")") + node + TYPED_LITERAL(CharT*, "\"},\n");
        }
        res += TYPED_LITERAL(CharT*, "],\n\"links\":[\n");

        for(auto&& link : links) {
            res += TYPED_LITERAL(CharT*, R"({"ID":)");
            res += std::_UIntegral_to_string<CharT>(++j);
            res += link.first;
            res += std::_Integral_to_string<CharT>(link.second);
            res += TYPED_LITERAL(CharT*, "},\n");
        }
        return res + TYPED_LITERAL(CharT*, "]}\n");
    }

    void parse() {
        std::size_t _nlen;
        parsed = true;

        for(auto&& row : data) {
            std::size_t count;

            _nlen = (std::size_t)std::count_if(row.begin(), row.end(), [](auto x) { return !x.empty(); });
            if(_nlen < row.size()) {
                row_type tmp;
                std::copy_if(row.begin(), row.end(), std::back_inserter(tmp), [](auto x) { return !x.empty(); });
                row.swap(tmp);
            }

            if((_nlen = row.size()) == 0)
                return;

            if(_nlen == 1) {
                if(row[0].empty())
                    throw std::runtime_error("Parse string Error.");
                nodes.emplace(row[0]);
            } else {
                count = _nlen == 2 ? 1 : (_nlen >> 1) + 1;

                for(std::size_t i = 0; i < count; ++i) {
                    auto src = row[i];
                    if(src.empty())
                        src = row[i - 1];
                    mapper(src, row[i + 1]);
                }
            }
        }
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
    SankeyData(nullptr_t) : data(NULL), len(-1), nodes({}), links({}) {}
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
        Py_ssize_t _nlen;
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
            Py_ssize_t count;

            if(row == NULL || (_nlen = PyObject_Length(row)) == -1) {
                Py_XDECREF(row);
                return false;
            }

            if(_nlen == 1) {
                std::wstring src = pyto_wstring(PySequence_GetItem(row, 0));
                if(src.empty()) {
                    Py_DECREF(row);
                    PyErr_Format(PyExc_ValueError, "Parse string Error.");
                    return false;
                }
                nodes.emplace(src);
            } else {
                count = _nlen == 2 ? 1 : (_nlen >> 1) + 1;

                for(Py_ssize_t i = 0; i < count; ++i) {
                    if((mapper(PySequence_GetItem(row, i), PySequence_GetItem(row, i + 1))) == false) {
                        Py_DECREF(row);
                        return false;
                    }
                }
            }

            Py_DECREF(row);
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
        errno_t err;
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
            err = memcpy_s(p, wsize * datasize, BEFORE_TEXT, wsize * bf_len);
            if(err) {
                Py_CLEAR(res);
                return PyErr_Format(PyExc_MemoryError, "Error. before_text data memory writing");
            }
            p += bf_len;

            err = memcpy_s(p, wsize * (datasize - bf_len), jsondata.data(), wsize * json_len);
            if(err) {
                Py_CLEAR(res);
                return PyErr_Format(PyExc_MemoryError, "Error. after_text data memory writing");
            }
            p += json_len;

            err = memcpy_s(p, wsize * af_len, AFTER_TEXT, wsize * af_len);
            if(err) {
                Py_CLEAR(res);
                return PyErr_Format(PyExc_MemoryError, "Error. after_text data memory writing");
            }
        }

        return res;
    }

   private:
    std::wstring pyto_wstring(PyObject* o) {
        wchar_t* ws;
        Py_ssize_t _len = -1;
        if(o == NULL) {
            PyErr_Format(PyExc_ValueError, "Failed Parse unicode Object.");
            return L"";
        }
        if(PyObject_Not(o)) {
            PyErr_Format(PyExc_ValueError, "Failed Parse unicode Object.");
            return L"";
        }

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
