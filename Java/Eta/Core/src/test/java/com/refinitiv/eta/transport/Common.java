package com.refinitiv.eta.transport;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import static org.junit.Assert.*;

public class Common {

    final static int TIMEOUTMS = 10000; // 10 seconds.

    static int executeHandshake(Selector selector, RsslSocketChannel channel, Error error, InProgInfo inProgInfo)
            throws InterruptedException {
        final int MAX_ATTEMPTS = 10;
        boolean client = channel._initChnlState != RsslSocketChannel.InitChnlState.READ_HDR;
        if (client) {
            initChannel(channel, error, inProgInfo);
        }
        for (int i = 0; i < MAX_ATTEMPTS; i++) {
            Set<SelectionKey> keySet = null;
            try {
                if (selector.select(1000) > 0) {
                    keySet = selector.selectedKeys();
                }
            } catch (IOException e) {
                System.out.println(e.getMessage());
                throw new InterruptedException();
            }
            if (keySet != null) {
                Iterator<SelectionKey> iter = keySet.iterator();

                while (iter.hasNext()) {
                    SelectionKey key = iter.next();
                    iter.remove();
                    if (!key.isValid()) {
                        key.cancel();
                        continue;
                    }
                    if (key.isReadable()) {
                        i = 0;
                        int state = initChannel(channel, error, inProgInfo);
                        if (state != ChannelState.INITIALIZING) {
                            return state;
                        }
                    }
                }
                Thread.sleep(500);
            }
        }
        return ChannelState.INACTIVE;
    }

    static int initChannel(Channel channel, Error error, InProgInfo inProg)
    {
        assertTrue(channel != null);
        int retval;

        // This finalizes the three-way handshake.

        if (channel.state() == ChannelState.INITIALIZING)
        {
            if ((retval = channel.init(inProg, error)) < TransportReturnCodes.SUCCESS)
            {
                // fail("\nChannel " + channel.selectableChannel() + " inactive: "
                // + error.text());
                System.out.println("initChannel(): Channel " + channel.selectableChannel()
                        + " inactive: " + error.text());
                // return channelState as inactive for our junits.
                return ChannelState.INACTIVE;
            }
            else
            {
                switch (retval)
                {
                    case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                        if (inProg.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                        {
                            System.out.println("\nChannel In Progress - New Channel: "
                                    + channel.selectableChannel() + " Old Channel: "
                                    + inProg.oldSelectableChannel());
                        }
                        System.out.println("\nChannel " + channel.selectableChannel() + " In Progress...");
                        break;
                    case TransportReturnCodes.SUCCESS:
                        System.out.println("\nChannel " + channel.selectableChannel() + " Is Active");
                        break;
                    default:
                        fail("\nBad return value channel=" + channel.selectableChannel() + " <"
                                + error.text() + ">");
                }
            }
        }

        return channel.state();
    }

    static Server serverBind(Selector selector, BindOptions bOpts, int pingTimeout, int minPingTimeout, int compressionType,
                             int compressionLevel, int protocolType, int majorVersion, int minorVersion, boolean blocking, Error error)
    {
        bOpts.pingTimeout(pingTimeout);
        bOpts.minPingTimeout(minPingTimeout);
        bOpts.guaranteedOutputBuffers(10);
        bOpts.compressionType(compressionType);
        if(compressionType > Ripc.CompressionTypes.NONE)
            bOpts.compressionLevel(compressionLevel);
        bOpts.protocolType(protocolType);
        bOpts.majorVersion(majorVersion);
        bOpts.minorVersion(minorVersion);
        bOpts.channelsBlocking(blocking);
        bOpts.serverBlocking(blocking);

        Server server = Transport.bind(bOpts, error);
        if (server == null)
            assertNotNull("failed to bind server, reason=" + error.text(), server);

        if(!blocking)
        {
            try
            {
                server.selectableChannel().register(selector, SelectionKey.OP_ACCEPT, server);
            }
            catch (Exception e)
            {
                assertTrue("failed to register server for OP_ACCEPT, exception=" + e.toString(), false);
            }
        }

        return server;
    }

    static Channel serverAccept(Selector selector, AcceptOptions aOpts, Error error)
    {

        Set<SelectionKey> keySet = null;

        try
        {
            if (selector.select(TIMEOUTMS) > 0)
            {
                keySet = selector.selectedKeys();
                if (keySet != null)
                {
                    Iterator<SelectionKey> iter = keySet.iterator();
                    while (iter.hasNext())
                    {
                        SelectionKey key = iter.next();
                        iter.remove();
                        if (key.isAcceptable())
                        {
                            Server server = (Server)key.attachment();
                            Channel channel = server.accept(aOpts, error);
                            if (channel != null)
                            {
                                channel.selectableChannel().register(selector, SelectionKey.OP_READ, channel);
                                return channel;
                            }
                            else
                            {
                                assertTrue("server.accept() failed to return a valid Channel, error="
                                        + error.text(), false);
                            }
                        }
                    }
                }
            }
        }
        catch (IOException e)
        {
            assertTrue("server failure during select, error=" + e.toString(), false);
        }

        assertTrue("server timed out waiting to accept a connection", false);
        return null;
    }
}
