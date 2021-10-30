#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from timeit import timeit
from datetime import datetime, timezone, timedelta
import os
import sys
from psutil import virtual_memory, Process
process = Process(os.getpid())

IMPORTS = ' import to_sankeyhtml'
try:
    from csankey.csankey import *
    smip = 'from csankey.csankey'
except:
    try:
        from csankey.build.csankey import *
        smip = 'from csankey.build.csankey'
    except:
        try:
            from build.csankey import *
            smip = 'from build.csankey'
        except:
            from csankey import *
            smip = 'from csankey'

smip += IMPORTS



def memusage():
    return process.memory_info()[0] / 1024

def runtimeit(funcstr, setup=smip, number=10000, normalize=10000):
    st = setup.strip()
    i = 0
    for fc in funcstr.strip().splitlines():
        fc = fc.strip()
        if i == 0:
            timeit(fc, st, number=number)
        bm = memusage()
        p = timeit(fc, st, number=number)
        am = (memusage() - bm)
        try:
            print("{}: {} ns (mem after {}KB)".format(fc, int(p * normalize), am))
        except UnicodeEncodeError:
            print("???: {} ns (mem after {}KB)".format(int(p * normalize), am))
        i += 1

def test_sankey():
    assert(to_sankeyhtml([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]]))
    runtimeit('to_sankeyhtml([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]])')


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

