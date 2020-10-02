'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');

const sidPi = SPA.SID.sidReserved + 5;
const idComputePi = SPA.BaseID.idReservedTwo + 1;

var cs = SPA.CS; //CS == Client side

var p = cs.newPool(sidPi);
global.p = p;

p.QueueName = 'lbqname';

if (!p.Start(cs.newCC('localhost', 20901, 'lb_client', 'pwd_lb_client'), 1)) {
    console.log(p.Error);
    return;
}
var pi = p.SeekByQueue();

(async () => {
    const nDivision = 1000, nNum = 10000000;
    var dStep = 1.0 / nNum / nDivision, dPi = 0.0;
    var sb = SPA.newBuffer();
    var vP = [];
    for (var n = 0; n < nDivision; ++n) {
        var dStart = 1.0 * n / nDivision;
        vP.push(pi.sendRequest(idComputePi, sb.SaveDouble(dStart).SaveDouble(dStep).SaveInt(nNum)));
    }
    for (var n = 0; n < vP.length; ++n) {
        var buff = await vP[n];
        var res = buff.LoadDouble(), dStart = buff.LoadDouble();
        dPi += res;
        console.log('dStart: ' + dStart + ', res: ' + res + ', pi: ' + dPi);
    }
    console.log('Final pi: ' + dPi);
})();
/*
(async () => {
    var index = 0;
    const nDivision = 1000, nNum = 10000000;
    var dStep = 1.0 / nNum / nDivision, dPi = 0.0;
    var sb = SPA.newBuffer();
    for (var n = 0; n < nDivision; ++n) {
        var dStart = 1.0 * n / nDivision;
        pi.SendRequest(idComputePi, sb.SaveDouble(dStart).SaveDouble(dStep).SaveInt(nNum), (buff, reqId) => {
            var res = buff.LoadDouble();
            dPi += res;
            console.log('dStart: ' + buff.LoadDouble() + ', res: ' + res + ', pi: ' + dPi);
            ++index;
            if (index == nDivision) console.log('Final pi: ' + dPi);
        });
    }
})();
*/
