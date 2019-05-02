
if(!this.UJSON)this.UJSON={};(function(){function f(n){return n<10?'0'+n:n;}
if(typeof Date.prototype.toJSON!=='function'){Date.prototype.toJSON=function(key){return isFinite(this.valueOf())?this.getUTCFullYear()+'-'+
f(this.getUTCMonth()+1)+'-'+
f(this.getUTCDate())+'T'+
f(this.getUTCHours())+':'+
f(this.getUTCMinutes())+':'+
f(this.getUTCSeconds())+'Z':null;};String.prototype.toJSON=Number.prototype.toJSON=Boolean.prototype.toJSON=function(key){return this.valueOf();};}
var cx=/[\u0000\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,escapable=/[\\\"\x00-\x1f\x7f-\x9f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,gap,indent,meta={'\b':'\\b','\t':'\\t','\n':'\\n','\f':'\\f','\r':'\\r','"':'\\"','\\':'\\\\'},rep;function quote(string){escapable.lastIndex=0;return escapable.test(string)?'"'+string.replace(escapable,function(a){var c=meta[a];return typeof c==='string'?c:'\\u'+('0000'+a.charCodeAt(0).toString(16)).slice(-4);})+'"':'"'+string+'"';}
function str(key,holder){var i,k,v,length,mind=gap,partial,value=holder[key];if(value&&typeof value==='object'&&typeof value.toJSON==='function'){value=value.toJSON(key);}
if(typeof rep==='function'){value=rep.call(holder,key,value);}
switch(typeof value){case'string':return quote(value);case'number':return isFinite(value)?String(value):'null';case'boolean':case'null':return String(value);case'object':if(!value){return'null';}
gap+=indent;partial=[];if(Object.prototype.toString.apply(value)==='[object Array]'){length=value.length;for(i=0;i<length;i+=1){partial[i]=str(i,value)||'null';}
v=partial.length===0?'[]':gap?'[\n'+gap+
partial.join(',\n'+gap)+'\n'+
mind+']':'['+partial.join(',')+']';gap=mind;return v;}
if(rep&&typeof rep==='object'){length=rep.length;for(i=0;i<length;i+=1){k=rep[i];if(typeof k==='string'){v=str(k,value);if(v){partial.push(quote(k)+(gap?': ':':')+v);}}}}else{for(k in value){if(Object.hasOwnProperty.call(value,k)){v=str(k,value);if(v){partial.push(quote(k)+(gap?': ':':')+v);}}}}
v=partial.length===0?'{}':gap?'{\n'+gap+partial.join(',\n'+gap)+'\n'+
mind+'}':'{'+partial.join(',')+'}';gap=mind;return v;}}
if(typeof UJSON.stringify!=='function'){UJSON.stringify=function(value,replacer,space){var i;gap='';indent='';if(typeof space==='number'){for(i=0;i<space;i+=1){indent+=' ';}}else if(typeof space==='string'){indent=space;}
rep=replacer;if(replacer&&typeof replacer!=='function'&&(typeof replacer!=='object'||typeof replacer.length!=='number')){throw new Error('JSON.stringify');}
return str('',{'':value});};}
if(typeof UJSON.parse!=='function'){UJSON.parse=function(text,reviver){var j;function walk(holder,key){var k,v,value=holder[key];if(value&&typeof value==='object'){for(k in value){if(Object.hasOwnProperty.call(value,k)){v=walk(value,k);if(v!==undefined){value[k]=v;}else{delete value[k];}}}}
return reviver.call(holder,key,value);}
text=String(text);cx.lastIndex=0;if(cx.test(text)){text=text.replace(cx,function(a){return'\\u'+
('0000'+a.charCodeAt(0).toString(16)).slice(-4);});}
if(/^[\],:{}\s]*$/.test(text.replace(/\\(?:["\\\/bfnrt]|u[0-9a-fA-F]{4})/g,'@').replace(/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,']').replace(/(?:^|:|,)(?:\s*\[)+/g,''))){j=eval('('+text+')');return typeof reviver==='function'?walk({'':j},''):j;}
throw new SyntaxError('JSON.parse');};}}());if(!window.SocketProAdapter)window.SocketProAdapter={};if(!SocketProAdapter.ClientSide)SocketProAdapter.ClientSide={};var UHTTP=SocketProAdapter.ClientSide.HTTP=function(defaultConfig){var qMeta=[];var qCall=[];var callIndex=0;var me={};function toInt(d){if(typeof(d)=='boolean')return d?1:0;if(!d)d="0";var str=String(d);return parseInt(str,10);}
if(!defaultConfig)defaultConfig={};var dc=defaultConfig;var loc=document.location;if(typeof dc.protocol=='string'&&dc.protocol.length>0)
dc.protocol=dc.protocol.toLowerCase();else
dc.protocol=loc.protocol.toLowerCase();if(typeof dc.hostname=='string'&&dc.hostname.length>0)
dc.hostname=dc.hostname.toLowerCase();else
dc.hostname=loc.hostname.toLowerCase();if(!dc.port||typeof dc.port!='number')
dc.port=toInt(loc.port);if(!dc.pathname||typeof dc.pathname!='string')
dc.pathname=loc.pathname;me.defaultConfig=dc;function onProcessed(index,data){for(var n=0;n<qCall.length;n++){var req=qCall[n];if(index==req.getCallIndex()){var cb=req.getCallback();req.abort();if(cb)cb(req,data);break;}}}
function createAjaxObject(){var xmlHttp;try{xmlHttp=new XMLHttpRequest();}
catch(e){try{xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");}
catch(e){xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");}}
return xmlHttp;}
var XMLRPCMessage=function(){this.toXml=function(methodName,params){var xml="";xml+="<?xml version=\"1.0\"?>\n";xml+="<methodCall>\n";xml+=("<methodName>"+methodName+"</methodName>\n");xml+="<params>\n";for(var i=0;i<params.length;i++){var data=params[i];xml+="<param>\n";if(data==null||typeof data=='undefined')
xml+=("<value>"+"<nil/>"+"</value>\n");else
xml+=("<value>"+getParam(dataTypeOf(data),data)+"</value>\n");xml+="</param>\n";}
xml+="</params>\n";xml+="</methodCall>";return xml;};function dateToISO8601(date){var year=new String(date.getYear());var month=leadingZero(new String(date.getMonth()));var day=leadingZero(new String(date.getDate()));var time=leadingZero(new String(date.getHours()))+":"+leadingZero(new String(date.getMinutes()))+":"+leadingZero(new String(date.getSeconds()));var converted=year+month+day+"T"+time;return converted;}
function leadingZero(n){if(n.length==1)
n="0"+n;return n;}
function getParam(type,data){var xml;switch(type){case"date":xml=doDate(data);break;case"array":xml=doArray(data);break;case"struct":xml=doStruct(data);break;case"boolean":xml=doBoolean(data);break;default:xml=doValue(type,data);break;}
return xml;}
function doStruct(data){var xml="<struct>\n";for(var i in data){xml+="<member>\n";xml+="<name>"+i+"</name>\n";if(data[i]==null||typeof(data[i])=='undefined')
xml+=("<value>"+"<nil/>"+"</value>\n");else
xml+=("<value>"+getParam(dataTypeOf(data[i]),data[i])+"</value>\n");xml+="</member>\n";}
xml+="</struct>\n";return xml;}
function doArray(data){var xml="<array><data>\n";for(var i=0;i<data.length;i++){if(data[i]==null||typeof(data[i])=='undefined')
xml+=("<value>"+"<nil/>"+"</value>\n");else
xml+=("<value>"+getParam(dataTypeOf(data[i]),data[i])+"</value>\n");}
xml+="</data></array>\n";return xml;}
function doDate(data){var xml="<dateTime.iso8601>";xml+=dateToISO8601(data);xml+="</dateTime.iso8601>";return xml;}
function doBoolean(data){var value=data?1:0;var xml="<boolean>"+value+"</boolean>";return xml;}
function doValue(type,data){var xml="<"+type+">"
if(type=='string'){var str=data.replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;");xml+=(str+"</"+type+">");}
else
xml+=(data+"</"+type+">");return xml;}
function dataTypeOf(o){var type=typeof(o);type=type.toLowerCase();switch(type){case"number":if(Math.round(o)==o)
type='i4';else
type='double';break;case'object':var con=o.constructor;if(con==Date)
type='date';else if(con==Array)
type='array';else
type='struct';break;default:break;}
return type;}
return this;}();function JSMethod(name,isXML,version,config){var urlScript;var callback;var urlCall;var jsScript=null;var cIndex=0;var xmlHttp=null;if(!name||typeof name!='string')
throw new Error(500,'Req name must be a valid string.');var byScript=true;var me={};var m_pl={};var ajaxDone=true;if(!version)version=1.1;function init(){if(typeof config.protocol=='string')
config.protocol=config.protocol.toLowerCase();switch(config.protocol){case'https:':case'https':if(!config.port)
config.port=443;config.protocol="https:";break;case'http:':case'http':if(!config.port)
config.port=80;config.protocol='http:';break;default:break;}
var loc=document.location;var url={protocol:loc.protocol.toLowerCase(),hostname:loc.hostname.toLowerCase(),port:loc.port,pathname:loc.pathname};if(!config.protocol)
config.protocol=url.protocol;if(!config.hostname)
config.hostname=url.hostname;url.port=toInt(url.port);do{if(url.port==0){switch(url.protocol){case'https:':case'https':url.port=443;break;case'http:':case'http':url.port=80;break;default:break;}}
if(!config.port)
config.port=url.port;if(url.protocol==config.protocol&&url.hostname==config.hostname&&url.port==config.port)
byScript=false;}while(false);me.config=config;}
function setUrls(){urlCall='';urlScript='';urlCall=me.config.protocol+'//'+me.config.hostname;if(me.config.port){urlCall+=':';urlCall+=me.config.port;}
if(byScript){urlScript=urlCall;if(typeof(me.config.pathname)=='string'&&me.config.pathname.length>0){var pos=me.config.pathname.indexOf('/');if(pos!=0)
me.config.pathname='/'+me.config.pathname;}
urlScript+=me.config.pathname;}
if(me.config.pathname){pos=me.config.pathname.indexOf('.');if(pos==-1)
urlCall+=me.config.pathname;else
urlCall+=me.config.pathname.substr(0,pos);}}
function sendDataByScript(strQuery){var script=document.createElement('script');script.setAttribute("type","text/javascript");script.setAttribute('src',strQuery);script.setAttribute('charset','utf-8');UHTTP.cb=onProcessed;document.body.appendChild(script);jsScript=script;}
function ajaxCallback(){if(xmlHttp&&xmlHttp.readyState==4){ajaxDone=true;if(callback)
callback(me,xmlHttp.responseText);}}
function doCall(isGet,sync){var strData='';var pos;setUrls();if(byScript){qCall.push(me);strData=urlScript;pos=strData.indexOf('?');if(pos!=(strData.length-1))
strData+='?';var temp="UJS_CB=UHTTP.cb("+cIndex+','+"&UJS_DATA="+toString(isGet);strData+=encodeURIComponent(temp);sendDataByScript(strData);}
else{if(jsScript){document.body.removeChild(jsScript);jsScript=null;}
strData=toString(isGet);if(isGet){strData=encodeURIComponent(strData);strData='UJS_DATA='+strData;}
xmlHttp=sendDataByAjax(!isGet,strData,sync);}}
function sendDataByAjax(post,data,sync){if(!xmlHttp)xmlHttp=createAjaxObject();if(!xmlHttp)return;if(!sync)xmlHttp.onreadystatechange=ajaxCallback;var strUrl=config.pathname;if(!post){var pos=strUrl.indexOf('?');if(pos!=strUrl.length-1)
strUrl+=('?'+data);else
strUrl+=data;}
ajaxDone=false;xmlHttp.open(post?'POST':'GET',strUrl,!sync);if(post)
xmlHttp.setRequestHeader('Content-length',data?data.length:0);xmlHttp.setRequestHeader('Content-Type',isXML?'text/xml; charset=utf-8':'application/json; charset=utf-8');xmlHttp.setRequestHeader('Accept',isXML?'text/xml,application/json':'application/json');xmlHttp.send(post?data:null);if(sync){ajaxDone=true;ajaxCallback(xmlHttp);}
return xmlHttp;}
function toString(isGet){var str;var temp=me.config;delete me.config;me.params=m_pl;me.id=cIndex;if(isXML&&!isGet&&!byScript){var params=[];for(var key in me.params)
params.push(me.params[key]);str=XMLRPCMessage.toXml(name,params);}
else{me.version=version;str=UJSON.stringify(me);delete me.version;}
delete me.params;delete me.id;me.config=temp;return str;}
me.isCrossSite=function(){return byScript;};me.method=name;me.getCallIndex=function(){return cIndex;};me.abort=function(){if(xmlHttp)xmlHttp.abort();for(var n=0;n<qCall.length;n++){if(qCall[n]==me){qCall.splice(n,1);if(jsScript){document.body.removeChild(jsScript);jsScript=null;}
break;}}
ajaxDone=true;};me.isCompleted=function(){for(var n=0;n<qCall.length;n++){if(qCall[n]==me){return false;}}
return ajaxDone;};me.getCallback=function(){return callback;};me.invoke=function(cb,isGet,sync){if(!me.method&&typeof(me.method)!='string')
throw new Error(500,"Request name not specified properly.");if(typeof(cb)=='string')
cb=eval(cb);if(typeof(cb)!='function')
throw new Error(500,"Callback function required");if(!me.isCompleted())
throw new Error(500,"Previous request not completed yet.");cIndex=++callIndex;callback=cb;doCall(isGet,sync);return callIndex;};me.clear=function(){m_pl={};};me.add=function(name,value){if(typeof name!='string'||!name)
throw new Error(500,'Bad parameter name.');m_pl[name]=value;};me.getScript=function(){return(jsScript||xmlHttp);};init();return me;}
me.createRequest=function(name,isXML,version,config){if(typeof config=='string')
eval(config);else if(!config)
config=UHTTP.defaultConfig;return new JSMethod(name,isXML,version,config);};me.Chat=function(){var c={};c.onMessage=null;c.isXML=0;c.version=1.1;c.sync=0;c.isGet=0;c.crossSite=0;var listening=0;var config=me.defaultConfig;var callback=function(req,data){var oMsg;var bS=0;if(data&&typeof(data)=='object')
oMsg=data;else if(typeof(data)=='string'&&data.length>0)
oMsg=UJSON.parse(data);if(typeof(oMsg)!='object'||typeof(req)!='object')
return;if(oMsg.messages&&oMsg.messages.length){var s=oMsg.messages[0];if(s.msg=='__UCOMET_MESSAGE__')
bS=1;}
if(req.method=='enter')
config.chatId=oMsg.ret;if(c.onMessage&&!bS)
c.onMessage(req,oMsg);if(req.method=='listen')
listening=false;var ok=c.chatting();if(ok&&((req.method=='listen'&&oMsg.ret!==false)||req.method=='enter'))
c.listen();};var addEvent=function(obj,type,fn,useCapture){if(obj.addEventListener)
obj.addEventListener(type,fn,useCapture);else if(obj.attachEvent){obj["e"+type+fn]=fn;obj[type+fn]=function(){obj["e"+type+fn](window.event);};obj.attachEvent("on"+type,obj[type+fn]);}};c.chatting=function(){return(typeof(config.chatId)=='string'&&config.chatId.length==38);};c.chatId=function(){if(c.chatting())
return config.chatId;return"";};c.speak=function(msg,groups){if(!isOk())
return 0;var req=me.createRequest('speak',c.isXML,c.version,UHTTP.defaultConfig);setInternal(req);req.add('groups',groups);req.add("message",msg);req.add("chatId",config.chatId);req.invoke(callback,c.isGet,c.sync);return req;};c.sendUserMessage=function(msg,userId){if(!isOk())
return 0;var req=me.createRequest('sendUserMessage',c.isXML,c.version,UHTTP.defaultConfig);setInternal(req);req.add('userId',userId);req.add("message",msg);req.add("chatId",config.chatId);req.invoke(callback,c.isGet,c.sync);return req;};c.enter=function(userId,groups){var req=me.createRequest('enter',c.isXML,c.version,UHTTP.defaultConfig);setInternal(req);req.add('groups',groups);req.add("userId",userId);req.invoke(callback,c.isGet,c.sync);return req;};function isOk(){if(!c.chatting())
config.chatId=UHTTP.defaultConfig.chatId;if(!c.chatting()){alert("No chat id available!");return 0;}
return 1;}
c.listen=function(){if(listening||!isOk())
return 0;var req=me.createRequest('listen',c.isXML,c.version,UHTTP.defaultConfig);setInternal(req);req.add("chatId",config.chatId);req.invoke(callback,c.isGet,false);listening=true;return req;};c.subscribe=function(delayTime){if(typeof delayTime!='number'||delayTime<0)
delayTime=0;setTimeout(c.listen,delayTime);};function setInternal(req){c.crossSite=req.isCrossSite()?1:0;if(c.crossSite){c.sync=0;c.isGet=1;}}
c.exit=function(){listening=false;var req=me.createRequest('exit',c.isXML,c.version,UHTTP.defaultConfig);setInternal(req);if(typeof(config.chatId)=='string'&&config.chatId.length==38){req.add("chatId",config.chatId);config.chatId='';req.invoke(callback,c.isGet,c.sync);}
config.chatId='';return req;};addEvent(window,'unload',c.exit);return c;}();return me;}();var UChat=UHTTP.Chat;