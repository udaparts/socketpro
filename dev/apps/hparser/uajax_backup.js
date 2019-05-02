SPA.ClientSide = function (host) {
    var me = {};
    var aWs = [];
    var callIndex = 0;
    var version = 1.2;
    var exSocketClosed = 'Web socket closed!';
    var exRequestName = 'Valid request name required';

    function cleanAll(e) {
        var n, len = aWs.length;
        for (n = 0; n < len; ++n) {
            var s = aWs[n];
        }
        aWs = [];
    }

    function remove(s) {
        var n, len = aWs.length;
        for (n = 0; n < len; ++n) {
            if (aWs[n] === s) {
                aWs.splice(n, 1);
                return;
            }
        }
    }

    var addEvent = function (obj, type, fn, useCapture) {
        if (obj.addEventListener)
            obj.addEventListener(type, fn, useCapture);
        else if (obj.attachEvent) {
            obj["e" + type + fn] = fn;
            obj[type + fn] = function () { obj["e" + type + fn](window.event); };
            obj.attachEvent("on" + type, obj[type + fn]);
        }
    };

    function createAjaxObject() {
        var xmlHttp;
        if (window.ActiveXObject) {
            try {
                xmlHttp = new ActiveXObject("Msxml2.XMLHTTP");
            }
            catch (e) {
                xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
            }
        }
        else
            xmlHttp = new XMLHttpRequest(); // Firefox, Opera 8.0+, Safari
        return xmlHttp;
    }

    addEvent(window, "unload", cleanAll);

    me.count = function () {
        return aWs.length;
    };

    me.defaultHost = function () {
        return host;
    };

    me.getVersion = function () {
        return version;
    };

    me.connect = function (onOpen, onClose, sub) {
        var wsRequest, wsMessage, r = {}, ms, opened = false, onResult = [];
        function removeCallback(index) {
            var cb, n, len = onResult.length;
            for (n = 0; n < len; ++n) {
                if (onResult[n].callIndex == index) {
                    cb = onResult[n].cb;
                    onResult.splice(n, 1);
                    break;
                }
            }
            return cb;
        }

        function onResultCome() {
            ms = wsRequest.responseText;
            if (!ms)
                ms = {};
            else
                ms = JSON.parse(ms);
            if (ms.callIndex) {
                var cb = removeCallback(ms.callIndex);
                if (cb)
                    cb(ms)
            }
            else if (r.onMessage) {
                r.onMessage(ms);
            }
        }

        function cbRequest(p0, p1) {
            switch (wsRequest.readyState) {
                case 0: //UNSENT
                    if (onClose) {
                        onClose();
                        onClose = null;
                    }
                    break;
                case 1: //OPENED
                    if (onOpen) {
                        opened = true;
                        onOpen();
                        onOpen = null;
                    }
                    break;
                case 2: //HEADERS_RECEIVED
                case 3: //LOADING
                    break;
                case 4: //DONE
                    onResultCome();
                    wsRequest.abort();
                    openAjaxForRequest();
                    break;
                default:
                    throw "Unknown AJAX event";
                    break;
            }
        }

        if (typeof (sub) == 'object' && sub.length)
            sub = sub.toString();

        wsRequest = createAjaxObject();

        function openAjaxForRequest() {
            wsRequest.onreadystatechange = cbRequest;
            wsRequest.open('POST', host, true, sub);
            wsRequest.setRequestHeader('Content-Type', 'application/json; charset=utf-8');
            wsRequest.setRequestHeader('Accept', 'application/json');
        }

        function cbMessage(p0, p1) {
            switch (wsRequest.readyState) {
                case 0: //UNSENT
                    if (onClose) {
                        onClose();
                        onClose = null;
                    }
                    break;
                case 1: //OPENED
                    if (onOpen) {
                        opened = true;
                        onOpen();
                        onOpen = null;
                    }
                    break;
                case 2: //HEADERS_RECEIVED
                case 3: //LOADING
                    break;
                case 4: //DONE
                    onResultCome();
                    wsRequest.abort();
                    openAjaxForRequest();
                    break;
                default:
                    throw "Unknown AJAX event";
                    break;
            }
        }

        function openAjaxForMessage() {
            wsMessage.onreadystatechange = cbMessage;
            wsMessage.open('POST', host, true);
            wsMessage.setRequestHeader('Content-Type', 'application/json; charset=utf-8');
            wsMessage.setRequestHeader('Accept', 'application/json');
        }

        openAjaxForRequest();

        r.close = function () {
            var m;
            if (wsRequest) {
                m = wsRequest;
                wsRequest = null;
                if (m.readyState > 0)
                    m.abort();
            }
            if (wsMessage) {
                m = wsMessage;
                wsMessage = null;
                if (m.readyState > 0)
                    m.abort();
            }
            remove(r);
        };

        r.isOpen = function () {
            return ((wsRequest || wsMessage) && opened);
        };

        r.send = function (data) {
            if (!wsRequest)
                throw exSocketClosed;
            wsRequest.send(data);
        };

        r.wsRequest = function () {
            return wsRequest;
        };

        r.getMsg = function () {
            return ms;
        };

        r.sendRequest = function () {
            if (!r.isOpen())
                throw exSocketClosed;
            ++callIndex;
            var cb, regxMethodName, req, methodName,
            args = Array.prototype.slice.call(arguments),
            len = args.length;
            if (len < 1)
                throw exRequestName;
            methodName = args[0];
            if (!methodName || typeof methodName != 'string')
                throw exRequestName;
            regxMethodName = /^([a-zA-Z_][\w]*)$/;
            if (!regxMethodName.exec(methodName))
                throw exRequestName;
            req = { reqName: methodName, callIndex: callIndex, version: version };
            args.splice(0, 1);
            --len;
            if (len && typeof (args[len - 1]) == 'function') {
                cb = args[len - 1];
                args.splice(len - 1, 1);
                onResult.push({ callIndex: callIndex, cb: cb });
            }
            req.args = args;
            wsRequest.send(JSON.stringify(req));
        };
        aWs.push(r);
        return r;
    };
    return me;
} 
