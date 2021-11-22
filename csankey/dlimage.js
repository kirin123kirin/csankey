function getSVGdata(SVGElement = document.querySelector("svg")) {
    SVGElement.version = 1.1
    SVGElement.xmlns = 'http://www.w3.org/2000/svg'

    // SVGElement.querySelectorAll("text,button")
    // .forEach(function(f) {
    // /* thanks for https://qiita.com/Nikkely/items/aa485ebdbec51e49ecbc */
    //   let queue = []
    //   queue.push(f)
    //   while (queue.length != 0) {
    //     let element = queue.pop()

    //     let computedStyle = window.getComputedStyle(element, '')
    //     for (let property of computedStyle) {
    //       element.style[property] = computedStyle.getPropertyValue(property)
    //     }

    //     let children = element.children

    //     for (let child of children) {
    //       queue.push(child)
    //     }
    //   }
    // })
    return new XMLSerializer().serializeToString(SVGElement);
}

/* thanks for https://kuroeveryday.blogspot.com/2016/05/file-download-from-browser.html?m=1 */
function browser_download(content, mimeType, savefilename) {
    // BOM mojibake
    let bom = new Uint8Array([0xEF, 0xBB, 0xBF]);
    let blob = new Blob([bom, content], { type: mimeType });

    let a = document.createElement('a');
    a.download = savefilename;
    a.target = '_blank';

    if (window.navigator.msSaveBlob) {
        // for IE
        window.navigator.msSaveBlob(blob, savefilename)
    }
    else if (window.URL && window.URL.createObjectURL) {
        // for Firefox
        a.href = window.URL.createObjectURL(blob);
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
    }
    else if (window.webkitURL && window.webkitURL.createObject) {
        // for Chrome
        a.href = window.webkitURL.createObjectURL(blob);
        a.click();
    }
    else {
        // for Safari
        window.open('data:' + mimeType + ';charset=utf-8;base64,' + window.Base64.encode(content), '_blank');
    }
}

/* thanks for https://zenn.dev/skryo/articles/7d7f1ce601510b */
function download_PNG() {
    let elem = document.querySelector("svg");
    let svgData = getSVGdata(elem)
    let canvas = document.createElement("canvas");

    let height = document.F1.T1[1].value;
    let width = document.F1.T1[0].value;
    canvas.width = elem.width.baseVal.value;
    canvas.height = elem.height.baseVal.value;

    let ctx = canvas.getContext("2d");
    let image = new Image;
    image.onload = function () {

        if (width && height)
            ctx.drawImage(image, 0, 0, width, height);
        else if (width)
            ctx.drawImage(image, 0, 0, width, canvas.height);
        else if (height)
            ctx.drawImage(image, 0, 0, canvas.width, height);
        else
            ctx.drawImage(image, 0, 0);

        var a = document.createElement("a");
        a.href = canvas.toDataURL("image/png");
        a.setAttribute("download", "sankeydiagram.png");
        a.dispatchEvent(new MouseEvent("click"));
    }
    image.src = "data:image/svg+xml;charset=utf-8;base64," + btoa(unescape(encodeURIComponent(svgData)));

}

/* thank you for https://kuroeveryday.blogspot.com/2016/05/file-download-from-browser.html */
function download_SVG() {
    let elem = document.querySelector("svg");
    browser_download(getSVGdata(elem), 'image/svg+xml', 'sankeydiagram.svg')
}

