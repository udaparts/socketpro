SPA.ClientSide = function (host) {
    var win = window;
    var me = {};
    var aWs = [];
    var callIndex = 0;
    var v = 1.2;
    var exSocketClosed = 'AJAX connection closed!';
    var exRequestName = 'Valid request name required';
    var func = 'function';
    var str = 'string';
    var obj = 'object';
    var invalidGroups = 'Invalid chat groups';
    var spa = SPA;
    var mapErr = [];
    var udoBatch = 'udoBatch';
    var uenter = 'uenter';
    var up = 'uping';
    var uswitchTo = 'uswitchTo';
    var request = 'Request ';

    mapErr[0] = 'Ok';
    mapErr[1] = 'Bad Json Request';
    mapErr[2] = 'Bad number of arguments';
    mapErr[3] = 'Bad arguments';
    mapErr[4] = 'Bad argument type';
    mapErr[5] = 'Bad request';
    mapErr[6] = 'Empty request';
    mapErr[7] = 'Not supported';
    mapErr[8] = 'Authentication failed';


    function createAjaxObject() {
        var xmlHttp;
        if (win.XMLHttpRequest)
            xmlHttp = new XMLHttpRequest(); // IE7 or later, Firefox, Opera 8.0+, Safari
        else if (win.ActiveXObject) {
            try {
                xmlHttp = new ActiveXObject("Msxml2.XMLHTTP");
            }
            catch (e) {
                xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
            }
        }
        return xmlHttp;
    }

    function cleanAll(e) {
        var n, len = aWs.length;
        for (n = len - 1; n >= 0; --n) {
            var s = aWs[n];
            if (s && s.isOpen()) {
                s.close();
            }
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

    function dateConvert(key, value) {
        if (typeof value === 'string') {
            var a = /^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})\.(\d{3})Z$/.exec(value);
            if (a) {
                return new Date(Date.UTC(+a[1], +a[2] - 1, +a[3], +a[4], +a[5], +a[6], +a[7]));
            }
        }
        return value;
    }

    var addEvent = function (obj, type, fn, useCapture) {
        if (obj.addEventListener)
            obj.addEventListener(type, fn, useCapture);
        else if (obj.attachEvent) {
            obj["e" + type + fn] = fn;
            obj[type + fn] = function () { obj["e" + type + fn](win.event); };
            obj.attachEvent("on" + type, obj[type + fn]);
        }
    };

    addEvent(win, "unload", cleanAll, false);

    me.count = function () {
        return aWs.length;
    };

    me.host = function (newHost) {
		if (newHost && typeof (newHost) == str)
			host = newHost;
        return host;
    };

    me.version = function () {
        return v;
    };

    me.connect = function (userId, pwd, onOpen, onClose, channel) {
        var ms,
        verified = 0,
        ws = createAjaxObject(),
        r = {},
        cIndex = 0,
        onResult = [],
        queuedRequests = [],
        ok = 1,
        timeout = spa.debug() ? 0x2FFFFFFF : 30000,
        prevTime = new Date(),
        id = '',
        pt = 300000;
        sync = 0;
        var reqBatched = { b: 0, rb: [] };

        if (typeof (channel) != str)
            channel = '';
        channel = channel.replace(/^\s+|\s+$/g, '');
        var pos = channel.indexOf('/');
        if (!pos)
            channel = channel.substr(1);

        spa.log('AJAX connecting to ' + host + channel + ' ......');

        function onErrorFound(errMsg) {
            spa.log('AJAX error = ' + errMsg);
            if (ws && typeof (r.onError) == func) {
                r.onError(errMsg);
            }
        }

        function processResponse(done) {
            prevTime = new Date();
            if (ws.status >= 300 || !ok) {
                onErrorFound(ws.statusText);
                if (ws && (ws.status >= 600 || !ok)) {
                    r.close();
                    reset();
                }
                return;
            }

            var j, p, len, ms = JSON.parse(ws.responseText, dateConvert), rb = [];

            ws.abort();
            if (queuedRequests.length) {
                var s, req = queuedRequests[0];
                queuedRequests.splice(0, 1);
                if (openAjaxForRequest(ws, cbRequest)) {
                    s = JSON.stringify(req);
                    ws.send(JSON.stringify(req));
                    spa.log(request + '<<' + s + '>> dequeued and sent with call index = ' + req.i);
                }
            }
            if (ms.rb)
                rb = ms.rb;
            else
                rb.push(ms);
            len = rb.length;
            for (j = 0; j < len; ++j) {
                ms = rb[j];
                if (ms.rc) {
                    onErrorFound(mapErr[ms.rc]);
                    reset();
                    return;
                }
                if (ms.i) {
                    p = removeCallback(ms.i);
                    if (p) {
                        ms.n = p.m;
                        if (p.m == uswitchTo) {
                            if (!ms.rc) {
                                verified = 1;
                                spa.log('AJAX connected to ' + host + channel);
                                id = ms.id;
                                pt = ms.pt;
                                if (onOpen) {
                                    onOpen();
                                }
                            }
                            else {
                                id = '';
                                onErrorFound(mapErr[ms.rc]);
                            }
                            onOpen = null;
                        }
                        if (p.cb)
                            p.cb(ms)
                    }
                }
                else {
                    if (typeof (r.push.onMessage) == func)
                        r.push.onMessage(ms);
                }
                spa.log('Result <<' + JSON.stringify(ms) + '>> returned');
            }
        }

        function cbRequest() {
            if (!ws)
                return;
            switch (ws.readyState) {
                case 0: //UNSENT
                case 1: //OPENED
                    ok = 0;
                    break;
                case 2: //HEADERS_RECEIVED
                case 3: //LOADING
                    ok = 1;
                    prevTime = new Date();
                    break;
                case 4: //DONE
                    processResponse(1);
                    break;
                default:
                    throw 'Unknown AJAX event';
                    break;
            }
        }

        function openAjaxForRequest(xh, cb) {
            xh.onreadystatechange = cb;
            try {
                xh.open('POST', host + channel, !sync);
            }
            catch (e) {
                spa.log('Fatal exception caught during sending request and connection will be closed');
                onErrorFound(e.toString());
                r.close();
                reset();
                return 0;
            }
            xh.setRequestHeader('uhttprequest', '1');
            xh.setRequestHeader('Content-Type', 'application/json; charset=utf-8');
            return 1;
        }

        function removeCallback(index) {
            var cb, n, len = onResult.length;
            for (n = 0; n < len; ++n) {
                if (onResult[n].i == index) {
                    cb = onResult[n];
                    onResult.splice(n, 1);
                    break;
                }
            }
            return cb;
        }

        r.isBatching = function () {
            return !!reqBatched.b;
        };

        r.getBatchSize = function () {
            //15 -- empty size
            return JSON.stringify(reqBatched).length - 15;
        };

        r.beginBatching = function () {
            if (!ws)
                throw exSocketClosed;
            reqBatched.b = 1;
        };

        r.commit = function (serverBatching) {
            if (!ws)
                throw exSocketClosed;
            reqBatched.b = 0;
            if (reqBatched.rb.length) {
                r.sendRequest(udoBatch, !!serverBatching, reqBatched.rb);
                reqBatched.rb = [];
            }
            return cIndex;
        };

        r.rollback = function () {
            if (!ws)
                throw exSocketClosed;
            var p, n, size = reqBatched.rb.length;
            for (n = 0; n < size; ++n) {
                p = removeCallback(reqBatched.rb[n].i);
            }
            reqBatched.rb = [];
            reqBatched.b = 0;
        };

        r.msg = function () {
            return ms;
        };

        r.timeout = function (newTime) {
            if (typeof (newTime) == 'number' && newTime >= 1000)
                timeout = newTime;
            return timeout;
        };

        r.pingTime = function (newTime) {
            if (typeof (newTime) == 'number' && newTime >= 1000 && newTime <= 59000)
                pt = newTime;
            return pt;
        };

        r.channel = function () {
            return channel;
        };

        function reset() {
            if (onClose) {
                onClose();
                onClose = null;
            }
            ws = null;
            spa.log('AJAX closed');
            remove(r);
            verified = 0;
            reqBatched.b = 0;
            queuedRequests = [];
            onResult = [];
        }

        r.close = function () {
            queuedRequests = [];
            onResult = [];
            timeout = 1000;
            if (r.isOpen()) {
                sync = 0;
                r.sendRequest('uclose', function (res) {
                    reset();
                });
            }
            id = '';
        };

        r.isOpen = function () {
            return (!!ws && !!verified && id.length > 17);
        };

        r.script = function () {
            return ws;
        };

        r.userId = function () {
            return userId;
        };

        r.cancel = function () {
            if (reqBatched.b)
                throw 'Bad operation -- batching requests';
            var canceled = { aborted: queuedRequests.length, ignored: onResult.length - queuedRequests.length };
            onResult = [];
            queuedRequests = [];
            return canceled;
        };

        r.count = function () {
            return (onResult.length + queuedRequests.length);
        };

        r.callIndex = function () { return cIndex; };

        r.sendRequest = function () {
            if (!r.isOpen())
                throw exSocketClosed;
            if (!r.count() && !openAjaxForRequest(ws, cbRequest))
                return 0;
            cIndex = ++callIndex;
            var cb, regxMethodName, req, methodName,
            args = Array.prototype.slice.call(arguments),
            len = args.length;
            if (len < 1)
                throw exRequestName;
            methodName = args[0];
            if (!methodName || typeof (methodName) != str)
                throw exRequestName;
            regxMethodName = /^([a-zA-Z_][\w]*)$/;
            if (!regxMethodName.exec(methodName))
                throw exRequestName;
            req = { n: methodName, i: callIndex, v: v, id: id };
            args.splice(0, 1);
            --len;
            var rc = { m: methodName, i: callIndex };
            if (len && typeof (args[len - 1]) == func) {
                cb = args[len - 1];
                args.splice(len - 1, 1);
                rc.cb = cb;
            }

            if (methodName != udoBatch) {
                onResult.push(rc);
            }
            req.a = args;

            if (reqBatched.b && methodName != up) {
                reqBatched.rb.push(req);
                spa.log(request + methodName + ' batched with call index = ' + cIndex);
                prevTime = new Date();
                return cIndex;
            }

            if ((onResult.length - reqBatched.rb.length) < 2) {
                prevTime = new Date();
                req = JSON.stringify(req);
                ws.send(req);
                spa.log(request + '<<' + req + '>> sent with call index = ' + cIndex);
            }
            else {
                queuedRequests.push(req);
                spa.log(request + methodName + ' queued with call index = ' + cIndex);
            }
            return cIndex;
        };

        r.push = function () {
            var chat = {};

            function checkGroups(groups) {
                if (typeof (groups) != obj || !groups.sort)
                    throw invalidGroups;
                for (var n = 0; n < groups.length; ++n) {
                    var d = parseInt(groups[n]);
                    if (!d) {
                        groups.splice(n, 1);
                        --n;
                    }
                    else
                        groups[n] = d;
                }
            }

            chat.ws = function () {
                return r;
            };

            chat.speak = function (msg, groups, cb) {
                checkGroups(groups);
                return r.sendRequest('uspeak', msg, groups, cb);
            };

            chat.sendUserMessage = function (msg, userId, cb) {
                return r.sendRequest('usendUserMessage', userId, msg, cb);
            };

            chat.enter = function (groups, cb) {
                checkGroups(groups);
                return r.sendRequest(uenter, groups, userId, cb);
            };

            chat.exit = function (cb) {
                return r.sendRequest('uexit', cb);
            };

            return chat;
        } ();

        aWs.push(r);
        if (!openAjaxForRequest(ws, cbRequest))
            return 0;
        cIndex = ++callIndex;
        onResult.push({ m: uswitchTo, i: cIndex });
        ws.send(JSON.stringify({ n: uswitchTo, i: cIndex, v: v, id: id, a: [userId, pwd] }));
        spa.log(request + 'uswitchTo sent with call index = ' + cIndex);

        setInterval(function (e) {
            var now = new Date(),
            diff = now.getTime() - prevTime.getTime();
            if (!r.count()) {
                if (r.isOpen() && diff > pt) {
                    prevTime = now;
                    var b = reqBatched.b;
                    reqBatched.b = 0;
                    r.sendRequest(up, function (res) {

                    });
                    reqBatched.b = b;
                }
            }
            else if (diff > timeout) {
                onErrorFound('Communication timed out');
                r.close();
                reset();
            }
        }, 250);

        return r;
    };
    return me;
}
