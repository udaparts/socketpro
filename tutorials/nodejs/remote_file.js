"use strict";

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

//streaming files downloading and uploading
if (!file.Download('spfile.test', 'jvm.lib', //1
    (errMsg, errCode) => {//a
        console.log('errMsg=' + errMsg + ', errCode=' + errCode);
    }, (pos, fileSize) => { //b
        console.log('Downloading: ' + Number(100 * pos / fileSize).toFixed(2) + '%');
    }, (canceled) => {//c
        console.log('File download stopped because ' + canceled ? 'downloading is canceled' : 'session is closed');
    })) {
    console.log(file.Socket.Error);
    return;
}
console.log('Downloading jvm.lib');

if (!file.Upload('spfile.test', 'jvm_copy.lib', //2
    (errMsg, res) => {//d
        console.log('errMsg=' + errMsg + ', errCode=' + res);
    }, (pos, fileSize) => {//e
        console.log('Uploading: ' + Number(100 * pos / fileSize).toFixed(2) + '%');
    }, (canceled) => {//f
        console.log('File upload stopped because ' + canceled ? 'uploading is canceled' : 'session is closed');
    })) {
    console.log(file.Socket.Error);
    return;
}
console.log('Uploading jvm_copy.lib');

async function echoFile(file, localFile, remoteFile, progress) {//3
    try {
        console.log('++++ use download instead of Download for Promise ++++');
        console.log(await file.download(localFile, remoteFile, progress)); //4

        remoteFile += '.copy';
        console.log('++++ use upload instead of Upload for Promise ++++');
        console.log(await file.upload(localFile, remoteFile, progress)); //5
    } catch (err) {
        console.log(err);
    }
}

echoFile(file, 'spfile.test', 'jvm.lib', //6
    (pos, fsize) => {//7
        console.log({
            pos: pos,
            fsize: fsize
        });
});
