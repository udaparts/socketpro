//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA=require('nja.js');

//create a memory buffer or queue for data serialization and de-serialization
var buf = SPA.newBuffer();

var now = new Date();
console.log(now);
var res = buf.SaveObject(now).LoadObject();
console.log(res);

var str= 'Hello SocketPro!';
console.log(str);
var res = buf.SaveObject(str).LoadObject();
console.log(res);

var arrStr = ['Hello', ' my world!'];
console.log(arrStr);
res = buf.SaveObject(arrStr).LoadObject();
console.log(res);

var arrBool = [true, false];
console.log(arrBool);
res = buf.SaveObject(arrBool).LoadObject();
console.log(res);

var arrDate = [new Date(), new Date()];
console.log(arrDate);
res = buf.SaveObject(arrDate).LoadObject();
console.log(res);

var buffer = new ArrayBuffer(16);
var int32View = new Int32Array(buffer);
for (var i = 0; i < int32View.length; i++) {
  int32View[i] = i * 2;
}
console.log(int32View);
res = buf.SaveObject(int32View).LoadObject();
console.log(res);

var f32View = new Float32Array(buffer);
for (var i = 0; i < f32View.length; i++) {
  f32View[i] = i * 2.42;
}
console.log(f32View);
res = buf.SaveObject(f32View).LoadObject();
console.log(res);

buffer = new ArrayBuffer(32);
var f64View = new Float64Array(buffer);
for (var i = 0; i < f64View.length; i++) {
  f64View[i] = i * 3.11;
}
console.log(f64View);
res = buf.SaveObject(f64View).LoadObject();
console.log(res);

console.log(buf.SaveString('MyTestForAllBinaryData').PopBytes());
console.log('Data size = ' + buf.getSize());
