var width = 0;
var height = 0;
var margin = { top: height * 0.05, right: width * 0.1, bottom: height * 0.13, left: width * 0.1 };
var outerwidth = width + margin.left + margin.right;
var outerheight = height + margin.top + margin.bottom;

var nodewidth = null;
var nodepaddingratio = null;
var strokeopacity = null;
var linkopacity = null;
var linkforwardcolor = null;
var linkreversecolor = null;
var nodealign = null;
var nodeiterations = null;
var nodeopacity = null;
var hideopacity = null;
var textalign = null;
var nodetextfontsize = null;
var nodetextfontweight = null;
var fontfamily = null;
var arrowlen = null;
var arrowgaplen = null;
var arrowheadsize = null;
var arrowcolor = null;
var badgetextsize = null;
var badgebgcolor = null;
var badgetextcolor = null;
var highlighttype = null;

var nodePadding = 20;

var circularLinkGap = 2;

var sankeyData = null;
var sankeyNodes = null;
var sankeyLinks = null;

var duration = 5;
var maxOffset = 10;
var percentageOffset = 1.0;

let animateDash = null;
var sankey = null;

var svg = null;
var g = null;
var nodecolor = null;
var nodetextfontcolor = null;
var linkG = null;
var nodeG = null;
var node = null;
var link = null;
var badge = null;
var arrowsG = null;

function init(event) {
    if (sankey)
        d3.selectAll("svg").remove()

    sankey = d3.sankeyCircular()
        .nodeWidth(nodewidth)
        .nodePadding(nodePadding) //note that this will be overridden by nodePaddingRatio
        .nodePaddingRatio(nodepaddingratio)
        .size([width, height])
        .nodeId(function (d) {
            return d.name;
        })
        .nodeAlign(nodealign)
        .iterations(nodeiterations)
        .circularLinkGap(circularLinkGap)
    // .sortNodes("col")

    //run the Sankey + circular over the data
    sankeyData = sankey(data);
    sankeyNodes = sankeyData.nodes;
    sankeyLinks = sankeyData.links;

    let depthExtent = d3.extent(sankeyNodes, function (d) { return d.depth; });

    svg = d3.select("#chart").append("svg")
        .attr("width", outerwidth)
        .attr("height", outerheight)
        .attr("viewBox", "0 0 " + String(outerwidth) + " " + String(outerheight));

    g = svg.append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")")
};

function makenode(event = null) {
    nodeG = g.append("g")
        .attr("class", "nodes")
        .style("opacity", nodeopacity)
        .selectAll("g");

    node = nodeG.data(sankeyNodes)
        .enter()
        .append("g")
        .each(function (d) {
            d.name = String(d.name)
            if (d.name.indexOf("\n") != -1)
                d.name = d.name.replaceAll("\n", "<br/>");
            d.innertext = d.name;
            if (d.innertext.indexOf("<") != -1) {
                /* thanks for https://stackoverflow.com/questions/494143/creating-a-new-dom-element-from-an-html-string-using-built-in-dom-methods-or-pro */
                let div = document.createElement('div');
                div.innerHTML = d.innertext.replaceAll(/<br\s*\/?\s*>/gi, "\n").trim();
                if (div.firstChild)
                    d.innertext = div.firstChild.innerText || div.firstChild.textContent;
            }

            let array = d.innertext.split("\n");
            d.innertext_maxline = array.length;
            if (d.innertext_maxline > 1)
                d.innertext_maxlength = d3.max(array, t => t.length);
            else
                d.innertext_maxlength = d.innertext.length;

        })
        .call(d3.drag()
            .subject(d => d)
            .on('start', function () {
                this.parentNode.appendChild(this);
            })
            .on('drag', dragmove));

    let nodetmp = node.append("rect")
        .attr("x", function (d) { return d.x0; })
        .attr("y", function (d) { return d.y0; })
        .attr("height", function (d) { return d.y1 - d.y0; })
        .attr("width", function (d) { return d.x1 - d.x0; })
        .style("shape-rendering", "crispEdges")
        .style("fill", d => nodecolor(d.x0))
        .style("opacity", nodeopacity)

    if (highlighttype != "nohighlight") {
        nodetmp.on("mouseover", function (d) {

            let thisName = d.name;

            node.selectAll("rect,text,.node-label")
                .style("opacity", function (d) {

                    let opacity = hideopacity
                    if (d.name == thisName) {
                        opacity = nodeopacity;
                    }
                    d.sourceLinks.forEach(function (link) {
                        if (link.target.name == thisName) {
                            opacity = nodeopacity;
                        };
                    })
                    d.targetLinks.forEach(function (link) {
                        if (link.source.name == thisName) {
                            opacity = nodeopacity;
                        };
                    })
                    return opacity;
                })

            d3.selectAll(".sankey-link,.g-arrow")
                .style("opacity", function (l) {
                    return l.source.name == thisName || l.target.name == thisName ? linkopacity : hideopacity;
                })

        })
            .on("mouseout", function (d) {
                d3.selectAll("rect,.node-label").style("opacity", nodeopacity);
                d3.selectAll(".sankey-link,.g-arrow").style("opacity", linkopacity);
                d3.selectAll("text").style("opacity", 1);
            })
    }

    node.append("foreignObject")
        .attr("x", function (d) { return ((d.x0 + d.x1 - (nodetextfontsize / 2 + d.innertext_maxlength * 9)) / 2); })
        .attr("y", function (d) { return d.y0 - (d.innertext_maxline * nodetextfontsize) - 12; })
        .attr("width", d => (nodetextfontsize * 1.33) + (d.innertext_maxlength * 9))
        .attr("height", d => d.innertext_maxline * nodetextfontsize)
        .append("xhtml:div")
        .attr("class", "node-label")
        .attr("style", function (d) {
            return `
            position: relative; \
            text-align : ${textalign};
            color: ${nodetextfontcolor(d.x0)}; \
            text-shadow: #fff 2px 0, #fff -2px 0, #fff 0 -2px, #fff 0 2px, #fff 2px 2px, #fff -2px 2px, #fff 2px -2px, #fff -2px -2px, #fff 1px 2px, #fff -1px 2px, #fff 1px -2px, #fff -1px -2px, #fff 2px 1px, #fff -2px 1px, #fff 2px -1px, #fff -2px -1px, rgba(0, 0, 0, .5) 3px 3px 3px; \
            font-size: ${nodetextfontsize}px; \
            font-weight : ${nodetextfontweight}; \
            font-family: ${fontfamily}; \
            `})
        .html(function (d) { return d.name; });

    node.append("title")
        .text(function (d) { return d.innertext + "\n" + (d.value); });

};
function makelink(event = null) {
    linkG = g.append("g")
        .attr("class", "links")
        .attr("fill", "none")
        .attr("stroke-opacity", strokeopacity)
        .selectAll("path");

    link = linkG.data(sankeyLinks)
        .enter()
        .append("g")
        .each(function (d) {
            d.value = d.innertext = String(d.value);
            if (d.innertext.indexOf("<") != -1) {
                /* thanks for https://stackoverflow.com/questions/494143/creating-a-new-dom-element-from-an-html-string-using-built-in-dom-methods-or-pro */
                let div = document.createElement('div');
                div.innerHTML = d.innertext.replaceAll(/<br\s*\/?\s*>/gi, "\n").trim();
                if (div.firstChild)
                    d.innertext = div.firstChild.innerText;
            }

            let array = d.innertext.split("\n");
            d.innertext_maxline = array.length;
            if (d.innertext_maxline > 1)
                d.innertext_maxlength = d3.max(array, t => t.length);
            else
                d.innertext_maxlength = d.innertext.length;

        });
    link.append("path")
        .attr("class", "sankey-link")
        .attr("d", l => l.path)
        .style("stroke-width", function (d) { return Math.max(1, d.width); })
        .style("opacity", linkopacity)
        .style("stroke", function (link, i) {
            return link.circular ? linkreversecolor : linkforwardcolor
        })

    link.append("title")
        .text(function (d) {
            return d.source.innertext + " → " + d.target.innertext + " = " + (d.value);
        });
};

function makebadge(event = null) {
    let ut = badgetextsize * (sankey.size()[1] / height / 1.33);
    badge = link.append("foreignObject")
        .attr("class", "sankey-link badge")
        .attr("_sourcename", l => l.source.name)
        .attr("_targetname", l => l.target.name)
        .attr("x", l => l.source.x1 + 3)
        .attr("y", l => l.y0)
        .attr("width", l => badgetextsize + l.innertext_maxlength * 9)
        .attr("height", l => l.innertext_maxline * badgetextsize)
        .append("xhtml:div")
        .append("button")
        .attr("type", "button")
        .attr("style", function (l) {
            return `\
            filter: drop-shadow(3px 3px 3px rgba(60, 60, 60, 0.432));\
            min-height: ${ut}px;\
            min-width: ${ut}px;\
            background: ${badgebgcolor};\
            font-weight: ${ut < 10 ? 'normal' : 'bold'};\
            color: ${badgetextcolor};\
            font-size: ${ut}pt;\
            border: none;\
            padding: ${ut / 10};\
            stroke: black;\
            outline: none;\
            border-radius: ${ut / 2}px ${ut / 2}px ${ut / 2}px ${ut / 2}px;\
            `})
        .attr("mouseover", "this.style.filter='drop-shadow(3px 5px 5px rgba(0, 0, 0, 0.45))';")
        .attr("mouseout", "this.style.filter='drop-shadow(3px 3px 3px rgba(60, 60, 60, 0.432))';")

    if (link.value != link.innertext) {
        let result = link.value.match(/(href=[\"\'][^\"\']+[\"\'])/g)
        badge = badge
            .attr("onclick", 'transform: scale(.95);location.' + result[1])
            .style("cursor", "pointer")
    } else {
        badge = badge.attr("disabled", false)
    }
    badge.html(l => l.value)

};

function makearrow(event = null) {
    arrowsG = linkG.data(sankeyLinks)
        .enter()
        .append("g")
        .attr("class", "g-arrow")
        .attr("_sourcename", l => l.source.name)
        .attr("_targetname", l => l.target.name)
        .call(appendArrows, arrowlen, arrowgaplen, arrowheadsize, arrowcolor)

    function updateDash() {
        arrowsG.selectAll(".g-arrow path")
            .style("stroke-dashoffset", percentageOffset * maxOffset)

        percentageOffset = percentageOffset <= 0.0 ? 10 : percentageOffset - 0.01
    }

    arrowsG.selectAll(".g-arrow path")
        .style("stroke-width", l => l.width / 4)
        .style("stroke-dasharray", "10,40")
    if (animateDash == null)
        animateDash = setInterval(updateDash, duration);

    const container = d3.select('svg').classed('container', true);

}

/* thanks for https://embed.plnkr.co/plunk/JjvFDnYpEc68hpzi */
function dragmove(d) {
    var rectY = d3.select(this).select("rect").attr("y"),
        rectX = d3.select(this).select("rect").attr("x");

    d.x0 = d.x0 + d3.event.dx;
    d.x1 = d.x1 + d3.event.dx;
    d.y0 = d.y0 + d3.event.dy;
    d.y1 = d.y1 + d3.event.dy;

    var yTranslate = d.y0 - rectY,
        xTranslate = d.x0 - rectX;

    d3.select(this).attr("transform", moved = "translate(" + xTranslate + "," + yTranslate + ")");
    d3.selectAll(".sankey-link").attr("d", l => l.path)

    d3.selectAll(".g-arrow,foreignObject.sankey-link").remove()
    makearrow(d3.event);
    makebadge(d3.event);
    sankey.update(sankeyData);
}

function animation_on() {
    arrowsG.selectAll(".g-arrow path")
        .style("stroke", arrowcolor)
};
function animation_off() {
    arrowsG.selectAll(".g-arrow path")
        .style("stroke", null);
};


function arrow_on() {
    arrowsG.selectAll('.arrow-head')
        .style("fill", arrowcolor);
};
function arrow_off() {
    arrowsG.selectAll('.arrow-head')
        .style("fill", null);
};


function value_on() {
    badge.attr("hidden", null)
};
function value_off() {
    badge.attr("hidden", true)
};


function nodetext_left() {
    node.selectAll(".node-label")
        .style("text-align", "left")
    node.selectAll("foreignObject").attr("x", function (l) {
        return l.x0;
    })
}
function nodetext_center() {
    node.selectAll(".node-label")
        .style("text-align", "center")
    node.selectAll("foreignObject").attr("x", function (l) {
        return l.x0 - nodewidth / 2 - this.clientWidth / 2.66;
    })
}
function nodetext_right() {
    node.selectAll(".node-label")
        .style("text-align", "right")
    node.selectAll("foreignObject").attr("x", function (l) {
        return l.x0 - this.clientWidth + nodewidth;
    })
}

function redraw(event = null) {
    width = Number(document.SVGSIZE.width.value);
    height = Number(document.SVGSIZE.height.value);

    margin = { top: height * 0.05, right: width * 0.1, bottom: height * 0.13, left: width * 0.1 };
    outerwidth = width + margin.left + margin.right;
    outerheight = height + margin.top + margin.bottom;

    nodewidth = Number(document.getElementById('nodewidth').value)
    nodepaddingratio = parseFloat(document.getElementById('nodepaddingratio').value)
    strokeopacity = parseFloat(document.getElementById('strokeopacity').value)
    linkopacity = 1 - strokeopacity;
    nodeiterations = Number(document.getElementById('nodeiterations').value)
    nodeopacity = parseFloat(document.getElementById('nodeopacity').value)
    hideopacity = parseFloat(document.getElementById('hideopacity').value)
    nodetextfontsize = Number(document.getElementById('nodetextfontsize').value)
    badgetextsize = Number(document.getElementById('badgetextsize').value)
    nodetextfontweight = Number(document.getElementById('nodetextfontweight').value)
    arrowlen = Number(document.getElementById('arrowlen').value)
    arrowgaplen = Number(document.getElementById('arrowgaplen').value)
    arrowheadsize = Number(document.getElementById('arrowheadsize').value)
    circularLinkGap = Number(document.getElementById('circularLinkGap').value)
    maxOffset = Number(document.getElementById('maxOffset').value)

    linkforwardcolor = document.getElementById('linkforwardcolor').value;
    linkreversecolor = document.getElementById('linkreversecolor').value;

    highlighttype = document.getElementById('highlighttype').value;

    _nv = document.getElementById('nodealign').value;
    if (_nv == "Left")
        nodealign = d3.sankeyLeft;
    else if (_nv == "Center")
        nodealign = d3.sankeyCenter;
    else if (_nv == "Right")
        nodealign = d3.sankeyRight;
    else
        nodealign = d3.sankeyJustify;

    _nc = document.getElementById('nodecolor').value;
    if (_nc == "#ffffff")
        nodecolor = d3.scaleSequential(d3.interpolateCool).domain([0, width]);
    else
        nodecolor = function (d) { return _nc }

    _ntc = document.getElementById('nodetextfontcolor').value
    if (_ntc == "#ffffff")
        nodetextfontcolor = nodecolor;
    else
        nodetextfontcolor = function (d) { return _ntc }

    document.f_nodepos.R4.forEach(function (d) {
        if (d.checked)
            textalign = d.value;
    })
    fontfamily = document.getElementById('fontfamily').value;
    arrowcolor = document.getElementById('arrowcolor').value;

    badgebgcolor = document.getElementById('badgebgcolor').value;
    badgetextcolor = document.getElementById('badgetextcolor').value;
    scale = 1

    if (event == null)
        init(event);
    else
        d3.selectAll(".nodes,.links").remove();
    makenode(event);
    makelink(event);
    makearrow(event);
    makebadge(event);

    if (document.f_animation.R1[0].checked) {
        animation_on()
    } else {
        animation_off()
    }

    if (document.f_arrow.R2[0].checked) {
        arrow_on()
    } else {
        arrow_off()
    }

    if (document.f_value.R3[0].checked) {
        value_on()
    } else {
        value_off()
    }

    if (textalign == "left")
        return nodetext_left();
    else if (textalign == "right")
        return nodetext_right();
    else
        return nodetext_center();

};
