if (!this.UJSON)
    this.UJSON = {};

(function() {

    function f(n) {
        // Format integers to have at least two digits.
        return n < 10 ? '0' + n : n;
    }

    if (typeof Date.prototype.toJSON !== 'function') {

        Date.prototype.toJSON = function(key) {

            return isFinite(this.valueOf()) ?
                   this.getUTCFullYear() + '-' +
                 f(this.getUTCMonth() + 1) + '-' +
                 f(this.getUTCDate()) + 'T' +
                 f(this.getUTCHours()) + ':' +
                 f(this.getUTCMinutes()) + ':' +
                 f(this.getUTCSeconds()) + 'Z' : null;
        };

        String.prototype.toJSON =
        Number.prototype.toJSON =
        Boolean.prototype.toJSON = function(key) {
            return this.valueOf();
        };
    }

    var cx = /[\u0000\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
        escapable = /[\\\"\x00-\x1f\x7f-\x9f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,
        gap,
        indent,
        meta = {    // table of character substitutions
            '\b': '\\b',
            '\t': '\\t',
            '\n': '\\n',
            '\f': '\\f',
            '\r': '\\r',
            '"': '\\"',
            '\\': '\\\\'
        },
        rep;


    function quote(string) {

        // If the string contains no control characters, no quote characters, and no
        // backslash characters, then we can safely slap some quotes around it.
        // Otherwise we must also replace the offending characters with safe escape
        // sequences.

        escapable.lastIndex = 0;
        return escapable.test(string) ?
            '"' + string.replace(escapable, function(a) {
                var c = meta[a];
                return typeof c === 'string' ? c :
                    '\\u' + ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
            }) + '"' :
            '"' + string + '"';
    }


    function str(key, holder) {

        // Produce a string from holder[key].

        var i,          // The loop counter.
            k,          // The member key.
            v,          // The member value.
            length,
            mind = gap,
            partial,
            value = holder[key];

        // If the value has a toJSON method, call it to obtain a replacement value.

        if (value && typeof value === 'object' &&
                typeof value.toJSON === 'function') {
            value = value.toJSON(key);
        }

        // If we were called with a replacer function, then call the replacer to
        // obtain a replacement value.

        if (typeof rep === 'function') {
            value = rep.call(holder, key, value);
        }

        // What happens next depends on the value's type.

        switch (typeof value) {
            case 'string':
                return quote(value);

            case 'number':

                // JSON numbers must be finite. Encode non-finite numbers as null.

                return isFinite(value) ? String(value) : 'null';

            case 'boolean':
            case 'null':

                // If the value is a boolean or null, convert it to a string. Note:
                // typeof null does not produce 'null'. The case is included here in
                // the remote chance that this gets fixed someday.

                return String(value);

                // If the type is 'object', we might be dealing with an object or an array or
                // null.

            case 'object':

                // Due to a specification blunder in ECMAScript, typeof null is 'object',
                // so watch out for that case.

                if (!value) {
                    return 'null';
                }

                // Make an array to hold the partial results of stringifying this object value.

                gap += indent;
                partial = [];

                // Is the value an array?

                if (Object.prototype.toString.apply(value) === '[object Array]') {

                    // The value is an array. Stringify every element. Use null as a placeholder
                    // for non-JSON values.

                    length = value.length;
                    for (i = 0; i < length; i += 1) {
                        partial[i] = str(i, value) || 'null';
                    }

                    // Join all of the elements together, separated with commas, and wrap them in
                    // brackets.

                    v = partial.length === 0 ? '[]' :
                    gap ? '[\n' + gap +
                            partial.join(',\n' + gap) + '\n' +
                                mind + ']' :
                          '[' + partial.join(',') + ']';
                    gap = mind;
                    return v;
                }

                // If the replacer is an array, use it to select the members to be stringified.

                if (rep && typeof rep === 'object') {
                    length = rep.length;
                    for (i = 0; i < length; i += 1) {
                        k = rep[i];
                        if (typeof k === 'string') {
                            v = str(k, value);
                            if (v) {
                                partial.push(quote(k) + (gap ? ': ' : ':') + v);
                            }
                        }
                    }
                } else {

                    // Otherwise, iterate through all of the keys in the object.

                    for (k in value) {
                        if (Object.hasOwnProperty.call(value, k)) {
                            v = str(k, value);
                            if (v) {
                                partial.push(quote(k) + (gap ? ': ' : ':') + v);
                            }
                        }
                    }
                }

                // Join all of the member texts together, separated with commas,
                // and wrap them in braces.

                v = partial.length === 0 ? '{}' :
                gap ? '{\n' + gap + partial.join(',\n' + gap) + '\n' +
                        mind + '}' : '{' + partial.join(',') + '}';
                gap = mind;
                return v;
        }
    }

    // If the JSON object does not yet have a stringify method, give it one.

    if (typeof UJSON.stringify !== 'function') {
        UJSON.stringify = function(value, replacer, space) {

            // The stringify method takes a value and an optional replacer, and an optional
            // space parameter, and returns a JSON text. The replacer can be a function
            // that can replace values, or an array of strings that will select the keys.
            // A default replacer method can be provided. Use of the space parameter can
            // produce text that is more easily readable.

            var i;
            gap = '';
            indent = '';

            // If the space parameter is a number, make an indent string containing that
            // many spaces.

            if (typeof space === 'number') {
                for (i = 0; i < space; i += 1) {
                    indent += ' ';
                }

                // If the space parameter is a string, it will be used as the indent string.

            } else if (typeof space === 'string') {
                indent = space;
            }

            // If there is a replacer, it must be a function or an array.
            // Otherwise, throw an error.

            rep = replacer;
            if (replacer && typeof replacer !== 'function' &&
                    (typeof replacer !== 'object' ||
                     typeof replacer.length !== 'number')) {
                throw new Error('JSON.stringify');
            }

            // Make a fake root object containing our value under the key of ''.
            // Return the result of stringifying the value.

            return str('', { '': value });
        };
    }


    // If the JSON object does not yet have a parse method, give it one.

    if (typeof UJSON.parse !== 'function') {
        UJSON.parse = function(text, reviver) {

            // The parse method takes a text and an optional reviver function, and returns
            // a JavaScript value if the text is a valid JSON text.

            var j;

            function walk(holder, key) {

                // The walk method is used to recursively walk the resulting structure so
                // that modifications can be made.

                var k, v, value = holder[key];
                if (value && typeof value === 'object') {
                    for (k in value) {
                        if (Object.hasOwnProperty.call(value, k)) {
                            v = walk(value, k);
                            if (v !== undefined) {
                                value[k] = v;
                            } else {
                                delete value[k];
                            }
                        }
                    }
                }
                return reviver.call(holder, key, value);
            }


            // Parsing happens in four stages. In the first stage, we replace certain
            // Unicode characters with escape sequences. JavaScript handles many characters
            // incorrectly, either silently deleting them, or treating them as line endings.

            text = String(text);
            cx.lastIndex = 0;
            if (cx.test(text)) {
                text = text.replace(cx, function(a) {
                    return '\\u' +
                        ('0000' + a.charCodeAt(0).toString(16)).slice(-4);
                });
            }

            // In the second stage, we run the text against regular expressions that look
            // for non-JSON patterns. We are especially concerned with '()' and 'new'
            // because they can cause invocation, and '=' because it can cause mutation.
            // But just to be safe, we want to reject all unexpected forms.

            // We split the second stage into 4 regexp operations in order to work around
            // crippling inefficiencies in IE's and Safari's regexp engines. First we
            // replace the JSON backslash pairs with '@' (a non-JSON character). Second, we
            // replace all simple value tokens with ']' characters. Third, we delete all
            // open brackets that follow a colon or comma or that begin the text. Finally,
            // we look to see that the remaining characters are only whitespace or ']' or
            // ',' or ':' or '{' or '}'. If that is so, then the text is safe for eval.

            if (/^[\],:{}\s]*$/.
test(text.replace(/\\(?:["\\\/bfnrt]|u[0-9a-fA-F]{4})/g, '@').
replace(/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g, ']').
replace(/(?:^|:|,)(?:\s*\[)+/g, ''))) {

                // In the third stage we use the eval function to compile the text into a
                // JavaScript structure. The '{' operator is subject to a syntactic ambiguity
                // in JavaScript: it can begin a block or an object literal. We wrap the text
                // in parens to eliminate the ambiguity.

                j = eval('(' + text + ')');

                // In the optional fourth stage, we recursively walk the new structure, passing
                // each name/value pair to a reviver function for possible transformation.

                return typeof reviver === 'function' ?
                    walk({ '': j }, '') : j;
            }

            // If the text is not JSON parseable, then a SyntaxError is thrown.

            throw new SyntaxError('JSON.parse');
        };
    }
} ());


//if (!this.UJSON)
//    UJSON = { _ESCAPES: /\\["\\\/bfnrtu]/g, _VALUES: /"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g, _BRACKETS: /(?:^|:|,)(?:\s*\[)+/g, _INVALID: /^[\],:{}\s]*$/, _SPECIAL_CHARS: /["\\\x00-\x1f\x7f-\x9f]/g, _PARSE_DATE: /^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z$/, _CHARS: { '\b': '\\b', '\t': '\\t', '\n': '\\n', '\f': '\\f', '\r': '\\r', '"': '\\"', '\\': '\\\\' }, _applyFilter: function(data, filter) {
//        var walk = function(k, v) {
//            var i, n; if (v && typeof v === 'object') { for (i in v) { if (v.hasOwnProperty(i)) { n = walk(i, v[i]); if (n === undefined) { delete v[i]; } else { v[i] = n; } } } }
//            return filter(k, v);
//        }; if (typeof (filter) == 'function') { walk('', data); }
//        return data;
//    },
//        isValid: function(str) {
//            if (typeof (str) != 'string' || str.length == 0) { return false; }
//            return this._INVALID.test(str.replace(this._ESCAPES, '@').replace(this._VALUES, ']').replace(this._BRACKETS, ''));
//        }, dateToString: function(d) {
//            function _zeroPad(v) { return v < 10 ? '0' + v : v; }
//            return '"' + d.getUTCFullYear() + '-' +
//_zeroPad(d.getUTCMonth() + 1) + '-' +
//_zeroPad(d.getUTCDate()) + 'T' +
//_zeroPad(d.getUTCHours()) + ':' +
//_zeroPad(d.getUTCMinutes()) + ':' +
//_zeroPad(d.getUTCSeconds()) + 'Z"';
//        },
//        stringToDate: function(str) {
//            if (this._PARSE_DATE.test(str)) {
//                var d = new Date(); d.setUTCFullYear(RegExp.$1, (RegExp.$2 | 0) - 1, RegExp.$3);
//                d.setUTCHours(RegExp.$4, RegExp.$5, RegExp.$6); return d;
//            }
//        },
//        parse: function(s, filter) {
//            if (this.isValid(s)) {
//                return this._applyFilter(eval('(' + s + ')'), filter);
//            }
//            throw new SyntaxError('parseJSON');
//        },
//        stringify: function(o, w, d) {
//            var J = this, m = J._CHARS, str_re = this._SPECIAL_CHARS, pstack = []; var _char = function(c) {
//                if (!m[c]) {
//                    var a = c.charCodeAt(); m[c] = '\\u00' + Math.floor(a / 16).toString(16) +
//(a % 16).toString(16);
//                }
//                return m[c];
//            }; var _string = function(s) { return '"' + s.replace(str_re, _char) + '"'; }; var _date = J.dateToString; var _stringify = function(o, w, d) {
//                var t = typeof o, i, len, j, k, v, vt, a; if (t === 'string') { return _string(o); }
//                if (t === 'boolean' || o instanceof Boolean) { return String(o); }
//                if (t === 'number' || o instanceof Number) { return isFinite(o) ? String(o) : 'null'; }
//                if (o instanceof Date) { return _date(o); }
//                if (o instanceof Array) {
//                    for (i = pstack.length - 1; i >= 0; --i) { if (pstack[i] === o) { return 'null'; } }
//                    pstack[pstack.length] = o; a = []; if (d > 0) { for (i = o.length - 1; i >= 0; --i) { a[i] = _stringify(o[i], w, d - 1) || 'null'; } }
//                    pstack.pop(); return '[' + a.join(',') + ']';
//                }
//                if (t === 'object') {
//                    if (!o) { return 'null'; }
//                    for (i = pstack.length - 1; i >= 0; --i) { if (pstack[i] === o) { return 'null'; } }
//                    pstack[pstack.length] = o; a = []; if (d > 0) { if (w) { for (i = 0, j = 0, len = w.length; i < len; ++i) { if (typeof w[i] === 'string') { v = _stringify(o[w[i]], w, d - 1); if (v) { a[j++] = _string(w[i]) + ':' + v; } } } } else { j = 0; for (k in o) { if (typeof k === 'string' && o.hasOwnProperty && o.hasOwnProperty(k)) { v = _stringify(o[k], w, d - 1); if (v) { a[j++] = _string(k) + ':' + v; } } } } }
//                    pstack.pop(); return '{' + a.join(',') + '}';
//                }
//                return undefined;
//            }; d = d >= 0 ? d : 1 / 0; return _stringify(o, w, d);
//        } 
//};;;

/*
UDAParts UJSON/XML -- RPC JavaScript remoting library

Copyright (c) 2008, UDAParts. All rights reserved.

1.  Use at your own risk without any warranty.
2.  Must keep this header in your project. 
3.  YOU MUST NOT CLAIM you write this library.
*/

/*
    1.  UHTTP -- singleton
    2.  defaultConfig -- an optional url address for processing UJSON or XML request. 
        If not specified, it will be set from current web page address.
    
    For example:
    var dc = {protocol:'http', hostname:'www.myhost.com', port:80, pathname:'/subpath/processing.aspx'};
*/
if(!window.SocketProAdapter) window.SocketProAdapter = {};
if(!SocketProAdapter.ClientSide) SocketProAdapter.ClientSide = {};

var UHTTP = SocketProAdapter.ClientSide.HTTP = function(defaultConfig) {
    var qMeta = [];
    var qCall = [];
    var callIndex = 0;
    var me = {};
    //var strUserAgent = window.navigator.userAgent.toLowerCase();
    //var bOpera = (strUserAgent.indexOf('opera') != -1);
    //var bIE = (strUserAgent.indexOf('msie') != -1);
    function toInt(d) {
        if (typeof (d) == 'boolean') return d ? 1 : 0;
        if (!d) d = "0";
        var str = String(d); //d -- a string, float or a number
        return parseInt(str, 10);
    }

    if (!defaultConfig) defaultConfig = {};
    var dc = defaultConfig;
    var loc = document.location;
    if (typeof dc.protocol == 'string' && dc.protocol.length > 0)
        dc.protocol = dc.protocol.toLowerCase();
    else
        dc.protocol = loc.protocol.toLowerCase();
    if (typeof dc.hostname == 'string' && dc.hostname.length > 0)
        dc.hostname = dc.hostname.toLowerCase();
    else
        dc.hostname = loc.hostname.toLowerCase();
    if (!dc.port || typeof dc.port != 'number')
        dc.port = toInt(loc.port);
    if (!dc.pathname || typeof dc.pathname != 'string')
        dc.pathname = loc.pathname;

    /*
    1.  Default configuration like {protocol:'https', hostname:'www.myhost.com', port:443, pathname:'/subpath/processing.aspx'}
    */
    me.defaultConfig = dc;

    function onProcessed(index, data) {
        for (var n = 0; n < qCall.length; n++) {
            var req = qCall[n];
            if (index == req.getCallIndex()) {
                var cb = req.getCallback();
                req.abort();
                if (cb) cb(req, data);
                break;
            }
        }
    }

    function createAjaxObject() {
        var xmlHttp;
        try {
            xmlHttp = new XMLHttpRequest(); // Firefox, Opera 8.0+, Safari
        }
        catch (e) {
            // Internet Explorer
            try {
                xmlHttp = new ActiveXObject("Msxml2.XMLHTTP");
            }
            catch (e) {
                xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
            }
        }
        return xmlHttp;
    }

    var XMLRPCMessage = function() {
        this.toXml = function(methodName, params) {
            var xml = "";
            xml += "<?xml version=\"1.0\"?>\n";
            xml += "<methodCall>\n";
            xml += ("<methodName>" + methodName + "</methodName>\n");
            xml += "<params>\n";

            // do individual parameters
            for (var i = 0; i < params.length; i++) {
                var data = params[i];
                xml += "<param>\n";
                if (data == null || typeof data == 'undefined')
                    xml += ("<value>" + "<nil/>" + "</value>\n");
                else
                    xml += ("<value>" + getParam(dataTypeOf(data), data) + "</value>\n");
                xml += "</param>\n";
            }
            xml += "</params>\n";
            xml += "</methodCall>";
            return xml; // for now
        };

        function dateToISO8601(date) {
            var year = new String(date.getYear());
            var month = leadingZero(new String(date.getMonth()));
            var day = leadingZero(new String(date.getDate()));
            var time = leadingZero(new String(date.getHours())) + ":" + leadingZero(new String(date.getMinutes())) + ":" + leadingZero(new String(date.getSeconds()));
            var converted = year + month + day + "T" + time;
            return converted;
        }

        function leadingZero(n) {
            if (n.length == 1)
                n = "0" + n;
            return n;
        }

        function getParam(type, data) {
            var xml;
            switch (type) {
                case "date":
                    xml = doDate(data);
                    break;
                case "array":
                    xml = doArray(data);
                    break;
                case "struct":
                    xml = doStruct(data);
                    break;
                case "boolean":
                    xml = doBoolean(data);
                    break;
                default:
                    xml = doValue(type, data);
                    break;
            }
            return xml;
        }

        function doStruct(data) {
            var xml = "<struct>\n";
            for (var i in data) {
                xml += "<member>\n";
                xml += "<name>" + i + "</name>\n";
                if (data[i] == null || typeof (data[i]) == 'undefined')
                    xml += ("<value>" + "<nil/>" + "</value>\n");
                else
                    xml += ("<value>" + getParam(dataTypeOf(data[i]), data[i]) + "</value>\n");
                xml += "</member>\n";
            }
            xml += "</struct>\n";
            return xml;
        }
        function doArray(data) {
            var xml = "<array><data>\n";
            for (var i = 0; i < data.length; i++) {
                if (data[i] == null || typeof (data[i]) == 'undefined')
                    xml += ("<value>" + "<nil/>" + "</value>\n");
                else
                    xml += ("<value>" + getParam(dataTypeOf(data[i]), data[i]) + "</value>\n");
            }
            xml += "</data></array>\n";
            return xml;
        }

        function doDate(data) {
            var xml = "<dateTime.iso8601>";
            xml += dateToISO8601(data);
            xml += "</dateTime.iso8601>";
            return xml;
        }
        function doBoolean(data) {
            var value = data ? 1 : 0;
            var xml = "<boolean>" + value + "</boolean>";
            return xml;
        }
        function doValue(type, data) {
            var xml = "<" + type + ">"
            if (type == 'string') {
                var str = data.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
                xml += (str + "</" + type + ">");
            }
            else
                xml += (data + "</" + type + ">");
            return xml;
        }
        function dataTypeOf(o) {
            var type = typeof (o);
            type = type.toLowerCase();
            switch (type) {
                case "number":
                    if (Math.round(o) == o)
                        type = 'i4';
                    else
                        type = 'double';
                    break;
                case 'object':
                    var con = o.constructor;
                    if (con == Date)
                        type = 'date';
                    else if (con == Array)
                        type = 'array';
                    else
                        type = 'struct';
                    break;
                default:
                    break;
            }
            return type;
        }
        return this;
    } ();

    function JSMethod(name, isXML, version, config) {
        var urlScript;
        var callback;
        var urlCall;
        var jsScript = null;
        var cIndex = 0;
        var xmlHttp = null;
        if (!name || typeof name != 'string')
            throw new Error(500, 'Req name must be a valid string.');
        var byScript = true;
        var me = {};
        var m_pl = {};
        var ajaxDone = true;
        if (!version) version = 1.1;
        function init() {
            if (typeof config.protocol == 'string')
                config.protocol = config.protocol.toLowerCase();
            switch (config.protocol) {
                case 'https:':
                case 'https':
                    if (!config.port)
                        config.port = 443;
                    config.protocol = "https:";
                    break;
                case 'http:':
                case 'http':
                    if (!config.port)
                        config.port = 80;
                    config.protocol = 'http:';
                    break;
                default:
                    break;
            }

            var loc = document.location;
            var url = { protocol: loc.protocol.toLowerCase(),
                hostname: loc.hostname.toLowerCase(),
                port: loc.port,
                pathname: loc.pathname
            };
            if (!config.protocol)
                config.protocol = url.protocol;
            if (!config.hostname)
                config.hostname = url.hostname;
            url.port = toInt(url.port);
            do {
                if (url.port == 0) {
                    switch (url.protocol) {
                        case 'https:':
                        case 'https':
                            url.port = 443;
                            break;
                        case 'http:':
                        case 'http':
                            url.port = 80;
                            break;
                        default:
                            break;
                    }
                }
                if (!config.port)
                    config.port = url.port;
                if (url.protocol == config.protocol && url.hostname == config.hostname && url.port == config.port)
                    byScript = false;
            } while (false);
            me.config = config;
        }

        function setUrls() {
            urlCall = '';
            urlScript = '';
            urlCall = me.config.protocol + '//' + me.config.hostname;
            if (me.config.port) {
                urlCall += ':';
                urlCall += me.config.port;
            }
            if (byScript) {
                urlScript = urlCall;
                if (typeof (me.config.pathname) == 'string' && me.config.pathname.length > 0) {
                    var pos = me.config.pathname.indexOf('/');
                    if (pos != 0)
                        me.config.pathname = '/' + me.config.pathname;
                }
                urlScript += me.config.pathname;
            }
            if (me.config.pathname) {
                pos = me.config.pathname.indexOf('.');
                if (pos == -1)
                    urlCall += me.config.pathname;
                else
                    urlCall += me.config.pathname.substr(0, pos);
            }
        }

        function sendDataByScript(strQuery) {
            var script = document.createElement('script');
            script.setAttribute("type", "text/javascript");
            script.setAttribute('src', strQuery);
            script.setAttribute('charset', 'utf-8');
            UHTTP.cb = onProcessed;
            document.body.appendChild(script);
            jsScript = script;
        }

        function ajaxCallback() {
            if (xmlHttp && xmlHttp.readyState == 4) {
                ajaxDone = true;
                if (callback)
                    callback(me, xmlHttp.responseText);
            }
        }

        function doCall(isGet, sync) {
            var strData = '';
            var pos;
            setUrls();
            if (byScript) {
                qCall.push(me);
                strData = urlScript;
                pos = strData.indexOf('?');
                if (pos != (strData.length - 1))
                    strData += '?';
                var temp = "UJS_CB=UHTTP.cb(" + cIndex + ',' + "&UJS_DATA=" + toString(isGet);
                strData += encodeURIComponent(temp);
                sendDataByScript(strData);
            }
            else {
                if (jsScript) {
                    document.body.removeChild(jsScript);
                    jsScript = null;
                }
                strData = toString(isGet);
                if (isGet) {
                    strData = encodeURIComponent(strData);
                    strData = 'UJS_DATA=' + strData;
                }
                xmlHttp = sendDataByAjax(!isGet, strData, sync);
            }
        }

        function sendDataByAjax(post, data, sync) {
            if (!xmlHttp) xmlHttp = createAjaxObject();
            if (!xmlHttp) return;
            //if (bIE) xmlHttp.abort();
            if (!sync) xmlHttp.onreadystatechange = ajaxCallback;
            var strUrl = config.pathname;
            if (!post) {
                var pos = strUrl.indexOf('?');
                if (pos != strUrl.length - 1)
                    strUrl += ('?' + data);
                else
                    strUrl += data;
            }
            ajaxDone = false;
            xmlHttp.open(post ? 'POST' : 'GET', strUrl, !sync);
            if (post)
                xmlHttp.setRequestHeader('Content-length', data ? data.length : 0);
            xmlHttp.setRequestHeader('Content-Type', isXML ? 'text/xml; charset=utf-8' : 'application/json; charset=utf-8');
            xmlHttp.setRequestHeader('Accept', isXML ? 'text/xml,application/json' : 'application/json');
            xmlHttp.send(post ? data : null);
            if (sync) {
                ajaxDone = true;
                ajaxCallback(xmlHttp);
            }
            return xmlHttp;
        }

        function toString(isGet) {
            var str;
            var temp = me.config;
            delete me.config;
            me.params = m_pl;
            me.id = cIndex;
            if (isXML && !isGet && !byScript) {
                var params = [];
                for (var key in me.params)
                    params.push(me.params[key]);
                str = XMLRPCMessage.toXml(name, params);
            }
            else {
                me.version = version;
                str = UJSON.stringify(me);
                delete me.version;
            }
            delete me.params;
            delete me.id;
            me.config = temp;
            return str;
        }

        /*
        It returns if the request will be sent through JavaScript tag.
        */
        me.isCrossSite = function() { return byScript; };

        /*
        request (method) name.
        */
        me.method = name;

        /*
        It returns a unique identification number.
        */
        me.getCallIndex = function() { return cIndex; };

        /*
        abort current asynchronous request, and ignore returning result.
        */
        me.abort = function() {
            if (xmlHttp) xmlHttp.abort();
            for (var n = 0; n < qCall.length; n++) {
                if (qCall[n] == me) {
                    qCall.splice(n, 1);
                    if (jsScript) {
                        document.body.removeChild(jsScript);
                        jsScript = null;
                    }
                    break;
                }
            }
            ajaxDone = true;
        };

        /*
        check if current request is processed and completed.
        */
        me.isCompleted = function() {
            for (var n = 0; n < qCall.length; n++) {
                if (qCall[n] == me) {
                    return false;
                }
            }
            return ajaxDone;
        };

        /*
        get current request identification number.
        */
        me.getCallback = function() {
            return callback;
        };


        /*
        invoke a request from browser to a web server.
            
        1.  cb -- required callback handler.
        2.  isGet -- default to POST. 1 or true for GET
        5.  sync -- default to asynchronous processing, and 1 or true for synchronous processing.
        */
        me.invoke = function(cb, isGet, sync) {
            if (!me.method && typeof (me.method) != 'string')
                throw new Error(500, "Request name not specified properly.");
            if (typeof (cb) == 'string')
                cb = eval(cb);
            if (typeof (cb) != 'function')
                throw new Error(500, "Callback function required");
            if (!me.isCompleted())
                throw new Error(500, "Previous request not completed yet.");
            cIndex = ++callIndex;
            callback = cb;
            doCall(isGet, sync);
            return callIndex;
        };

        /*
        clear all of paremeters.
        */
        me.clear = function() {
            m_pl = {};
        };

        /*
        add parameter name and its value.
            
        1.  name -- required parameter name.
        2.  value -- parameter value.
        */
        me.add = function(name, value) {
            if (typeof name != 'string' || !name)
                throw new Error(500, 'Bad parameter name.');
            m_pl[name] = value;
        };

        /*
        get JavaScript remoting object used for sending request from browser to web server.
        */
        me.getScript = function() {
            return (jsScript || xmlHttp);
        };
        init();
        return me;
    }

    /*
    1.  name -- required request (method) name.
    2.  isXML -- default to json. 
    3.  version -- optional, request version number.
    4.  config -- optional. If not specified, defaultConfig will be used instead.
    */
    me.createRequest = function(name, isXML, version, config) {
        if (typeof config == 'string')
            eval(config);
        else if (!config)
            config = UHTTP.defaultConfig;
        return new JSMethod(name, isXML, version, config);
    };

    me.Chat = function() {
        var c = {};
        c.onMessage = null;
        c.isXML = 0;
        c.version = 1.1;
        c.sync = 0;
        c.isGet = 0;
        c.crossSite = 0;
        var listening = 0;
        var config = me.defaultConfig;
        var callback = function(req, data) {
            var oMsg;
            var bS = 0;
            if (data && typeof (data) == 'object')
                oMsg = data;
            else if (typeof (data) == 'string' && data.length > 0)
                oMsg = UJSON.parse(data);
            if (typeof (oMsg) != 'object' || typeof (req) != 'object')
                return;
            if (oMsg.messages && oMsg.messages.length) {
                var s = oMsg.messages[0];
                if (s.msg == '__UCOMET_MESSAGE__')
                    bS = 1;
            }
            if (req.method == 'enter')
                config.chatId = oMsg.ret;
            if (c.onMessage && !bS)
                c.onMessage(req, oMsg);
            if (req.method == 'listen')
                listening = false;
            var ok = c.chatting();
            if (ok && ((req.method == 'listen' && oMsg.ret !== false) || req.method == 'enter'))
                c.listen();
        };

        var addEvent = function(obj, type, fn, useCapture) {
            if (obj.addEventListener)
                obj.addEventListener(type, fn, useCapture);
            else if (obj.attachEvent) {
                obj["e" + type + fn] = fn;
                obj[type + fn] = function() { obj["e" + type + fn](window.event); };
                obj.attachEvent("on" + type, obj[type + fn]);
            }
        };

        c.chatting = function() {
            return (typeof (config.chatId) == 'string' && config.chatId.length == 38);
        };

        c.chatId = function() {
            if (c.chatting())
                return config.chatId;
            return "";
        };

        c.speak = function(msg, groups) {
            if (!isOk())
                return 0;
            var req = me.createRequest('speak', c.isXML, c.version, UHTTP.defaultConfig);
            setInternal(req);
            req.add('groups', groups);
            req.add("message", msg);
            req.add("chatId", config.chatId);
            req.invoke(callback, c.isGet, c.sync);
            return req;
        };

        c.sendUserMessage = function(msg, userId) {
            if (!isOk())
                return 0;
            var req = me.createRequest('sendUserMessage', c.isXML, c.version, UHTTP.defaultConfig);
            setInternal(req);
            req.add('userId', userId);
            req.add("message", msg);
            req.add("chatId", config.chatId);
            req.invoke(callback, c.isGet, c.sync);
            return req;
        };

        c.enter = function(userId, groups) {
            var req = me.createRequest('enter', c.isXML, c.version, UHTTP.defaultConfig);
            setInternal(req);
            req.add('groups', groups);
            req.add("userId", userId);
            req.invoke(callback, c.isGet, c.sync);
            return req;
        };

        function isOk() {
            if (!c.chatting())
                config.chatId = UHTTP.defaultConfig.chatId;
            if (!c.chatting()) {
                alert("No chat id available!");
                return 0;
            }
            return 1;
        }

        c.listen = function() {
            if (listening || !isOk())
                return 0;
            var req = me.createRequest('listen', c.isXML, c.version, UHTTP.defaultConfig);
            setInternal(req);
            req.add("chatId", config.chatId);
            req.invoke(callback, c.isGet, false);
            listening = true;
            return req;
        };

        c.subscribe = function(delayTime) {
            if (typeof delayTime != 'number' || delayTime < 0)
                delayTime = 0;
            setTimeout(c.listen, delayTime);
        };

        function setInternal(req) {
            c.crossSite = req.isCrossSite() ? 1 : 0;
            if (c.crossSite) {
                c.sync = 0;
                c.isGet = 1;
            }
        }

        c.exit = function() {
            listening = false;
            var req = me.createRequest('exit', c.isXML, c.version, UHTTP.defaultConfig);
            setInternal(req);
            if (typeof (config.chatId) == 'string' && config.chatId.length == 38) {
                req.add("chatId", config.chatId);
                config.chatId = '';
                req.invoke(callback, c.isGet, c.sync);
            }
            config.chatId = '';
            return req;
        };
        addEvent(window, 'unload', c.exit);
        return c;
    } ();
    return me;
} ();

var UChat = UHTTP.Chat;