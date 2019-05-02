
if(!window.SocketProAdapter) window.SocketProAdapter = {};
if(!SocketProAdapter.ClientSide) SocketProAdapter.ClientSide = {};

SocketProAdapter.ClientSide.CAsyncServiceHandler = function(){
    var m_cs;
    this.Attach = function(clientSocket){
        if(m_cs)
            return false;
        if(!clientSocket){
            alert("Must attach with valid socket!");
            return false;
        }
        m_cs = clientSocket;
        clientSocket.Attach(this);
        return true;
    };
    this.Detach = function(){
        if(m_cs){
			cs = m_cs;
			m_cs = null;
            cs.Detach(this);
		}
    };
    this.GetAttachedClientSocket = function(){
        return m_cs;
    };
    this.SendRequest = function(requestId){
        if(!m_cs) return false;
        s = m_cs.GetUSocket();
        if(!s)return false;
        s.SendJSRequest(requestId);
        return (s.Rtn == 0);
    };
};

SocketProAdapter.ClientSide.CClientSocket = function(s){
    var NA;
    var na = "No socket object available";
    var asyncHandlers = [];
    this.getClientSocket = function(){
        return s;
    };
    this.getAsyncHandlers = function(){
        return asyncHandlers;
    };
    this.Speak = function(msg, lGroups){
        if(!s)return na;
        if(!lGroups) lGroups = -1;
        s.Speak(msg, lGroups);
        return (s.Rtn == 0);
    };
    this.SendUserMessage = function(uid, msg){
        if(!s)return na;
        s.SendUserMessage(uid, msg);
        return (s.Rtn == 0);
    };
    this.Exit = function(){
        if(!s)return na;
        s.Exit();
        return (s.Rtn == 0);
    };
    this.Enter = function(lGroups){
        if(!s)return na;
        if(!lGroups) lGroups = -1;
        s.Enter(lGroups);
        return (s.Rtn == 0);
    };
    this.Attach = function(handler){
        if(!handler)
            return false;
        len = asyncHandlers.length;
        for(n=0; n<len; n++){
            if(asyncHandlers[n].GetSvsID() == handler.GetSvsID()){
                alert("Handler service identification number must be unqiue to each of sockets!");
                return false;
            }
            else if(asyncHandlers[n] === handler){
                return true; //already attached
            }
        }
        handler.Attach(this);
        asyncHandlers.push(handler);
        return true;
    };
    this.Detach = function(handler){
        if(handler){
            handler.Detach();
            len = asyncHandlers.length;
            for(i=0; i<len; i++){
                if(asyncHandlers[i] == handler){
                     asyncHandlers.splice(i, 1);
                     break;
                }
            }
        }
    };
    this.GetAsyncHandlers = function(){
        return asyncHandlers;
    };
    this.GetCountOfAttachedServiceHandlers = function(){
        return asyncHandlers.length;
    };
    this.SendRequest = function(requestId){
        if(!s)return na;
        return s.SendJSRequest(requestId)
    };
    this.Wait = function(usRequestID, ulTimeout, ulSvsID){
        if(!s)return na;
        if(ulTimeout === NA) 
            ulTimeout = -1;
        if(ulSvsID == 0)
            ulSvsID = GetCurrentServiceID();
        bTimeout = s.Wait(usRequestID, ulTimeout, ulSvsID);
        return (s.Rtn == 0 && bTimeout == false);
    };
    this.WaitAll = function(ulTimeout){
        if(!s)return na;
        if(ulTimeout === NA) 
            ulTimeout = -1;
        bTimeout = s.WaitAll(ulTimeout);
        return (s.Rtn == 0 && bTimeout == false);
    };
    this.Shutdown = function(lHow){
        if(!s)return na;
        if(lHow === NA) 
            lHow = 1;
        s.Shutdown(lHow);
    };
    this.GetUSocket = function(){
        if(!s)return na;
        return s;
    };
    this.SetUID = function(uid){
        if(!s)return na;
        s.UserID = uid;
    };
    this.SetPassword = function(pwd){
        if(!s)return na;
        s.Password = pwd;
    };
    this.Rollback = function(){
        if(!s)return na;
        s.AbortBatching();
        return (s.Rtn == 0);
    };
    this.IsConnected = function(){
        if(!s)return na;
        hSocket = s.Socket;
        return (hSocket != 0 && hSocket != -1);
    };
    this.SwitchTo = function(ulSvsID){
        if(!s)return na;
        s.SwitchTo(ulSvsID);
        s.Password = "";
        return (s.Rtn == 0 && this.IsConnected());
    };
    this.IsBatching = function(){
        if(!s)return na;
        return s.IsBatching;
    };
    this.GetSocket = function(){
        if(!s)return na;
        return s.Socket;
    };
    this.GetMemoryQueue = function(){
        if(!s)return na;
        return s.JSUQueue;
    };
    this.GetErrorMsg = function(){
        if(!s)return na;
        return s.ErrorMsg;
    };
    this.GetErrorCode = function(){
        if(!s)return na;
        return s.Rtn;
    };
    this.GetCurrentServiceID = function(){
        if(!s)return na;
        return s.CurrentSvsID;
    };
    this.GetCountOfRequestsInQueue = function(){
        if(!s)return na;
        return s.CountOfRequestsInQueue;
    };
    this.GetBytesBatched = function(){
        if(!s)return na;
        return s.BytesBatched;
    };
    this.Disconnect = function(){
        if(!s)return na;
        s.Disconnect();
    };
    this.DisableUI = function(bDisable){
        if(!s)return na;
        if(bDisable === NA) bDisable = true;
        s.Frozen = bDisable;
    };
    this.Commit = function(b){
        if(!s)return na;
        if(b === NA) 
            b = false;
        s.CommitBatching(b);
        return (s.Rtn == 0);
    }; 
    this.CleanTrack = function(){
        if(!s)return na;
        s.CleanTrack();
    };
    this.Cancel = function(lRequests){
        if(!lRequests)
            lRequests = -1;
        if(!s)return na;
        s.Cancel(lRequests);
        return (s.Rtn == 0);
    };
    this.BeginBatching = function(){
        if(!s)return na;
        s.StartBatching();
        return (s.Rtn == 0);
    };
    this.Connect = function(strHost, nPort, bSyn){
        if(!s)return na;
        if(bSyn === NA) 
            bSyn = false;
        s.JSObject = this;
        s.Connect(strHost, nPort, bSyn);
        return (s.Rtn == 0);
    };
    function init(){
        if(!s){            
            var newdiv = document.createElement("div");
            newdiv.style.visibility = "hidden";
            if(navigator.plugins && navigator.mimeTypes && navigator.mimeTypes.length){
                newdiv.innerHTML = '<embed type="application/udaparts-usocket-1" height="0" width="0" id="mysocket"></embed>';
            }
            else{
                newdiv.innerHTML = '<OBJECT ID="mysocket" WIDTH=0 HEIGHT=0 CLASSID="CLSID:AAD83433-CC8D-4F6D-9C62-E224DCA81358"></OBJECT>';
	        }
            document.body.appendChild(newdiv);
		    s = document.getElementById("mysocket");
	        if(s == null){
	            alert("Failed in creating socket!");
	            return;
	        }
	    }
	}
	init();
	return this;
};

SocketProAdapter.ClientSide.CClientSocket.prototype.OnRequestProcessed = function(hSocket, nRequestID, lLen, lLenInBuffer, sFlag){
    if(sFlag != 32) //rfCompleted
        return;
    var s = this.getClientSocket();
    var handlers = this.getAsyncHandlers();
    len =  handlers.length;
    if(nRequestID >= 0 && nRequestID < 46) { //46 -- idClose of UFile
        if(this.OnBaseRequestProcessed)
            this.OnBaseRequestProcessed(nRequestID);
	    for(n=0; n<len; n++){
		    if(handlers[n].OnBaseRequestProcessed) //notify all of attached handlers
		        handlers[n].OnBaseRequestProcessed(nRequestID);
	    }
	    return;
    }
    var CurrentSvsID = s.CurrentSvsID;
    for(n=0; n<len; n++){
        if(handlers[n].GetSvsID() == CurrentSvsID){
            handlers[n].OnResultReturned(nRequestID, s.JSUQueue); //if no OnResultReturned available
            s.JSUQueue.size = 0; //discard all of remaining data in memory queue
            break;
        }
    }
};