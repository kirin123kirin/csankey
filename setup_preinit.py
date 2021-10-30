import re
from os.path import dirname, normpath, join as pathjoin

SRCDIR = pathjoin(dirname(__file__), "src")
TARGET = normpath(pathjoin(SRCDIR, "index.html"))

def make_compiler_input(srcdir=SRCDIR, target=TARGET, minify=True):
    if minify:
        re_minify = re.compile("\n\s*").sub
    else:
        re_minify = lambda a, b: b
    dat = None
    with open(target, "r", encoding="utf-8") as f:
        dat = f.read()

    if dat:
        scripts = "<script>"

        start = 0
        end = 0

        for i, re_scp in enumerate(re.finditer('<script\s+src="([^\"]+)">\s*</script>', dat)):
            if i == 0:
                start = re_scp.start()
            end = re_scp.end()

            scpfile = normpath(pathjoin(SRCDIR, re_scp.group(1)))
            with open(scpfile, "r", encoding="utf-8") as f:
                scripts += f.read()
        scripts += "</script>"

        bf = dat[:start] + scripts
        af = dat[end:]

        mbf = re.search("<!-- My Sankey Data Section -->\s*<script>\s*var\s+data\s*=\s*", af, re.MULTILINE)
        if not mbf:
            raise ValueError("unexpected `before` template data.")

        bfend = mbf.end()
        bf += af[:bfend]
        af = af[bfend:]

        maf = re.search("</script>", af)
        if not maf:
            raise ValueError("unexpected `after` template data.")

        with open(normpath(pathjoin(srcdir, "bf.txt")), "w", encoding="utf-8") as w:
            w.write("{")
            for d in map(repr, re_minify("", bf)):
                if d == '"\'"':
                    d = "'\\''"
                w.write("L" + d + ",")
            w.write("NULL}")

        with open(normpath(pathjoin(srcdir, "af.txt")), "w", encoding="utf-8") as w:
            w.write("{")
            for d in map(repr, re_minify("", af[maf.start():])):
                if d == '"\'"':
                    d = "'\\''"
                w.write("L" + d + ",")
            w.write("NULL}")

    else:
        raise ValueError("Is Empty" + target + "?")

