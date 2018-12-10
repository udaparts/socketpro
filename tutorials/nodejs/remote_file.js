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
if (!file.Download('spfile.test', 'jvm.lib', (errMsg, errCode) => {
        console.log('errMsg=' + errMsg + ', errCode=' + errCode);
    }, (pos, fileSize) => {
        console.log('Downloading: ' + 100 * pos / fileSize + '%');
    }, (canceled) => {
        console.log('File download stopped because ' + canceled ? 'downloading is canceled' : 'session is closed');
    })) {
    console.log(file.Socket.Error);
    return;
}
console.log('Downloading jvm.lib');

if (!file.Upload('spfile.test', 'jvm_copy.lib', (errMsg, res) => {
        console.log('errMsg=' + errMsg + ', errCode=' + res);
    }, (pos, fileSize) => {
        console.log('Uploading: ' + 100 * pos / fileSize + '%');
    }, (canceled) => {
        console.log('File upload stopped because ' + canceled ? 'uploading is canceled' : 'session is closed');
    })) {
    console.log(file.Socket.Error);
    return;
}
console.log('Uploading jvm_copy.lib');

async function echoFile(file, localFile, remoteFile, progress) {
    try {
        //use download instead of Download for Promise
        var result = await file.download(localFile, remoteFile, progress);
        console.log(result);

        remoteFile += '.copy';
        //use upload instead of Upload for Promise
        result = await file.upload(localFile, remoteFile, progress);
        console.log(result);
    } catch (err) {
        console.log(err);
    }
}

echoFile(file, 'spfile.test', 'jvm.lib', (pos, fsize) => {
    console.log({
        pos: pos,
        fsize: fsize
    });
});
