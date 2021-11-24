#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from timeit import timeit
from psutil import Process
from lxml import etree

from os.path import dirname, abspath, join as pjoin
shome = abspath(pjoin(dirname(__file__), ".."))
sys.path.insert(0, pjoin(shome, "build"))
sys.path.insert(0, pjoin(shome, "_skbuild", "cmake-install"))
sys.path.insert(0, pjoin(shome, "build", "cmake-install"))
from csankey import *

process = Process(os.getpid())
def memusage():
    return process.memory_info()[0] / 1024

def runtimeit(funcstr, number=10000):
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

from xml.etree import ElementTree as etree
from html.parser import HTMLParser





# parser=HTMLParser()
# print(parser.get_starttag_text())
# print(etree.fromstring(r, parser=HTMLParser().feed(r)))
# runtimeit('to_sankeyhtml([["abc", "bbb", "ccc"], ["ccc", "aaa", "bbb", "abc"]])')

# print(repr(r[-10:]))

# with open("hoge.html", "w", encoding="utf-8") as w:
#     w.write(r)


# print(to_sankeyjson([[[],[]]]))


