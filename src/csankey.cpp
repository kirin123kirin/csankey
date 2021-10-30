#include <Python.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

static constexpr wchar_t BEFORE_TEXT[] =
#include "bf.txt"
    ;
static constexpr wchar_t AFTER_TEXT[] =
#include "af.txt"
    ;

wchar_t* parse_py2string(PyObject* o, Py_ssize_t* len) {
    *len = (std::size_t)PyObject_Length(o);
    wchar_t* ws = NULL;

    if(*len == std::size_t(-1)) {
        PyErr_Format(PyExc_ValueError, "Failed length Object.");
        return NULL;
    }
    if(*len == 0)
        return L"";

    Py_INCREF(o);

    if(PyUnicode_Check(o)) {
        ws = PyUnicode_AsWideCharString(o, len);
    } else {
        PyObject* str = PyObject_Str(o);
        if(str == NULL) {
            PyErr_Format(PyExc_ValueError, "Failed Parse unicode Object.");
            return NULL;
        }
        ws = PyUnicode_AsWideCharString(str, len);
        Py_DECREF(str);
    }

    return ws;
}

std::wstring dirtytable_tojson(PyObject* py2darraydata) {
    std::unordered_set<std::wstring> nodes = {};
    std::unordered_map<std::wstring, int> links = {};
    std::size_t datasize = 31;  // header + footer

    for(Py_ssize_t n = 0, len = PyObject_Length(py2darraydata); n < len; n++) {
        PyObject* row = PySequence_GetItem(py2darraydata, n);
        Py_ssize_t nlen = PyObject_Length(row);

        if(nlen == -1) {
            PyErr_Format(PyExc_IndexError, "argument is 2d list or tuple object only.");
            return L"";
        }
        Py_ssize_t count = (nlen >> 1) + 1;

        for(Py_ssize_t i = 0; i < count; ++i) {
            PyObject *pysrc, *pytar;
            wchar_t *src, *tar;
            std::wstring key;

            if((pysrc = PySequence_GetItem(row, i)) == NULL) {
                Py_CLEAR(row);
                PyErr_Format(PyExc_IndexError, "argument is 2d list or tuple object only.");
                return L"";
            }

            if((pytar = PySequence_GetItem(row, i + 1)) == NULL) {
                Py_CLEAR(row);
                PyErr_Format(PyExc_IndexError, "argument is 2d list or tuple object only.");
                return L"";
            }

            Py_ssize_t src_len = 0, tar_len = 0;
            src = parse_py2string(pysrc, &src_len);
            tar = parse_py2string(pytar, &tar_len);

            if(!(src && tar)) {
                PyErr_Format(PyExc_ValueError, "Parse string Error.");
                return L"";
            }

            key = LR"(  { "source": ")" + std::wstring(src) + LR"(", "target": ")" + std::wstring(tar) +
                  LR"(", "value": )";

            links[key] += 1;
            if(links.at(key) == 1)
                datasize += key.size() + 4;

            if(nodes.find(src) == nodes.end()) {
                nodes.emplace(src);
                datasize += src_len + 18;
            }
            PyMem_FREE(src);

            if(nodes.find(tar) == nodes.end()) {
                nodes.emplace(tar);
                datasize += tar_len + 18;
            }
            PyMem_FREE(tar);
        }
        Py_DECREF(row);
    }

    Py_DECREF(py2darraydata);

    std::wstring res = L"{\n\"nodes\":[\n";

    for(auto&& node : nodes)
        res += LR"({"name":")" + node + L"\"},\n";
    res += L"],\n\"links\":[\n";

    for(auto&& link : links)
        res += link.first + std::to_wstring(link.second) + L"},\n";

    return res + L"]}\n";
}

PyObject* render_flatten(PyObject* py2darraydata) {
    PyObject* res = NULL;
    wchar_t *ret, *p;
    errno_t err;
    std::size_t json_len, bf_len, af_len, wsize, datasize;

    std::wstring jsontbl = dirtytable_tojson(py2darraydata);

    if((json_len = jsontbl.size()) == 0)
        return NULL;

    wsize = sizeof(wchar_t);
    bf_len = (sizeof(BEFORE_TEXT) / wsize) - 1;
    af_len = (sizeof(AFTER_TEXT) / wsize) - 1;
    datasize = json_len + bf_len + af_len;

    {
        /* Make Faster PyUnicode Object Make. */
        if((res = PyUnicode_New(datasize, wsize == 2 ? 65535 : 1114111)) == NULL)
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

        err = memcpy_s(p, wsize * (datasize - bf_len), jsontbl.data(), wsize * json_len);
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


extern "C" PyObject* sankey_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* o;

    const char* kwlist[2] = {"o", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O", (char**)kwlist, &o))
        return NULL;

    if(PyList_Check(o) || PyTuple_Check(o))
        return render_flatten(o);
    else
        return PyErr_Format(PyExc_ValueError, "argument is list or tuple object only.");
}

#define MODULE_NAME csankey
#define MODULE_NAME_S "csankey"

/* {{{ */
// this module description
#define MODULE_DOCS                       \
    "Make html data of Sankey Diagram.\n" \
    "Sankey diagram made using d3.js and the sankey plugin."

#define sankey_py_DESC "\n"

/* }}} */
#define PY_ADD_METHOD(py_func, c_func, desc) \
    { py_func, (PyCFunction)c_func, METH_VARARGS, desc }
#define PY_ADD_METHOD_KWARGS(py_func, c_func, desc) \
    { py_func, (PyCFunction)c_func, METH_VARARGS | METH_KEYWORDS, desc }

/* Please extern method define for python */
/* PyMethodDef Parameter Help
 * https://docs.python.org/ja/3/c-api/structures.html#c.PyMethodDef
 */
static PyMethodDef py_methods[] = {PY_ADD_METHOD_KWARGS("sankey", sankey_py, sankey_py_DESC), {NULL, NULL, 0, NULL}};

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
