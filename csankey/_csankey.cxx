/* csankey.cpp | MIT License | https://github.com/kirin123kirin/csankey/raw/main/LICENSE */
#include "csankey.hpp"

extern "C" PyObject* to_sankeyhtml_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* o, *res;
    int header = -1;

    const char* kwlist[3] = {"o", "header", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", (char**)kwlist, &o, &header))
        return NULL;

    if(!PyList_Check(o) && !PyTuple_Check(o))
        return PyErr_Format(PyExc_TypeError, "argument is list or tuple object only.");

    SankeyData<wchar_t, PyObject*> snk(o);
    bool r;
    if(header == -1)
        r = snk.parse();
    else
        r = snk.parse((bool)header);
    
    if (r && (res = snk.to_html()) != NULL)
        return res;

    return PyErr_Format(PyExc_ValueError, "Unknown Error Occured.");
}

extern "C" PyObject* to_sankeyjson_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* o;
    std::wstring jsondata;
    int header = -1;

    const char* kwlist[3] = {"o", "header", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", (char**)kwlist, &o, &header))
        return NULL;

    if(!PyList_Check(o) && !PyTuple_Check(o))
        return PyErr_Format(PyExc_TypeError, "argument is list or tuple object only.");

    SankeyData<wchar_t, PyObject*> snk(o);
    bool r;
    if(header == -1)
        r = snk.parse();
    else
        r = snk.parse((bool)header);
    
    if (r && !(jsondata = snk.to_json()).empty())
        return PyUnicode_FromWideChar(jsondata.data(), (Py_ssize_t)jsondata.size());

    return PyErr_Format(PyExc_ValueError, "Unknown Error Occured.");
}

#define MODULE_NAME _csankey
#define MODULE_NAME_S "_csankey"

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
