# Python C Extention Easy Core functions
[![Upload pypi.org](https://github.com/kirin123kirin/csankey/actions/workflows/pypi.yml/badge.svg?branch=v0.5.4)](https://github.com/kirin123kirin/csankey/actions/workflows/pypi.yml)

# Overview
There is an exciting application that uses d3.js sankey-circular.js.
However, the way to create json data is very complicated.

# Goal
* The goal is to automatically generate a sankey diagram from a csv or spreadsheet.
* and also to create a sankey diagram by copying a spreadsheet table to the clipboard and running this program.

# Implementation
It uses a library created in C++ and imported as a library from python.
The reason for this is that file and string manipulation is much easier in python.

# Install
```
$ pip install csankey
```

# UnInstall
```
$ pip uninstall csankey
```

# Requirement
* python3.3 later.

# run command Environment
* Windows
* Linux
* Mac OSX

# Supported Browsers of OutputHTML file
| Checked Browser        | Result        |
| ---------------------- | ------------- |
| Chrome(95.0.4638.69)   | OK            |
| Edge(95.0.1020.40)     | OK            |
| Firefox                | ?Unknown      |
| IE                     | ?Unknown      |
| Opera                  | ?Unknown      |
| Safari                 | ?Unknown      |

# Usage
TODO

# Example
TODO

# Perfomance
TODO

# Libraries used
* https://d3js.org/d3.v4.min.js
* https://bl.ocks.org/tomshanley/raw/6f3fcf68c0dbc401548733dd0c64e3c3/d3-sankey-circular.js
* https://www.npmjs.com/package/@riversun/sortable-table sortable-table.js copyright riversun
* https://github.com/kirin123kirin/csv-parser/parser.hpp copyright kirin123kirin

# References used
* https://github.com/tomshanley/d3-sankey-circular
* https://github.com/d3/d3-sankey copyright Mike Bostock
* https://bl.ocks.org/tomshanley/6eb025290888935f10b142e4bc576d8d#d3-path-arrows.js
* https://bl.ocks.org/tomshanley/874923fe54b173735b456479423ac7d6#d3-sankey-circular.js (function appendArrows)


