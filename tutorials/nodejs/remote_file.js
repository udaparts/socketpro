'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidFile);
global.p = p;

//create a connection context
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var file = p.Seek(); //seek an async file handler

(async () => {
    try {
        var p0 = file.download('spfile.test', 'jvm.lib', (pos, fsize) => {
            console.log({ pos: pos, fsize: fsize });
        });
        var p1 = file.upload('spfile.test', 'jvm_copy.lib');
        var my_array = await Promise.all([p0, p1]);
        console.log(my_array);
    } catch (ex) {
        console.log(ex);
    }
})();
