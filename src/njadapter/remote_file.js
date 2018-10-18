//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA=require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a global socket pool object
p=cs.newPool(SPA.SID.sidFile);

//create a connection context
var cc = cs.newCC('localhost',20901,'root','Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc,1)) {
	console.log(p.Error);
	return;
}
var file = p.Seek(); //seek an async file handler

//streaming files downloading and uploading
if (!file.Download('spfile1.test', 'jvm.lib', (errMsg, errCode)=>{
		console.log('errMsg=' + errMsg + ', errCode=' + errCode);
	}, (pos, fileSize)=>{
		//console.log('Downloading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File download stopped because ' + canceled ? 'downloading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Downloading jvm.lib');

if (!file.Download('spfile2.test', 'libboost_wave-vc100-mt-sgd-1_60.lib', (errMsg, errCode)=>{
		console.log('errMsg=' + errMsg + ', errCode=' + errCode);
	}, (pos, fileSize)=>{
		//console.log('Downloading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File download stopped because ' + canceled ? 'downloading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Downloading libboost_wave-vc100-mt-sgd-1_60.lib');

if (!file.Download('spfile3.test', 'libboost_coroutine-vc100-mt-s-1_60.lib', (errMsg, errCode)=>{
		console.log('errMsg=' + errMsg + ', errCode=' + errCode);
	}, (pos, fileSize)=>{
		//console.log('Downloading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File download stopped because ' + canceled ? 'downloading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Downloading libboost_coroutine-vc100-mt-s-1_60.lib');

if (!file.Download('spfile4.test', 'libboost_serialization-vc100-mt-s-1_60.lib', (errMsg, errCode)=>{
		console.log('errMsg=' + errMsg + ', errCode=' + errCode);
	}, (pos, fileSize)=>{
		//console.log('Downloading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File download stopped because ' + canceled ? 'downloading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Downloading libboost_serialization-vc100-mt-s-1_60.lib');

if (!file.Download('spfile5.test', 'libboost_math_tr1f-vc100-mt-sgd-1_60.lib', (errMsg, errCode)=>{
		console.log('errMsg=' + errMsg + ', errCode=' + errCode);
	}, (pos, fileSize)=>{
		//console.log('Downloading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File download stopped because ' + canceled ? 'downloading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Downloading libboost_math_tr1f-vc100-mt-sgd-1_60.lib');

if (!file.Upload('spfile1.test', 'jvm_copy.lib', (errMsg, res) => {
		console.log('errMsg=' + errMsg + ', errCode=' + res);
	}, (pos, fileSize)=>{
		//console.log('Uploading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File upload stopped because ' + canceled ? 'uploading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Uploading jvm_copy.lib');

if (!file.Upload('spfile2.test', 'libboost_wave-vc100-mt-sgd-1_60_copy.lib', (errMsg, res) => {
		console.log('errMsg=' + errMsg + ', errCode=' + res);
	}, (pos, fileSize)=>{
		//console.log('Uploading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File upload stopped because ' + canceled ? 'uploading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Uploading libboost_wave-vc100-mt-sgd-1_60_copy.lib');

if (!file.Upload('spfile3.test', 'libboost_coroutine-vc100-mt-s-1_60_copy.lib', (errMsg, res) => {
		console.log('errMsg=' + errMsg + ', errCode=' + res);
	}, (pos, fileSize)=>{
		//console.log('Uploading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File upload stopped because ' + canceled ? 'uploading is canceled' : 'session is closed');
	})) {
	console.log(file.Socket.Error);
	return;
}
console.log('Uploading libboost_coroutine-vc100-mt-s-1_60_copy.lib');

if (!file.Upload('spfile4.test', 'libboost_serialization-vc100-mt-s-1_60_copy.lib', (errMsg, res) => {
		console.log('errMsg=' + errMsg + ', errCode=' + res);
	}, (pos, fileSize)=>{
		//console.log('Uploading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File upload stopped because ' + canceled ? 'uploading is canceled' : 'session is closed');
	})) {
		console.log(file.Socket.Error);
		return;
}
console.log('Uploading libboost_serialization-vc100-mt-s-1_60_copy.lib');

if (!file.Upload('spfile5.test', 'libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib', (errMsg, res) => {
		console.log('errMsg=' + errMsg + ', errCode=' + res);
	}, (pos, fileSize)=>{
		//console.log('Uploading: ' + 100 * pos / fileSize + '%');
	}, (canceled)=>{
		console.log('File upload stopped because ' + canceled ? 'uploading is canceled' : 'session is closed');
	})) {
		console.log(file.Socket.Error);
		return;
}
console.log('Uploading libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib');
