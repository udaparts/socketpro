SPA.ClientSide = function () {
    var me = {};
    var aWs = [];
    var callIndex = 0;
    var version = 1.1;
    function cleanAll(e) {
        var n, len = aWs.length;
        for (n = 0; n < len; ++n) {
            var s = aWs[n];
            s.ws().close();
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

    addEvent(window, "unload", cleanAll);

    me.count = function () {
        return aWs.length;
    };

    me.connect = function (url, sub) {
        var ws = new WebSocket(url, sub);
        var r = {};
        var ms, onResult = [];

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

        ws.onopen = function (e) {
            if (r.onOpen)
                r.onOpen();
        };

        ws.onclose = function (e) {
            if (r.onClose)
                r.onClose();
            remove(r);
            ws = null;
        };

        ws.onerror = function (e) {
            if (ws && r.onError)
                r.onError(e);
        };

        ws.onmessage = function (e) {
            ms = e.data;
            if (!ms)
                return;
            var obj = JSON.parse(ms);
            if (obj.callIndex) {
                var cb = removeCallback(obj.callIndex);
                if (cb)
                    cb(obj)
            }
            else if (r.onMessage) {
                r.onMessage(ms);
            }
        };

        r.getMsg = function () {
            return ms;
        };

        r.close = function () {
            if (ws) {
                var m = ws;
                ws = null;
                if (m.readyState < 2)
                    m.close();
                remove(r);
            }
        };

        r.isOpen = function () {
            return (!!ws && ws.readyState == 1);
        };

        r.send = function (data) {
            if (!ws)
                throw "Web socket closed!";
            ws.send(data);
        };

        r.ws = function () {
            return ws;
        };

        r.sendRequest = function () {
            if (!ws)
                throw "Web socket closed!";
            ++callIndex;
            var args = Array.prototype.slice.call(arguments);
            var req = { reqName: args[0], callIndex: callIndex, version: version };
            args.splice(0, 1);
            var len = args.length;
            if (len && typeof (args[len - 1]) == 'function') {
                var cb = args[len - 1];
                args.splice(len - 1, 1);
                onResult.push({ callIndex: callIndex, cb: cb });
            }
            req.args = args;
            ws.send(JSON.stringify(req));
        };
        aWs.push(r);
        return r;
    };
    return me;
} ();
var UHTTP = SPA.ClientSide;