SPA.ClientSide = function (host) {
    var win = window;
    var me = {};
    var aWs = [];
    var callIndex = 0;
    var v = 1.2;
    var exSocketClosed = 'Javascript connection closed!';
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

    function createJsScript() {
        var script = document.createElement('script');
        script.setAttribute("type", "text/javascript");
        script.setAttribute('charset', 'utf-8');
        return script;
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

    me.jsCallback = function (res) {
        if (!res)
            return;
        var n, len = aWs.length;
        for (n = 0; n < len; ++n) {
            if (aWs[n].jsCallback(res))
                return;
        }
    }

    me.connect = function (userId, pwd, onOpen, onClose, channel) {
        var ms, sent = 0,
        verified = 0,
        ws = null,
        r = {},
        cIndex = 0,
        onResult = [],
        queuedRequests = [],
        db = document.body,
        prevTime = new Date(),
        timeout = spa.debug() ? 0x2FFFFFFF : 30000,
        id = '',
        pt = 4000;
        var reqBatched = { b: 0, rb: [] };

        if (typeof (channel) != str)
            channel = '';
        channel = channel.replace(/^\s+|\s+$/g, '');
        var pos = channel.indexOf('/');
        if (!pos)
            channel = channel.substr(1);

        spa.log('Javascript connecting to ' + host + channel + ' ......');

        function abort(s) {
            if (s) {
                try {
                    db.removeChild(s);
                }
                catch (ex) {
                }
            }
        }

        function onErrorFound(errMsg) {
            spa.log('JavaScript error = ' + errMsg);
            if (r && typeof (r.onError) == func) {
                r.onError(errMsg);
            }
        }

        function openJsForRequest(s) {
            if (s)
                abort(s);
            return createJsScript();
        }

        function send(s, data) {
            data = encodeURIComponent(data);
            s.setAttribute('src', host + channel + '?UJS_DATA=' + data);
            s.setAttribute('charset', 'utf-8');
            db.appendChild(s);
        }

        function reset() {
            if (onClose) {
                onClose();
                onClose = null;
            }
            spa.log('Javascript closed');
            remove(r);
            queuedRequests = [];
            onResult = [];
            verified = 0;
            abort(ws);
            ws = null;
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

        r.channel = function () {
            return channel;
        };

        r.jsCallback = function (obj) {
            var req, s, cb, dq, n, size, rb = [];
            prevTime = new Date();
            if (obj.rb)
                rb = obj.rb;
            else
                rb.push(obj);
            size = rb.length;

            //spa.log('JavaScript state = ' + ws.readyState + ', sent = ' + sent);

            for (n = 0; n < size; ++n) {
                ms = JSON.parse(JSON.stringify(rb[n]), dateConvert);
                if (ms.rc) {
                    onErrorFound(mapErr[ms.rc]);
                    reset();
                    return;
                }

                if (ms.i) {
                    --sent;
                    dq = removeCallback(ms.i);
                    ms.n = dq.m;
                    spa.log('Result <<' + JSON.stringify(ms) + '>> returned for call index = ' + ms.i);
                    if (dq)
                        cb = dq.cb;
                    else
                        cb = null;
                    if (!verified) {
                        verified = 1;
                        spa.log('Javascript connected to ' + host + channel);
                        if (!ms.rc) {
                            id = ms.id;
                            pt = ms.pt;
                            if (onOpen)
                                onOpen();
                        }
                        else {
                            id = '';
                            onErrorFound(mapErr[ms.rc]);
                        }
                        onOpen = null;
                    }
                    if (cb)
                        cb(ms);
                    if (queuedRequests.length && !sent) {
                        req = queuedRequests[0];
                        if (req.n == udoBatch)
                            sent = req.a[1].length;
                        else
                            sent = 1;
                        queuedRequests.splice(0, 1);
                        ws = openJsForRequest(ws);
                        s = JSON.stringify(req);
                        spa.log(request + '<<' + s + '>> dequeued and sent with call index = ' + req.i);
                        send(ws, s);
                    }
                }
                else {
                    spa.log('Result <<' + JSON.stringify(ms) + '>> returned');
                    if (typeof (r.push.onMessage) == func)
                        r.push.onMessage(ms);
                }
            }
        };

        r.close = function () {
            timeout = 1000;
            if (r.isOpen()) {
                r.sendRequest('uclose', function (res) {
                    reset();
                });
                id = '';
            }
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

        r.sendRequest = function () {
            if (!r.isOpen())
                throw exSocketClosed;
            if (!r.count())
                ws = openJsForRequest(ws);
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

            if (onResult.length - reqBatched.rb.length < 2) {
                if (methodName != udoBatch)
                    sent = 1;
                else
                    sent = args[1].length;
                req = JSON.stringify(req);
                prevTime = new Date();
                spa.log(request + '<<' + req + '>> sent with call index = ' + cIndex);
                send(ws, req);
            }
            else {
                spa.log(request + methodName + ' queued with call index = ' + cIndex);
                queuedRequests.push(req);
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
        ws = openJsForRequest();
        cIndex = ++callIndex;
        onResult.push({ m: uswitchTo, i: cIndex });
        sent = 1;
        send(ws, JSON.stringify({ n: uswitchTo, i: cIndex, v: v, id: id, a: [userId, pwd] }));
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
        }, 400);
        return r;
    };
    return me;
}