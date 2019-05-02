
CMyAsyncHandler = function(){
    var m_idSvsID = 268435476; //tutorial one service id
    var idQueryCountCTOne = 8193;
    var idQueryGlobalCountCTOne = 8194;
    var idQueryGlobalFastCountCTOne = 8195;
    var idSleepCTOne = 8196;
    var idEchoCTOne = 8197;
    
    var me = new SocketProAdapter.ClientSide.CAsyncServiceHandler(); //inheriet from the base CAsyncServiceHandler
    me.GetSvsID = function(){ //must define the method GetSvsID()
        return m_idSvsID; 
    };
    me.OnResultReturned = function(nRequestID, jsQueue){ //must define the callback OnResultReturned(nRequestID, jsQueue)
        switch(nRequestID){
        case idQueryCountCTOne:
            m_QueryCountRtn = jsQueue.loadInt32();
            break;
        case idQueryGlobalCountCTOne:
            m_QueryGlobalCountRtn = jsQueue.loadInt32();
            break;
        case idQueryGlobalFastCountCTOne:
            m_QueryGlobalFastCountRtn = jsQueue.loadInt32();
            break;
        case idSleepCTOne:
            break;
        case idEchoCTOne:
            m_EchoRtn = jsQueue.load();
            break;
        default:
            break;
        }
    };
    
    me.m_QueryCountRtn = 0; //int
	me.QueryCountAsyn = function(){
	    var queue = me.GetAttachedClientSocket().GetMemoryQueue();
	    queue.size = 0;
		return me.SendRequest(idQueryCountCTOne);
	};
	me.m_QueryGlobalCountRtn = 0; //int
	me.QueryGlobalCountAsyn = function(){
	    var queue = me.GetAttachedClientSocket().GetMemoryQueue();
	    queue.size = 0;
		return me.SendRequest(idQueryGlobalCountCTOne);
	};
	me.m_QueryGlobalFastCountRtn = 0; //int
	me.QueryGlobalFastCountAsyn = function(){
	    var queue = me.GetAttachedClientSocket().GetMemoryQueue();
	    queue.size = 0;
		return me.SendRequest(idQueryGlobalFastCountCTOne);
	};
	me.SleepAsyn = function(nTime){
		var queue = me.GetAttachedClientSocket().GetMemoryQueue();
	    queue.size = 0;
	    queue.saveInt32(nTime);
		return me.SendRequest(idSleepCTOne);
	}
    	me.m_EchoRtn = 0; //variant
	me.EchoAsyn = function(objInput) //variant
	{
	    m_EchoRtn = null;
		var queue = me.GetAttachedClientSocket().GetMemoryQueue();
	    queue.size = 0;
	    queue.save(objInput);
		var b = me.SendRequest(idEchoCTOne);
		return b;
	};
	me.QueryCount = function(){
		me.QueryCountAsyn();
		if(me.GetAttachedClientSocket().WaitAll())
		    return m_QueryCountRtn;
		return false;
	};
	me.QueryGlobalCount = function(){
		if(!me.QueryGlobalCountAsyn())
		    return false;
		if(me.GetAttachedClientSocket().WaitAll())
		    return m_QueryGlobalCountRtn;
		return false;
	};
	me.QueryGlobalFastCount = function(){
		if(!me.QueryGlobalFastCountAsyn())
		    return false;
		if(me.GetAttachedClientSocket().WaitAll())
		    return m_QueryGlobalFastCountRtn;
		return false;
	};
	me.Sleep = function(nTime){
		if(!me.SleepAsyn(nTime))
		    return false;
		return me.GetAttachedClientSocket().WaitAll();
	};
	me.Echo = function(objInput){
		if(!me.EchoAsyn(objInput))
		    return false;
		if(me.GetAttachedClientSocket().WaitAll())
		    return m_EchoRtn;
		return false;
	};
	
	me.SendAllRequestsAsyncnronusly = function(sleepTime, echoData){
	    //error handling ignore here
	    me.GetAttachedClientSocket().BeginBatching();
	    me.EchoAsyn(echoData);
	    me.SleepAsyn(sleepTime);
	    me.QueryGlobalCountAsyn();
	    me.QueryCountAsyn();
	    me.QueryGlobalFastCountAsyn();
	    
	    //Send all reuests here, and true -- returning results in batch
	    me.GetAttachedClientSocket().Commit(true);
	    
	    me.GetAttachedClientSocket().WaitAll();
	};
    return me;
};
