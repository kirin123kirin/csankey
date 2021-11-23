import re
from os.path import dirname, normpath, join as pathjoin
from urllib import request

SRCDIR = normpath(pathjoin(dirname(__file__), "../csankey"))
TARGET = normpath(pathjoin(SRCDIR, "index.html"))


def make_compiler_input(srcdir=SRCDIR, target=TARGET, minify=False):
    if minify:
        re_minify = re.compile(r"^\s+", re.MULTILINE).sub
    else:
        def re_minify(_, b):
            return b
    dat = None
    UA = [('User-Agent', 'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1941.0 Safari/537.36')]
    with open(target, "r", encoding="utf-8") as f:
        dat = f.read()

    if dat:
        for re_css in re.finditer(r'<link\s+href=[\"\']([^\"\']+)[\"\']\s+rel=[\"\']stylesheet[\"\']\s*>', dat):
            cssname = re_css.group(1)
            if(cssname.startswith("http")):
                opener = request.build_opener()
                opener.addheaders = UA
                request.install_opener(opener)
                with request.urlopen(cssname) as res:
                    dat = dat.replace(re_css.group(0), '<style type="text/css" media="screen">\n' + re_minify("", res.read().decode()) + "\n</style>")
            else:
                cssfile = normpath(pathjoin(SRCDIR, cssname))
                with open(cssfile, "r", encoding="utf-8") as f:
                    dat = dat.replace(re_css.group(0), '<style type="text/css" media="screen">\n' + re_minify("", f.read()) + "\n</style>")

        scripts = '<script type="text/javascript">'
        start = 0
        end = 0

        for i, re_scp in enumerate(re.finditer(r'<script\s+src="([^\"]+)"\s*>.*</script>', dat)):
            if i == 0:
                start = re_scp.start()
            end = re_scp.end()
            scpname = re_scp.group(1)
            if (scpname.startswith("http")):
                opener = request.build_opener()
                opener.addheaders = UA
                request.install_opener(opener)
                with request.urlopen(scpname) as res:
                    scripts += res.read().decode()
            else:
                scpfile = normpath(pathjoin(SRCDIR, scpname))
                with open(scpfile, "r", encoding="utf-8") as f:
                    scripts += f.read()

        scripts += "</script>"

        bf = dat[:start] + scripts
        af = dat[end:]

        mbf = re.search(r"<!-- My Sankey Data Section -->\s*<script>\s*var\s+data\s*=\s*", af, re.MULTILINE)
        if not mbf:
            raise ValueError("unexpected `before` template data.")

        bfend = mbf.end()
        bf += af[:bfend]
        af = af[bfend:]

        maf = re.search("</script>", af)
        if not maf:
            raise ValueError("unexpected `after` template data.")
        BOM = b"\xEF\xBB\xBF".decode()
        with open(normpath(pathjoin(srcdir, "bf.cc")), "w", encoding="utf-8") as w:
            w.write(BOM)
            w.write("{")
            for d in map(repr, re_minify("", bf)):
                if d == '"\'"':
                    d = "'\\''"
                w.write("L" + d + ",")
            w.write("NULL}")

        with open(normpath(pathjoin(srcdir, "af.cc")), "w", encoding="utf-8") as w:
            w.write(BOM)
            w.write("{")
            for d in map(repr, re_minify("", af[maf.start():])):
                if d == '"\'"':
                    d = "'\\''"
                w.write("L" + d + ",")
            w.write("NULL}")

    else:
        raise ValueError("Is Empty" + target + "?")
