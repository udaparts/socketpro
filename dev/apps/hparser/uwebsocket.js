SPA.ClientSide = function (host) {
    var win = window;
    var me = {};
    var aWs = [];
    var callIndex = 0;
    var v = 1.2;
    var exSocketClosed = 'Web socket closed!';
    var exRequestName = 'Valid request name required';
    var func = 'function';
    var spa = SPA;
    var str = 'string';
    var obj = 'object';
    var invalidGroups = 'Invalid chat groups';
    var mapErr = [];
    var udoBatch = 'udoBatch';
    var uenter = 'uenter';
    var up = 'uping';
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

    function cleanAll(e) {
        /*       var n, len = aWs.length;
        for (n = 0; n < len; ++n) {
        var s = aWs[n];
        s.script().close();
        }*/
        aWs = [];
    }

    function remove(s) {
        var n, len = aWs.length;
        for (n = 0; n < len; ++n) {
            if (aWs[n] === s) {
                aWs.splice(n, 1);
                spa.log('WebSocket closed');
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

    me.connect = function (userId, pwd, onOpen, onClose, channel) {
        if (typeof (channel) != str)
            channel = '';
        var id = '', timeout = 30000; //30 seconds for timeout
        var pt = 300000;
        var listening = 0;
        var reqBatched = { b: 0, rb: [] };
        channel = channel.replace(/^\s+|\s+$/g, '');
        var prevTime = new Date();
        var pos = channel.indexOf('/');
        if (!pos)
            channel = channel.substr(1);
        var verified, ms, ws = new WebSocket(host + channel, 'uhttprequest'), r = {}, cIndex = 0, onResult = [], queuedRequests = [];
        spa.log('WebSocket connecting to ' + host + channel + ' ......');
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

        function onErrorFound(errMsg) {
            spa.log('WebSocket error = ' + errMsg);
            if (ws && typeof (r.onError) == func) {
                r.onError(errMsg);
            }
        }

        ws.onopen = function (e) {
            r.sendRequest('uswitchTo', userId, pwd, function (res) {
                verified = 1;
                spa.log('Connected to ' + host + channel);
                if (!res.rc) {
                    id = res.id;
                    if (res.pt > timeout)
                        pt = res.pt;
                    else
                        pt = timeout;
                    if (onOpen)
                        onOpen();
                }
                else {
                    id = '';
                    onErrorFound(mapErr[res.rc]);
                }
            });
        };

        ws.onclose = function (e) {
            verified = 0;
            if (onClose) {
                onClose();
                onClose = null;
            }
            remove(r);
            ws = null;
            queuedRequests = [];
            onResult = [];
        };

        ws.onerror = function (e) {
            onErrorFound(e.toString());
        };

        ws.onmessage = function (e) {
            if (ws == null)
                return;
            prevTime = new Date();
            ms = e.data;
            ms = JSON.parse(ms);
            spa.log('Result <<' + e.data + '>> returned for call index = ' + ms.i);
            if (ms.i) {
                if (queuedRequests.length) {
                    var s, req = queuedRequests[0];
                    queuedRequests.splice(0, 1);
                    s = JSON.stringify(req);
                    ws.send(s);
                    spa.log(request + '<<' + s + '>> dequeued and sent with call index = ' + req.i);
                }
                var p = removeCallback(ms.i);
                if (p) {
                    if (!r.push.chatting() && p.m == uenter) {
                        listening = 1;
                        spa.log('Message from server returned for call index = ' + ms.i);
                    }
                    if (p.cb)
                        p.cb(ms)
                }
            }
            else if (r.push.onMessage) {
                r.push.onMessage(ms);
            }
        };

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

        r.close = function () {
            verified = 0;
            if (ws) {
                ws.close();
                ws = null;
                if (onClose) {
                    onClose();
                    onClose = null;
                }
                remove(r);
            }
            queuedRequests = [];
            onResult = [];
            pt = 300000;
            listening = 0;
            reqBatched.b = 0;
        };

        r.isOpen = function () {
            return (!!ws && ws.readyState == 1 && !!verified && id.length > 17);
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
            if (!ws)
                throw exSocketClosed;
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
                prevTime = new Date;
                return cIndex;
            }

            if ((onResult.length - reqBatched.rb.length) < 10) {
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
                listening = 0;
                return r.sendRequest('uexit', cb);
            };

            return chat;
        } ();

        aWs.push(r);
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
        }, 250);
        return r;
    };
    return me;
}