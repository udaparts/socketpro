var SPA = function (spAddr, debug, df) {
    var q, js, win = window, nav = navigator, cs = {};
    function loadScript() {
        q = 'json=' + (win.JSON ? 1 : 0) + '&ws=' + (win.WebSocket ? 1 : 0) + '&swfobject=' + (win.swfobject ? 1 : 0) + '&fv=' + detectFlash();
	if (win.File && win.FileReader && win.FileList && win.Blob)
		q += '&fapi=1';
	if (win.ArrayBuffer)
		q += '&ab=1';
	if (win.indexedDB)
		q += '&idb=1';
        spAddr += ('?' + q);
        js = document.createElement('script');
        js.setAttribute('type', 'text/javascript');
        js.setAttribute('src', spAddr);
        js.setAttribute('charset', 'utf-8');
        document.body.appendChild(js);
    }
    function detectFlash() {
        if (!df)
            return 0;
        var ctrl, t, mt, v = 0, plugins = nav.plugins;
        if (win.ActiveXObject) {
            try {
                ctrl = new ActiveXObject('ShockwaveFlash.ShockwaveFlash');
                v = ctrl.GetVariable('$version').substring(4);
                v = v.split(',');
                v = parseFloat(v[0] + '.' + v[1]);
            }
            catch (e) {
            }
        }
        else if (plugins && plugins.length) {
            t = 'application/x-shockwave-flash';
            mt = nav.mimeTypes;
            if (mt && mt[t] && mt[t].enabledPlugin && mt[t].enabledPlugin.description) {
                v = mt[t].enabledPlugin.description.replace(/^.*?([0-9]+)\.([0-9])+.*$/, '$1,$2').split(',');
                v = parseFloat(v[0] + '.' + v[1]);
            }
        }
        return v;
    }
    cs.onLoad = function () {
        var load = win.onUHTTPLoaded;
        if (load)
            load();
    };
    cs.version = function () {
        return 1.2;
    };

    cs.debug = function () {
        return debug;
    };

    cs.log = function (str) {
        if (debug && win.console && win.console.log)
            win.console.log(str);
    };

    loadScript();
    return cs;
} ('spadapter.js', 1);
