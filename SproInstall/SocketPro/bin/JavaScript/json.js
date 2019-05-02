
if(!this.JSON)
JSON={_ESCAPES:/\\["\\\/bfnrtu]/g,_VALUES:/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,_BRACKETS:/(?:^|:|,)(?:\s*\[)+/g,_INVALID:/^[\],:{}\s]*$/,_SPECIAL_CHARS:/["\\\x00-\x1f\x7f-\x9f]/g,_PARSE_DATE:/^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z$/,_CHARS:{'\b':'\\b','\t':'\\t','\n':'\\n','\f':'\\f','\r':'\\r','"':'\\"','\\':'\\\\'},_applyFilter:function(data,filter){var walk=function(k,v){var i,n;if(v&&typeof v==='object'){for(i in v){if(v.hasOwnProperty(i)){n=walk(i,v[i]);if(n===undefined){delete v[i];}else{v[i]=n;}}}}
return filter(k,v);};if(typeof(filter)=='function'){walk('',data);}
return data;},isValid:function(str){if(typeof(str)!='string'){return false;}
return this._INVALID.test(str.replace(this._ESCAPES,'@').replace(this._VALUES,']').replace(this._BRACKETS,''));},dateToString:function(d){function _zeroPad(v){return v<10?'0'+v:v;}
return'"'+d.getUTCFullYear()+'-'+
_zeroPad(d.getUTCMonth()+1)+'-'+
_zeroPad(d.getUTCDate())+'T'+
_zeroPad(d.getUTCHours())+':'+
_zeroPad(d.getUTCMinutes())+':'+
_zeroPad(d.getUTCSeconds())+'Z"';},stringToDate:function(str){if(this._PARSE_DATE.test(str)){var d=new Date();d.setUTCFullYear(RegExp.$1,(RegExp.$2|0)-1,RegExp.$3);d.setUTCHours(RegExp.$4,RegExp.$5,RegExp.$6);return d;}},parse:function(s,filter){if(this.isValid(s)){return this._applyFilter(eval('('+s+')'),filter);}
throw new SyntaxError('parseJSON');},stringify:function(o,w,d){var J=this,m=J._CHARS,str_re=this._SPECIAL_CHARS,pstack=[];var _char=function(c){if(!m[c]){var a=c.charCodeAt();m[c]='\\u00'+Math.floor(a/16).toString(16)+
(a%16).toString(16);}
return m[c];};var _string=function(s){return'"'+s.replace(str_re,_char)+'"';};var _date=J.dateToString;var _stringify=function(o,w,d){var t=typeof o,i,len,j,k,v,vt,a;if(t==='string'){return _string(o);}
if(t==='boolean'||o instanceof Boolean){return String(o);}
if(t==='number'||o instanceof Number){return isFinite(o)?String(o):'null';}
if(o instanceof Date){return _date(o);}
if(o instanceof Array){for(i=pstack.length-1;i>=0;--i){if(pstack[i]===o){return'null';}}
pstack[pstack.length]=o;a=[];if(d>0){for(i=o.length-1;i>=0;--i){a[i]=_stringify(o[i],w,d-1)||'null';}}
pstack.pop();return'['+a.join(',')+']';}
if(t==='object'){if(!o){return'null';}
for(i=pstack.length-1;i>=0;--i){if(pstack[i]===o){return'null';}}
pstack[pstack.length]=o;a=[];if(d>0){if(w){for(i=0,j=0,len=w.length;i<len;++i){if(typeof w[i]==='string'){v=_stringify(o[w[i]],w,d-1);if(v){a[j++]=_string(w[i])+':'+v;}}}}else{j=0;for(k in o){if(typeof k==='string'&&o.hasOwnProperty(k)){v=_stringify(o[k],w,d-1);if(v){a[j++]=_string(k)+':'+v;}}}}}
pstack.pop();return'{'+a.join(',')+'}';}
return undefined;};d=d>=0?d:1/0;return _stringify(o,w,d);}};