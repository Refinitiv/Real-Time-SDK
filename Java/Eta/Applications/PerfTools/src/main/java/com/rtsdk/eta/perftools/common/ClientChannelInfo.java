package com.rtsdk.eta.perftools.common;

import java.util.Queue;

import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.valueadd.reactor.ReactorChannel;
import com.rtsdk.eta.valueadd.reactor.TunnelStream;

/**
 * Used by the ChannelHandler object. Maintains information about a channel,
 * such as ping time information and whether flushing is being done.
 */
public class ClientChannelInfo
{
    public Channel                      channel;                // The ETA Channel associated with this info.
    public Object                       userSpec;               // Reference to user-specified data associated with this channel.
    public boolean                      needFlush;              // Whether this channel needs to have data flushed.
    public boolean                      needRead;               // Whether this channel has additional data to read.
    public boolean                      receivedMsg;            // Whether a ping or messages have been received since the last ping check.
    public boolean                      sentMsg;                // Whether a ping or messages have been sent since the last ping check.
    public boolean                      checkPings;             // Whether ping handling is done for this channel.
    public long                         nextReceivePingTime;    // Time before which this channel should receive a ping.
    public long                         nextSendPingTime;       // Time before which a ping should be sent for this channel.
    public Queue<ClientChannelInfo>     parentQueue;            // Reference back to the list this channel is an element of.
    public ReactorChannel               reactorChannel;         // Use the VA Reactor instead of the ETA Channel for sending and receiving.
    public boolean                      tunnelStreamOpenSent;   // flag to track if we already made a tunnel stream open request.
    public TunnelStream                 tunnelStream;           // The tunnel stream for this client's channel.
}
