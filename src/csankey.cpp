/* csankey.cpp | MIT License | https://github.com/kirin123kirin/csankey/raw/main/LICENSE */
#include <Python.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

static constexpr wchar_t BEFORE_TEXT[] =
#include "bf.cc"
    ;
static constexpr wchar_t AFTER_TEXT[] =
#include "af.cc"
    ;

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

struct SankeyData {
    PyObject* data;
    Py_ssize_t len;
    std::unordered_set<std::wstring> nodes;
    std::unordered_map<std::wstring, int> links;

    SankeyData() : data(NULL), len(-1), nodes({}), links({}) {}
    SankeyData(nullptr_t) : data(NULL), len(-1), nodes({}), links({}) {}
    SankeyData(PyObject*& _py2darraydata)
        : data(_py2darraydata), len(PyObject_Length(_py2darraydata)), nodes({}), links({}) {}
    ~SankeyData() {}

    std::wstring to_json() {  //_to_datajson
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

    bool mapper(PyObject* pysrc, PyObject* pytar, int value = 1) {  //_append_data
        std::wstring src, tar;
        src = pyto_wstring(pysrc);
        if(src.empty()) {
            PyErr_Format(PyExc_ValueError, "Parse string Error.");
            return false;
        }
        tar = pyto_wstring(pytar);
        if(tar.empty()) {
            PyErr_Format(PyExc_ValueError, "Parse string Error.");
            return false;
        }

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

        return true;
    }

    bool parse() {
        Py_ssize_t _nlen;
        const char* errmsg = "argument is 2d list or tuple object?";

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

    bool _table_parse(Py_ssize_t startrowidx, Py_ssize_t needcolsize, Py_ssize_t srcidx, Py_ssize_t taridx) {
        Py_ssize_t _nlen;
        const char* errmsg = "argument is 2d list or tuple object?";

        if(len == -1) {
            PyErr_Format(PyExc_IndexError, errmsg);
            return false;
        }

        for(Py_ssize_t n = startrowidx; n < len; ++n) {
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

    bool parse(int header) {
        PyObject* row;
        const char* errmsg = "argument is 2d list or tuple object?";
        Py_ssize_t startrowidx = 0, nlen = -1;

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
};

PyObject* htmlrender(std::wstring jsondata) {
    PyObject* res = NULL;
    wchar_t *ret, *p;
    errno_t err;
    std::size_t json_len, bf_len, af_len, wsize, datasize;

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

extern "C" PyObject* to_sankeyhtml_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* o;
    int header = -1;

    const char* kwlist[3] = {"o", "header", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", (char**)kwlist, &o, &header))
        return NULL;

    if(!PyList_Check(o) && !PyTuple_Check(o))
        return PyErr_Format(PyExc_TypeError, "argument is list or tuple object only.");

    SankeyData arr(o);
    std::wstring jsondata;

    if(header == -1) {
        if(arr.parse() && !(jsondata = arr.to_json()).empty())
            return htmlrender(jsondata);
        return NULL;
    } else {
        /* expected 2d table data */
        if(arr.parse((bool)header) && !(jsondata = arr.to_json()).empty())
            return htmlrender(jsondata);

        if(header == 0 || header == 1)
            return NULL;

        /* if previous failed when dirty table to json */
        if(arr.parse() && !(jsondata = arr.to_json()).empty())
            return htmlrender(jsondata);
    }

    return PyErr_Format(PyExc_ValueError, "Unknown Error Occured.");
}

extern "C" PyObject* to_sankeyjson_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* o;
    int header = -1;

    const char* kwlist[3] = {"o", "header", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", (char**)kwlist, &o, &header))
        return NULL;

    if(!PyList_Check(o) && !PyTuple_Check(o))
        return PyErr_Format(PyExc_TypeError, "argument is list or tuple object only.");

    SankeyData arr(o);
    std::wstring jsondata;

    if(header == -1) {
        if(arr.parse() && !(jsondata = arr.to_json()).empty())
            return PyUnicode_FromWideChar(jsondata.data(), (Py_ssize_t)jsondata.size());
        return NULL;
    } else {
        /* expected 2d table data */
        if(arr.parse((bool)header) && !(jsondata = arr.to_json()).empty())
            return PyUnicode_FromWideChar(jsondata.data(), (Py_ssize_t)jsondata.size());

        if(header == 0 || header == 1)
            return NULL;

        /* if previous failed when dirty table to json */
        if(arr.parse() && !(jsondata = arr.to_json()).empty())
            return PyUnicode_FromWideChar(jsondata.data(), (Py_ssize_t)jsondata.size());
    }

    return PyErr_Format(PyExc_ValueError, "Unknown Error Occured.");
}

#define MODULE_NAME csankey
#define MODULE_NAME_S "csankey"

/* {{{ */
// this module description
#define MODULE_DOCS                                            \
    "Make html data of Sankey Diagram.\n"                      \
    "Sankey diagram made using d3.js and the sankey plugin.\n" \
    ""

#define to_sankeyhtml_py_DESC "\n"
#define to_sankeyjson_py_DESC "\n"

/* }}} */
#define PY_ADD_METHOD(py_func, c_func, desc) \
    { py_func, (PyCFunction)c_func, METH_VARARGS, desc }
#define PY_ADD_METHOD_KWARGS(py_func, c_func, desc) \
    { py_func, (PyCFunction)c_func, METH_VARARGS | METH_KEYWORDS, desc }

/* Please extern method define for python */
/* PyMethodDef Parameter Help
 * https://docs.python.org/ja/3/c-api/structures.html#c.PyMethodDef
 */
static PyMethodDef py_methods[] = {PY_ADD_METHOD_KWARGS("to_sankeyhtml", to_sankeyhtml_py, to_sankeyhtml_py_DESC),
                                   PY_ADD_METHOD_KWARGS("to_sankeyjson", to_sankeyjson_py, to_sankeyjson_py_DESC),
                                   {NULL, NULL, 0, NULL}};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef py_defmod = {PyModuleDef_HEAD_INIT, MODULE_NAME_S, MODULE_DOCS, 0, py_methods};
#define PARSE_NAME(mn) PyInit_##mn
#define PARSE_FUNC(mn) \
    PyMODINIT_FUNC PARSE_NAME(mn)() { return PyModule_Create(&py_defmod); }

#else
#define PARSE_NAME(mn) \
    init##mn(void) { (void)Py_InitModule3(MODULE_NAME_S, py_methods, MODULE_DOCS); }
#define PARSE_FUNC(mn) PyMODINIT_FUNC PARSE_NAME(mn)
#endif

PARSE_FUNC(MODULE_NAME);
