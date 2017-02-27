package SPA.ServerSide;

	public interface IServerQueue extends SPA.IMessageQueueBasic
	{
		int getHandle();
		/** 
		 Replicate all messages within this queue onto one target queue
		 
		 @param serverQueue A target queue for appending messages from this queue
		 @return True for success; and false for fail. To make the call success, a target queue should be already opened and available
		*/
		boolean AppendTo(IServerQueue serverQueue);

		/** 
		 Replicate all messages within this queue onto an array of queues
		 
		 @param serverQueues An array of target queues for appending messages from this queue
		 @return True for success; and false for fail. To make the call success, all of target queues should be already opened and available
		*/
		boolean AppendTo(IServerQueue[] serverQueues);

		/** 
		 Replicate all messages within this queue onto an array of queues
		 
		 @param queueHandles An array of target queues for appending messages from this queue
		 @return True for success; and false for fail. To make the call success, all of target queues should be already opened and available
		*/
		boolean AppendTo(int[] queueHandles);


		/** 
		 Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
		 
		 @param serverQueue A target queue for appending messages from this queue 
		 @return True for success; and false for fail. To make the call success, a target queue should be already opened and available
		*/
		boolean EnsureAppending(IServerQueue serverQueue);

		/** 
		 Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
		 
		 @param serverQueues An array of target queues for appending messages from this queue
		 @return True for success; and false for fail. To make the call success, all of target queues should be already opened and available
		*/
		boolean EnsureAppending(IServerQueue[] serverQueues);

		/** 
		 Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
		 
		 @param queueHandles An array of target queues for appending messages from this queue
		 @return True for success; and false for fail. To make the call success, all of target queues should be already opened and available
		*/
		boolean EnsureAppending(int[] queueHandles);
	}