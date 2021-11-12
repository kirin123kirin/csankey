#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from timeit import timeit
from datetime import datetime, timezone, timedelta
import os
import sys
from psutil import virtual_memory, Process
from lxml import etree

process = Process(os.getpid())

try:
    from csankey.csankey import *
except:
    try:
        from csankey.build.csankey import *
    except:
        try:
            from build.csankey import *
        except:
            from csankey import *


def memusage():
    return process.memory_info()[0] / 1024

def runtimeit(funcstr, number=100000):
    i = 0

    for fc in funcstr.strip().splitlines():
        fc = fc.strip()
        if i == 0:
            timeit(fc, globals=globals(), number=number)
        bm = memusage()
        p = timeit(fc, globals=globals(), number=number)
        
        am = (memusage() - bm)
        assert am < 1000, "{} function {}KB Memory Leak Error".format(fc, am)
        print("{}: {} ns (mem after {}KB)".format(fc, int(1000000000 * p / number), am))
        i += 1


ans1 = """{
"nodes":[
{"ID":1,"name":"abc"},
{"ID":2,"name":"bbb"},
{"ID":3,"name":"ccc"},
{"ID":4,"name":"aaa"},
],
"links":[
{"ID":1,"source":"bbb","target":"abc","value":1},
{"ID":2,"source":"ccc","target":"aaa","value":1},
{"ID":3,"source":"abc","target":"bbb","value":1},
{"ID":4,"source":"bbb","target":"ccc","value":1},
{"ID":5,"source":"aaa","target":"bbb","value":1},
]}
"""

ans2 = """{
"nodes":[
{"ID":1,"name":"a"},
{"ID":2,"name":"1"},
{"ID":3,"name":"3"},
{"ID":4,"name":"b"},
],
"links":[
{"ID":1,"source":"1","target":"3","value":1},
{"ID":2,"source":"a","target":"b","value":1},
]}
"""

ans3 = """{
"nodes":[
{"ID":1,"name":"a"},
{"ID":2,"name":"1"},
],
"links":[
]}
"""

ans4 = """{
"nodes":[
{"ID":1,"name":"a"},
{"ID":2,"name":"1"},
{"ID":3,"name":"3"},
{"ID":4,"name":"b"},
],
"links":[
{"ID":1,"source":"1","target":"3","value":20},
{"ID":2,"source":"a","target":"b","value":40},
]}
"""

def test_to_sankeyjson_regular_case():
    assert(to_sankeyjson([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]]) == ans1)
    assert(to_sankeyjson([["1", "3"],["a", "b"]]) == ans2)
    assert(to_sankeyjson([["1"],["a"]]) == ans3)

def test_to_sankeyjson_regular_case_Ncolumns():
    assert(to_sankeyjson([["1", "3"],["a", "b"]], False) == ans2)
    assert(to_sankeyjson([["1", "3", 20],["a", "b", 40]], False) == ans4)


def test_to_sankeyhtml_regular_case():
    assert(len(etree.fromstring(to_sankeyhtml([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]]), etree.HTMLParser())))
    assert(len(etree.fromstring(to_sankeyhtml([["1", "3"],["a", "b"]]), etree.HTMLParser())))
    assert(len(etree.fromstring(to_sankeyhtml([["1"],["a"]]), etree.HTMLParser())))

def test_to_sankeyhtml_regular_case_Ncolumns():
    assert(len(etree.fromstring(to_sankeyhtml([["1", "3"],["a", "b"]], False), etree.HTMLParser())))
    assert(len(etree.fromstring(to_sankeyhtml([["1", "3", 20],["a", "b", 40]], False), etree.HTMLParser())))

def _irregular_case(errtype, func, args):
    try:
        func(*args)
    except errtype:
        pass
    else:
        raise AssertionError

def test_to_sankeyjson_irregular_case():
    _irregular_case(ValueError, to_sankeyjson, ([["1"],["a"]], False))
    _irregular_case(ValueError, to_sankeyjson, ([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]], False))

def test_to_sankeyjson_irregular_case():
    _irregular_case(TypeError, to_sankeyjson, [])
    _irregular_case(TypeError, to_sankeyjson, [None])
    _irregular_case(TypeError, to_sankeyjson, [1])
    _irregular_case(TypeError, to_sankeyjson, ["a"])
    _irregular_case(TypeError, to_sankeyjson, [""])
    _irregular_case(ValueError, to_sankeyjson, [[]])
    _irregular_case(ValueError, to_sankeyjson, [()])
    _irregular_case(ValueError, to_sankeyjson, [[[],[]]])
    _irregular_case(ValueError, to_sankeyjson, [[[]]])

def test_to_sankeyjson_irregular_case2():
    _irregular_case(TypeError, to_sankeyjson, [None, False])
    _irregular_case(TypeError, to_sankeyjson, [1, False])
    _irregular_case(TypeError, to_sankeyjson, ["a", False])
    _irregular_case(TypeError, to_sankeyjson, ["", False])
    _irregular_case(ValueError, to_sankeyjson, [[], False])
    _irregular_case(ValueError, to_sankeyjson, [(), False])
    _irregular_case(ValueError, to_sankeyjson, [[[],[]], False])
    _irregular_case(ValueError, to_sankeyjson, [[[]], False])

def test_to_sankeyhtml_irregular_case():
    _irregular_case(ValueError, to_sankeyhtml, ([["1"],["a"]], False))
    _irregular_case(ValueError, to_sankeyhtml, ([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]], False))

def test_to_sankeyhtml_irregular_case():
    _irregular_case(TypeError, to_sankeyhtml, [])
    _irregular_case(TypeError, to_sankeyhtml, [None])
    _irregular_case(TypeError, to_sankeyhtml, [1])
    _irregular_case(TypeError, to_sankeyhtml, ["a"])
    _irregular_case(TypeError, to_sankeyhtml, [""])
    _irregular_case(ValueError, to_sankeyhtml, [[]])
    _irregular_case(ValueError, to_sankeyhtml, [()])
    _irregular_case(ValueError, to_sankeyhtml, [[[],[]]])
    _irregular_case(ValueError, to_sankeyhtml, [[[]]])

def test_to_sankeyhtml_irregular_case2():
    _irregular_case(TypeError, to_sankeyhtml, [None, False])
    _irregular_case(TypeError, to_sankeyhtml, [1, False])
    _irregular_case(TypeError, to_sankeyhtml, ["a", False])
    _irregular_case(TypeError, to_sankeyhtml, ["", False])
    _irregular_case(ValueError, to_sankeyhtml, [[], False])
    _irregular_case(ValueError, to_sankeyhtml, [(), False])
    _irregular_case(ValueError, to_sankeyhtml, [[[],[]], False])
    _irregular_case(ValueError, to_sankeyhtml, [[[]], False])


def test_to_sankeyjson_regular_case_perf():
    runtimeit('to_sankeyjson([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]])')
    runtimeit('to_sankeyjson([["1", "3"],["a", "b"]])')
    runtimeit('to_sankeyjson([["1"],["a"]])')

def test_to_sankeyjson_regular_case_Ncolumns_perf():
    runtimeit('to_sankeyjson([["1", "3"],["a", "b"]], False)')
    runtimeit('to_sankeyjson([["1", "3", 20],["a", "b", 40]], False)')

def test_to_sankeyhtml_regular_case_perf():
    runtimeit('to_sankeyhtml([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]])')
    runtimeit('to_sankeyhtml([["1", "3"],["a", "b"]])')
    runtimeit('to_sankeyhtml([["1"],["a"]])')

def test_to_sankeyhtml_regular_case_Ncolumns_perf():
    runtimeit('to_sankeyhtml([["1", "3"],["a", "b"]], False)')
    runtimeit('to_sankeyhtml([["1", "3", 20],["a", "b", 40]], False)')

def test_to_sankeyjsonruntimeit():
    runtimeit('_irregular_case(ValueError, to_sankeyjson, ([["1"],["a"]], False))')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, ([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]], False))')

def test_to_sankeyjsonruntimeit():
    runtimeit('_irregular_case(TypeError, to_sankeyjson, [])')
    runtimeit('_irregular_case(TypeError, to_sankeyjson, [None])')
    runtimeit('_irregular_case(TypeError, to_sankeyjson, [1])')
    runtimeit('_irregular_case(TypeError, to_sankeyjson, ["a"])')
    runtimeit('_irregular_case(TypeError, to_sankeyjson, [""])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [[]])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [()])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [[[],[]]])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [[[]]])')

def test_to_sankeyjsonruntimeit():
    runtimeit('_irregular_case(TypeError, to_sankeyjson, [None, False])')
    runtimeit('_irregular_case(TypeError, to_sankeyjson, [1, False])')
    runtimeit('_irregular_case(TypeError, to_sankeyjson, ["a", False])')
    runtimeit('_irregular_case(TypeError, to_sankeyjson, ["", False])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [[], False])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [(), False])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [[[],[]], False])')
    runtimeit('_irregular_case(ValueError, to_sankeyjson, [[[]], False])')

def test_to_sankeyhtmlruntimeit():
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, ([["1"],["a"]], False))')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, ([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]], False))')

def test_to_sankeyhtmlruntimeit():
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, [])')
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, [None])')
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, [1])')
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, ["a"])')
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, [""])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [[]])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [()])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [[[],[]]])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [[[]]])')

def test_to_sankeyhtmlruntimeit():
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, [None, False])')
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, [1, False])')
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, ["a", False])')
    runtimeit('_irregular_case(TypeError, to_sankeyhtml, ["", False])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [[], False])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [(), False])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [[[],[]], False])')
    runtimeit('_irregular_case(ValueError, to_sankeyhtml, [[[]], False])')


if __name__ == '__main__':
    import os
    import traceback

    curdir = os.getcwd()
    try:
        os.chdir(os.path.dirname(os.path.abspath(__file__)))
        for fn, func in dict(locals()).items():
            if fn.startswith("test_"):
                print("Runner: %s" % fn)
                func()
    except Exception as e:
        traceback.print_exc()
        raise (e)
    finally:
        os.chdir(curdir)

