package com.thomsonreuters.upa.examples.edfexamples.edfconsumer;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.CommandLine;
import com.thomsonreuters.upa.examples.common.PingHandler;
import com.thomsonreuters.upa.examples.common.ResponseCallback;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.SymbolListHandler.SymbolListEntry;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceSeqMcastInfo;

/**
 * Handles processing of messages from the gap fill data feed. 
 *
 */

public class GapFillSession implements ResponseCallback
{

    /* Session structure */
    public class GapFillSessionState
    {
        static final int GAPFILL_STATE_START = 1;
        static final int GAPFILL_STATE_INITIALIZED = 2;
        static final int GAPFILL_STATE_LOGIN_REQUESTED = 3;
        static final int GAPFILL_STATE_ITEMS_REQUESTED = 4;
        static final int GAPFILL_STATE_READY = 5;
        static final int GAPFILL_STATE_FINISHED = 6;
    }

    long runtime;
    int state = GapFillSessionState.GAPFILL_STATE_START;
    int seqNumInt;
    int msgSize;
    byte[] byteInput = new byte[4];
    static final int headerSize = 6;
    List<Integer> serversConnectedTo = new ArrayList<Integer>();
    
    StringBuilder outputString = new StringBuilder();

    private DictionaryHandler dictionaryHandler;

    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private Msg responseMsg = CodecFactory.createMsg();

    static List<EDFChannelSession> channelSessions = new ArrayList<EDFChannelSession>();
    private static Error error = TransportFactory.createError(); // error
                                                                 // information

    private SymbolListHandler symbolListHandler;
    private ServiceSeqMcastInfo seqMcastInfo;
    private SymbolListEntry itemEntry;
    
    private EDFWatchList watchlist;

    public GapFillSession(EDFWatchList watchlist)
    {
        dictionaryHandler = new DictionaryHandler();
        channelSessions = new ArrayList<EDFChannelSession>();
        this.watchlist = watchlist;
        error = TransportFactory.createError();
    }

    private static void closeChannel()
    {
        for (ChannelSession channel : channelSessions)
            channel.uninit(error);
    }

    public void init(String[] args, RefDataSession refDataSession, SnapshotSession snapshotSession)
    {
        // process command line args
        EDFConsumer.addCommandLineArgs();
        try
        {
            CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ile)
        {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(CodecReturnCodes.FAILURE);
        }

        try
        {
            runtime = System.currentTimeMillis() + CommandLine.intValue("runtime") * 1000;
        }
        catch (NumberFormatException ile)
        {
            System.err.println("Invalid argument, number expected.\t");
            System.err.println(ile.getMessage());
            System.exit(CodecReturnCodes.FAILURE);
        }

        // load dictionary
        dictionaryHandler = refDataSession.dictionary();
        
        seqMcastInfo = refDataSession.seqMcastInfo();
        symbolListHandler = snapshotSession.symbolListHandler();
        
        PingHandler pingHandler = new PingHandler();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        
        List<String> addresses;
        List<String> ports;
        List<String> interfaces;
        
        if (state != GapFillSessionState.GAPFILL_STATE_READY)
        {
            if ((addresses = CommandLine.values("gfsa")) != null &&
                        (ports = CommandLine.values("gfsp")) != null &&
                        (interfaces = CommandLine.values("gfif")) != null)
            {
                for (int i = 0; i < addresses.size(); ++i)
                {
                    try
                    {
                        connect(inProg, addresses.get(i), ports.get(i), interfaces.get(i));
                    }
                    catch (InterruptedException e)
                    {
                        System.out.println("Thread: " + Thread.currentThread() + " interrupted. Error:"
                                + e.getLocalizedMessage());
                        return;
                    }
                }
            }
            else
                try
                {
                        if (CommandLine.values("mbo") != null)
                            for (int i = 0; i < CommandLine.values("mbo").size(); ++i)
                            {
                                if ((itemEntry = symbolListHandler.symbolList().get(CommandLine.values("mbo").get(i))) != null)
                                {
                                    if(!checkIfConnected(itemEntry.getGapFillChannelId(DomainTypes.MARKET_BY_ORDER)))
                                    {
                                        connect(inProg, itemEntry.getGapFillChannelId(DomainTypes.MARKET_BY_ORDER));
                                    }
                                }
                                    
                            }
                        if (CommandLine.values("mbp") != null)
                            for (int i = 0; i < CommandLine.values("mbp").size(); ++i)
                            {
                                if ((itemEntry = symbolListHandler.symbolList().get(CommandLine.values("mbp").get(i))) != null)
                                {
                                    if(!checkIfConnected(itemEntry.getGapFillChannelId(DomainTypes.MARKET_BY_PRICE)))
                                    {
                                        connect(inProg, itemEntry.getGapFillChannelId(DomainTypes.MARKET_BY_PRICE));
                                    }
             
                                }
                                    
                            }
                        if (CommandLine.values("mp") != null)
                            for (int i = 0; i < CommandLine.values("mp").size(); ++i)
                            {
                                if ((itemEntry = symbolListHandler.symbolList().get(CommandLine.values("mp").get(i))) != null)
                                {
                                    if(!checkIfConnected(itemEntry.getGapFillChannelId(DomainTypes.MARKET_PRICE)))
                                    {
                                        connect(inProg, itemEntry.getGapFillChannelId(DomainTypes.MARKET_PRICE));
                                    }
                                }
                                    
                            }
                }
                catch (InterruptedException intExp)
                {
                    System.out.println("Thread: " + Thread.currentThread() + " interrupted. Error:"
                            + intExp.getLocalizedMessage());
                    return;
                }
        }
        // Handle run-time
        if (System.currentTimeMillis() >= runtime)
        {
            System.out.println("GAPFILLSession  run-time expired...");
            return;
        }

        state = GapFillSessionState.GAPFILL_STATE_READY;

        
        // Initialize ping handlers
        for (ChannelSession channelSession : channelSessions)
            pingHandler.initPingHandler(channelSession.channel().pingTimeout());
        
    }

    private boolean checkIfConnected(int connection)
    {
        if (serversConnectedTo.contains(connection))
            return true;
        else
            return false;
    }
    
    // connection to multicast groups
    private void connect(InProgInfo inProg, int serverSelect) throws InterruptedException
    {
        String hostName = new String(seqMcastInfo.gapMCastChanServerList().get(serverSelect).data().array());
        String portNumber = seqMcastInfo.gapMCastChanPortList().get(serverSelect).toString();
        String interfaceName = CommandLine.value("gfif");
        
        System.out.println("Starting connection to Gap Fill Session...");
        
        channelSessions.add(new EDFChannelSession(this));
        if (channelSessions.get(channelSessions.size() - 1).initTransport(false, error) < CodecReturnCodes.SUCCESS)
            System.exit(error.errorId());

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSessions.get(channelSessions.size() - 1).enableXmlTrace(dictionaryHandler.dictionary());
        }

        // get connect options from the channel session
        ConnectOptions copts = channelSessions.get(channelSessions.size() - 1).getConnectOptions();

        // set the connection parameters on the connect options
        copts.unifiedNetworkInfo().address(hostName);
        copts.unifiedNetworkInfo().serviceName(portNumber);
        copts.unifiedNetworkInfo().interfaceName(interfaceName);
        copts.blocking(false);
        copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);

        channelSessions.get(channelSessions.size() - 1).connect(inProg, error);
        
        serversConnectedTo.add(serverSelect);
    }
    
    // connection to multicast groups
    private void connect(InProgInfo inProg, String address, String port, String interfaceName) throws InterruptedException
    {
        System.out.println("Starting connection to Gap Fill Session...");
        
        channelSessions.add(new EDFChannelSession());
        if (channelSessions.get(channelSessions.size() - 1).initTransport(false, error) < CodecReturnCodes.SUCCESS)
            System.exit(error.errorId());

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSessions.get(channelSessions.size() - 1).enableXmlTrace(dictionaryHandler.dictionary());
        }

        // get connect options from the channel session
        ConnectOptions copts = channelSessions.get(channelSessions.size() - 1).getConnectOptions();

        // set the connection parameters on the connect options
        copts.unifiedNetworkInfo().address(address);
        copts.unifiedNetworkInfo().serviceName(port);
        copts.unifiedNetworkInfo().interfaceName(interfaceName);
        copts.blocking(false);
        copts.connectionType(ConnectionTypes.SEQUENCED_MCAST);

        channelSessions.get(channelSessions.size() - 1).connect(inProg, error);
    }

    /**
     * Call back method to process responses from channel. Processing responses
     * consists of performing a high level decode of the message and then
     * calling the applicable specific method for further processing. 
     * @param chnl - The channel of the response 
     * @param buffer - The message buffer containing the response.
     */
    
    @Override
    public void processResponse(ChannelSession chnl, TransportBuffer buffer)
    {
        if (buffer.length() < 40)
            return;
        
        outputString = new StringBuilder();

        // If message, display which channel Session this is on
        for (int i = 0; i < channelSessions.size(); ++i)
        {
            if (channelSessions.get(i).channelInfo().channel() == ((EDFChannelSession)chnl).channelInfo().channel())
            {
                outputString.append("<Gapfill Channel " + i + "> ");
                break;
            }
        }
        
        // clear decode iterator
        dIter.clear();
        
        outputString.append("SEQ NO: " + ((EDFChannelSession)chnl).channelInfo().readArgs().seqNum() + "\n");

        // set buffer and version info
        dIter.setBufferAndRWFVersion(buffer, chnl.channel().majorVersion(), chnl.channel()
                .minorVersion());

        int ret = responseMsg.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("\nDecodeMsg(): Error " + ret + " on SessionData Channel="
                    + chnl.channel().selectableChannel() + "  Size "
                    + (buffer.data().limit() - buffer.data().position()));
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        processResponse(chnl, responseMsg, dIter, outputString);
    }

    private void processResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter, StringBuilder outputString)
    {
        switch (responseMsg.domainType())
        {
            case DomainTypes.MARKET_PRICE:
                processMarketPriceResp(responseMsg, dIter, outputString);
                break;
            case DomainTypes.MARKET_BY_ORDER:
                processMarketByOrderResp(responseMsg, dIter, outputString);
                break;
            case DomainTypes.MARKET_BY_PRICE:
                processMarketByPriceResp(responseMsg, dIter, outputString);
                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
        }
    }
    
    private void processMarketByPriceResp(Msg responseMsg, DecodeIterator dIter, StringBuilder outputString)
    {
        outputString.append("DOMAIN: MARKET_BY_PRICE" + "\n");
        if (watchlist.processMsg(responseMsg, dIter, this, outputString) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void processMarketByOrderResp(Msg responseMsg, DecodeIterator dIter, StringBuilder outputString)
    {
        outputString.append("DOMAIN: MARKET_BY_ORDER" + "\n");
        if (watchlist.processMsg(responseMsg, dIter, this, outputString) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }

    private void processMarketPriceResp(Msg responseMsg, DecodeIterator dIter, StringBuilder outputString)
    {
        outputString.append("DOMAIN: MARKET_PRICE" + "\n");
        if (watchlist.processMsg(responseMsg, dIter, this, outputString) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }
    
    private void closeDictAndItemStreams()
    {
        // close dictionary streams if opened
        for (ChannelSession channelSession : channelSessions)
            dictionaryHandler.closeStreams(channelSession, error);
    }
    
    
    private void flushChannel()
    {
        int retval = 1;
        for (ChannelSession channelSession : channelSessions)
        {
            while (retval > TransportReturnCodes.SUCCESS)
            {
                retval = channelSession.flush(error);
            }
    
            if (retval < TransportReturnCodes.SUCCESS)
            {
                System.out.println("Flush() failed with return code " + retval + "- <" + error
                        .text() + ">");
            }
        }
    }
    
    /**
     * Closes all streams for the Gap fill session.
     */
    public void uninitialize()
    {
        System.out.println("Gap fill Session unitializing...");
        for (ChannelSession channelSession : channelSessions)
        {
            if (channelSession.channel() == null)
            {
                channelSession.uninit(error);
                System.exit(TransportReturnCodes.SUCCESS);
            }
    
            closeDictAndItemStreams();
        }

        // flush before exiting
        flushChannel();

        closeChannel();
    }
    
    
    public List<EDFChannelSession> channelSessions()
    {
        return channelSessions;
    }

}
