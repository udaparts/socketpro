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

    me.host = function () {
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
        var ms,
        verified = 0,
        ws = null,
        r = {},
        cIndex = 0,
        onResult = [],
        queuedRequests = [],
        db = document.body,
        prevTime = new Date(),
        timeout = 30000,
        id = '',
        listening,
        pt = 300000;
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
            db.appendChild(s);
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

        function stopListening() {
            if (listening) {
                abort(listening);
                listening = null;
            }
        }

        function subscribe() {
            listening = openJsForRequest(listening)
            cIndex = ++callIndex;
            var req = { n: 'ulisten', i: cIndex, v: v, id: id, a: [] };
            send(listening, JSON.stringify(req));
            return cIndex;
        }

        function startListening() {
            if (!listening) {
                subscribe();
            }
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
            ms = obj;
            prevTime = new Date();
            var cb, dq = removeCallback(ms.i);
            spa.log('Result <<' + JSON.stringify(ms) + '>> returned for call index = ' + ms.i);

            if (!dq) {

                //ulistening .... 

                spa.log('Message from server returned for call index = ' + ms.i);
                return;
            }

            if (queuedRequests.length) {
                var s, req = queuedRequests[0];
                if (dq.m != udoBatch && req.i > dq.i + 1)
                    return;
                queuedRequests.splice(0, 1);
                ws = openJsForRequest(ws);
                s = JSON.stringify(req);
                send(ws, s);
                spa.log(request + '<<' + s + '>> dequeued and sent with call index = ' + req.i);
            }
            else if (dq.m == 'uclose') {
                abort(ws);
            }

            if (!r.push.chatting() && dq.m == uenter)
                startListening();

            cb = dq.cb;
            if (!verified) {
                verified = 1;
                spa.log('Javascript connected to ' + host + channel);
                if (!ms.rc) {
                    id = ms.id;
                    if (ms.pt > timeout)
                        pt = ms.pt;
                    else
                        pt = timeout;
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
        };

        r.close = function () {
            if (ws) {
                r.sendRequest('uclose');
                stopListening();
                if (onClose) {
                    onClose();
                    onClose = null;
                }
                spa.log('Javascript closed');
                remove(r);
                //abort(ws);
                //ws = null;
            }
            queuedRequests = [];
            onResult = [];
            verified = 0;
            stopListening();
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

        r.abort = function () {
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

            onResult.push(rc);

            req.a = args;

            if (reqBatched.b && methodName != up) {
                reqBatched.rb.push(req);
                spa.log(request + methodName + ' batched with call index = ' + cIndex);
                prevTime = new Date();
                return cIndex;
            }

            if (onResult.length - reqBatched.rb.length < 2) {
                req = JSON.stringify(req);
                prevTime = new Date();
                send(ws, req);
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
            }

            chat.ws = function () {
                return r;
            };

            chat.chatting = function () {
                return (r.isOpen() && !!listening);
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
                stopListening();
                return r.sendRequest('uexit', cb);
            };

            return chat;
        } ();

        aWs.push(r);
        ws = openJsForRequest();
        cIndex = ++callIndex;
        onResult.push({ m: uswitchTo, i: cIndex });
        send(ws, JSON.stringify({ n: uswitchTo, i: cIndex, v: v, id: id, a: [userId, pwd] }));
        spa.log(request + 'uswitchTo sent with call index = ' + cIndex);
        setInterval(function (e) {
            var now = new Date(),
            diff = now.getTime() - prevTime.getTime();
            if (!r.count()) {
                if (r.isOpen() && (diff + 2000) > pt) {
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
            }
        }, 400);
        return r;
    };
    return me;
}