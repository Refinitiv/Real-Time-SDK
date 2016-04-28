package com.thomsonreuters.upa.valueadd.examples.common;

import com.thomsonreuters.upa.transport.Channel;

/**
 * Client session information.
 */
public class ClientSessionInfo
{
   Channel clientChannel;
   PingHandler pingHandler = new PingHandler();
   long start_time;
   
   
   public Channel clientChannel()
   {
       return clientChannel;
   }
   
   public void clear()
   {
	   clientChannel = null;
       pingHandler.clear();
       start_time = 0;
   }
   
  public long startTime()
  {
	  return start_time;
  }
}
