function parseUrl(s) {
    var a, h, r = /^([^:]*)+(:\/\/)?([^:]*:[^@]*@)?([^\/:\?]*\.[^\/:\?]*)?(:[^\/]*)?(\/[^?#]*)?(\?[^#]*)?(#.*)?$/i;
    a = r.exec(s);
    h = { h: a[4].toLowerCase(), //host
        p: a[1].toLowerCase(), //protocol, http or https
        r: a[6], //relative path
        q: a[7], //query
        n: 0, //port
        u: a[3], //user:password@
        o: a[0] //original url string
    };
    s = a[5];
    if (s)
        h.n = parseInt(s.substr(s.indexOf(':') + 1));
    if (!h.n && h.p == 'https') h.n = 443;
    if (!h.n && h.p == 'http') h.n = 80;
    return h;
}