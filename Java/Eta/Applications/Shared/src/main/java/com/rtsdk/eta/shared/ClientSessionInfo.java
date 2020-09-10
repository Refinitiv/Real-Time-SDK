package com.rtsdk.eta.shared;

import com.rtsdk.eta.transport.Channel;

/**
 * Client session information.
 */
public class ClientSessionInfo
{
   Channel clientChannel;
   PingHandler pingHandler = new PingHandler();
   long start_time;
   int socketFdValue = -1;
   
   
   public Channel clientChannel()
   {
       return clientChannel;
   }
   
   public void clear()
   {
	   clientChannel = null;
       pingHandler.clear();
       start_time = 0;
       socketFdValue = -1;
   }
   
  public long startTime()
  {
	  return start_time;
  }

  public int socketFdValue() {
       return socketFdValue;
  }
}
