package com.refinitiv.eta.valueadd.reactor;

import java.util.Queue;

import com.refinitiv.eta.valueadd.common.VaNode;

class QueueLink extends VaNode 
{
    Queue<ReactorChannel> reactorQueue;

    void queue(Queue<ReactorChannel> reactorQueue)
    {
        this.reactorQueue = reactorQueue;
    }

    void clear()
    {
        if (reactorQueue != null)
            reactorQueue.clear();
    }
    
    int size()
    {
    	if (reactorQueue != null)
    		return reactorQueue.size();
    	else
    		return 0;
    }
    
    void add(ReactorChannel reactorChannel)
    {
    	if (reactorQueue != null)
    		reactorQueue.add(reactorChannel);
    }

	public ReactorChannel poll() {
		return reactorQueue.poll();
	}
}
