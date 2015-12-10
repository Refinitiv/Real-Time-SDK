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
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession.ChannelInfo;
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
 *  Handles processing of messages from the realtime data feed. 
*/

public class RealTimeSession implements ResponseCallback
{

    /* Session structure */
    public class RealTimeSessionState 
    {
        static final int REALTIME_STATE_START = 1;
        static final int REALTIME_STATE_INITIALIZED = 2;
        static final int REALTIME_STATE_LOGIN_REQUESTED = 3;
        static final int REALTIME_STATE_ITEMS_REQUESTED = 4;
        static final int REALTIME_STATE_READY = 5;
        static final int REALTIME_STATE_FINISHED = 6;
    }

    long runtime;
    int state = RealTimeSessionState.REALTIME_STATE_START;
    int seqNumInt;
    int msgSize;
    byte[] byteInput = new byte[4];
    static final int headerSize = 6;
    List<Integer> serversConnectedTo = new ArrayList<Integer>();

    private DictionaryHandler dictionaryHandler;

    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private Msg responseMsg = CodecFactory.createMsg();

    static List<EDFChannelSession> channelSessions = new ArrayList<EDFChannelSession>();
    private static Error error = TransportFactory.createError(); // error
                                                                 // information
    
    private EDFWatchList watchlist;

    private SymbolListHandler symbolListHandler;
    private ServiceSeqMcastInfo seqMcastInfo;
    private SymbolListEntry itemEntry;

    private StringBuilder outputString = new StringBuilder("");

    private boolean gapDetected = false;


    public RealTimeSession(EDFWatchList watchlist)
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
        
        if (state != RealTimeSessionState.REALTIME_STATE_READY)
        {
            if ((addresses = CommandLine.values("rtda")) != null &&
                        (ports = CommandLine.values("rtdp")) != null &&
                        (interfaces = CommandLine.values("rtif")) != null)
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
                            if(!checkIfConnected(itemEntry.getRealTimeChannelId(DomainTypes.MARKET_BY_ORDER)))
                            {
                                connect(inProg, itemEntry.getRealTimeChannelId(DomainTypes.MARKET_BY_ORDER));
                            }

                        }

                    }
                if (CommandLine.values("mbp") != null)
                    for (int i = 0; i < CommandLine.values("mbp").size(); ++i)
                    {
                        if ((itemEntry = symbolListHandler.symbolList().get(CommandLine.values("mbp").get(i))) != null)
                        {
                            if(!checkIfConnected(itemEntry.getRealTimeChannelId(DomainTypes.MARKET_BY_PRICE)))
                            {
                                connect(inProg, itemEntry.getRealTimeChannelId(DomainTypes.MARKET_BY_PRICE));
                            }
                        }

                    }
                if (CommandLine.values("mp") != null)
                    for (int i = 0; i < CommandLine.values("mp").size(); ++i)
                    {
                        if ((itemEntry = symbolListHandler.symbolList().get(CommandLine.values("mp").get(i))) != null)
                        {
                            if(!checkIfConnected(itemEntry.getRealTimeChannelId(DomainTypes.MARKET_PRICE)))
                            {
                                connect(inProg, itemEntry.getRealTimeChannelId(DomainTypes.MARKET_PRICE));
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
            System.out.println("RealTimeSession  run-time expired...");
            return;
        }

        state = RealTimeSessionState.REALTIME_STATE_READY;

        
     
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
        String hostName = new String(seqMcastInfo.streamingMCastChanServerList().get(serverSelect).data().array());
        String portNumber = seqMcastInfo.streamingMCastChanPortList().get(serverSelect).toString();
        String interfaceName = CommandLine.value("rtif");
        
        System.out.println("Starting connection to Real Time Session...");
        
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
        
        channelSessions.get(channelSessions.size() - 1).channelInfo().connectOptions(copts);

        serversConnectedTo.add(serverSelect);
    }

    // connection to multicast groups
    private void connect(InProgInfo inProg, String address, String port, String interfaceName) throws InterruptedException
    {
        
        System.out.println("Starting connection to Real Time Session...");
        
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
        
        outputString.delete(0, outputString.length());
        
        // If message, display which channel Session this is on
        for (int i = 0; i < channelSessions.size(); ++i)
        {
            if (channelSessions.get(i).channelInfo().channel() == ((EDFChannelSession)chnl).channelInfo().channel())
            {
                outputString.append("\n<Realtime Channel " + i + "> ");
                break;
            }
        }
        
        // clear decode iterator
        dIter.clear();
        
        outputString.append("SEQ NO: " + ((EDFChannelSession)chnl).channelInfo().readArgs().seqNum() + " ");
        
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

        ChannelInfo channelInfo = ((EDFChannelSession) chnl).channelInfo();

        for (ChannelInfo chanInfo : ((EDFChannelSession)chnl).channels())
        {
            if (chanInfo.connectOptions().unifiedNetworkInfo().address() == channelInfo.connectOptions().unifiedNetworkInfo().address() &&
                    chanInfo.connectOptions().unifiedNetworkInfo().serviceName() == channelInfo.connectOptions().unifiedNetworkInfo().serviceName())
            {
                
                if ( channelInfo.readArgs().seqNum() > chanInfo.gapInfo().start + 1 && chanInfo.gapInfo().start != 0)
                {
                    gapDetected = true;
                    chanInfo.gapInfo().start = chanInfo.gapInfo().start + 1;
                    chanInfo.gapInfo().end =  channelInfo.readArgs().seqNum() - 1;
                }
                else
                {
                    chanInfo.gapInfo().start =  channelInfo.readArgs().seqNum();
                    chanInfo.gapInfo().end =  channelInfo.readArgs().seqNum();
                }
                
                // Check if address and port are on this gapInfo, if not, put it on based on channel's info
                if (chanInfo.gapInfo().address.data() == null)
                {
                    chanInfo.gapInfo().address.data(channelInfo.connectOptions().unifiedNetworkInfo().address());
                    chanInfo.gapInfo().port = Integer.valueOf(channelInfo.connectOptions().unifiedNetworkInfo().serviceName());
                }

                break;
            }
        }
        


        processResponse(chnl, responseMsg, dIter, outputString);
    }

    private void processResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter, StringBuilder outputString)
    {
        switch (responseMsg.domainType())
        {
            case DomainTypes.MARKET_PRICE:
            case DomainTypes.MARKET_BY_ORDER:
            case DomainTypes.MARKET_BY_PRICE:
                watchlist.processMsg(chnl, responseMsg, dIter, this, outputString);
                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
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
    }
    
    /**
     * Closes all streams for the snapshot session.
     */
    public void uninitialize()
    {
        System.out.println("Real time session unitializing...");
        for (ChannelSession channelSession : channelSessions)
        {
            if (channelSession.channel() == null)
            {
                if (channelSession.channel() == null)
                {
                    channelSession.uninit(error);
                    System.exit(TransportReturnCodes.SUCCESS);
                }
        
                closeDictAndItemStreams();
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
    
    public boolean gapDetected()
    {
        return gapDetected;
    }
    
    public void gapDetected(boolean gapDetectedBoolean)
    {
        gapDetected = gapDetectedBoolean;
    }
}
