package com.refinitiv.eta.perftools.transportperf;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.UnknownHostException;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.perftools.common.ClientChannelInfo;
import com.refinitiv.eta.perftools.common.CountStat;
import com.refinitiv.eta.perftools.common.LatencyRandomArray;
import com.refinitiv.eta.perftools.common.LatencyRandomArrayOptions;
import com.refinitiv.eta.perftools.common.ShutdownCallback;
import com.refinitiv.eta.perftools.common.TimeRecord;
import com.refinitiv.eta.perftools.common.TimeRecordQueue;
import com.refinitiv.eta.perftools.common.ValueStatistics;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.IoctlCodes;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;

/** Handles one transport thread. */
public class TransportThread extends Thread
{
    private final int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
    public static final int TEST_PROTOCOL_TYPE = 88;

    private TransportChannelHandler _channelHandler;    /* Contains a list of open channels and maintains those channels. */
    private long                    _currentTicks;      /* Current position in ticks per second. */
    private long                    _threadIndex;
    private long                    _connectTime;       /* Time of first connection. */
    private long                    _disconnectTime;    /* Time of last disconnection. */
    private CountStat               _msgsSent;          /* Total messages sent. */
    private CountStat               _bytesSent;         /* Total bytes sent(counting any compression) */
    private CountStat               _msgsReceived;      /* Total messages received. */
    private CountStat               _bytesReceived;     /* Total bytes received. */
    private CountStat               _outOfBuffersCount; /* Messages not sent for lack of output buffers. */
    private ValueStatistics         _latencyStats;      /* Latency statistics (recorded by stats thread). */
    private PrintWriter             _statsFile;         /* Statistics file for recording. */
    private PrintWriter             _latencyLogFile;    /* File for logging latency for this thread. */
    private SessionHandler          _sessionHandler;    /* Session handler for this thread. */
    private LatencyRandomArray      _latencyRandomArray; /* Random latency array */
    private LatencyRandomArrayOptions _randomArrayOpts; /* Random array options */

    private ProcessMsg _processMsg;
    
    private ConnectOptions _copts;
    private Error _error;
    private long _nsecPerTick;
    private ChannelInfo _chnlInfo;

    private ShutdownCallback _shutdownCallback; /* shutdown callback to main application */
    private volatile boolean    _shutdown;                  /* Signals thread to shutdown. */
    private volatile boolean    _shutdownAck;               /* Acknowledges thread is shutdown. */

    {
        _msgsSent = new CountStat();
        _bytesSent = new CountStat();
        _msgsReceived = new CountStat();
        _bytesReceived = new CountStat();
        _outOfBuffersCount = new CountStat();
        _latencyStats = new ValueStatistics();
        _latencyRandomArray = new LatencyRandomArray();
        _randomArrayOpts = new LatencyRandomArrayOptions();
        _copts = TransportFactory.createConnectOptions();
        _chnlInfo = TransportFactory.createChannelInfo();
    }

    /**
     * Instantiates a new transport thread.
     *
     * @param i the i
     * @param sessionHandler the session handler
     * @param processMsg the process msg
     * @param shutdownCallback the shutdown callback
     * @param error the error
     */
    public TransportThread(int i, SessionHandler sessionHandler, ProcessMsg processMsg, ShutdownCallback shutdownCallback, Error error)
    {
        _msgsSent.init();
        _bytesSent.init();
        _msgsReceived.init();
        _bytesReceived.init();
        _outOfBuffersCount.init();
        _latencyStats.clear();

        _connectTime = 0;
        _disconnectTime = 0;
        _threadIndex = i;
        _sessionHandler = sessionHandler;
        _processMsg = processMsg;
        _shutdownCallback = shutdownCallback;
        _error = error;
        _channelHandler = new TransportChannelHandler(_shutdownCallback, _error);

        /* create stats file writer for this thread */
        try
        {
            File statsFile = File.createTempFile(TransportThreadConfig.statsFilename()
                    + (_threadIndex + 1) + "_", ".csv", new File(System.getProperty("user.dir")));

            _statsFile = new PrintWriter(statsFile);
        }
        catch (FileNotFoundException e)
        {
            System.out.printf("Error: Failed to open stats file '%s'.\n",
                              TransportThreadConfig.statsFilename());
            System.exit(-1);
        }
        catch (IOException ioe)
        {
            System.out.printf("Error: Failed to open stats file '%s'.\n",
                              TransportThreadConfig.statsFilename());
            System.exit(-1);
        }
        _statsFile.println("UTC, Msgs sent, Bytes sent, Msgs received, Bytes received, Latency msgs received, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), CPU usage (%), Memory (MB)");

        /* create latency log file writer for this thread */
        if (TransportThreadConfig.logLatencyToFile())
        {
            try
            {
                File latencyLogFile = File.createTempFile(TransportThreadConfig.latencyLogFilename()
                                                    + (_threadIndex + 1) + "_", ".csv", new File(System.getProperty("user.dir")));

                _latencyLogFile = new PrintWriter(latencyLogFile);
            }
            catch (FileNotFoundException e)
            {
                System.out.printf("Error: Failed to open latency log file '%s'.\n", TransportThreadConfig.latencyLogFilename());
                System.exit(-1);
            }
            catch (IOException e)
            {
                System.out.printf("Error: Failed to open latency log file '%s'.\n", TransportThreadConfig.latencyLogFilename());
                System.exit(-1);
            }
            _latencyLogFile.println("Send Time, Receive Time, Latency(nsec)");
        }

        _currentTicks = 0;
        _nsecPerTick = 1000000000L/TransportThreadConfig.ticksPerSec();
        _channelHandler.init(this);
        
        if (TransportThreadConfig.latencyMsgsPerSec() > 0)
        {
            _randomArrayOpts.totalMsgsPerSec(TransportThreadConfig.msgsPerSec());
            _randomArrayOpts.latencyMsgsPerSec(TransportThreadConfig.latencyMsgsPerSec());
            _randomArrayOpts.ticksPerSec(TransportThreadConfig.ticksPerSec());
            _randomArrayOpts.arrayCount(LATENCY_RANDOM_ARRAY_SET_COUNT);
            if (_latencyRandomArray.create(_randomArrayOpts) != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Unable to create LatencyRandomArray");
                _shutdownCallback.shutdown();
                _shutdownAck = true;
            }
        }
    }
    
    /**
     * Cleanup.
     *
     * @param error the error
     */
    public void cleanup(Error error)
    {
        _channelHandler.cleanup(error);
        _statsFile.close();
        if (TransportThreadConfig.logLatencyToFile())
        {
            _latencyLogFile.close();
        }
    }

    /**
     * Process inactive channel.
     *
     * @param channelHandler the channel handler
     * @param channelInfo the channel info
     * @param error the error
     */
    public void processInactiveChannel(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, Error error)
    {
        SessionHandler handler = (SessionHandler)channelHandler.userSpec();
        TransportSession session = (TransportSession)channelInfo.userSpec;

        /* If the channel was active and this is a client, we won't attempt to reconnect,
         * so stop the test. */
        if (!_shutdown &&
            TransportPerfConfig.appType() == TransportPerfConfig.CLIENT &&
            session.timeActivated() > 0)
        {
            _shutdown = true;
        }
        
        if(error == null || error.errorId() == TransportReturnCodes.SUCCESS || "".equals(error.text()))
        {
            System.out.printf("Channel Closed.\n");
        }
        else
        {
            System.out.printf("Channel Closed: %d(%s)\n", error.errorId(), error.text());
        }

        handler.handlerLock().lock();
        handler.openChannelsCount(handler.openChannelsCount() - 1);
        handler.handlerLock().unlock();

        /* Record last disconnection time. */
        _disconnectTime = System.nanoTime();
        
        _shutdownAck = true;
    }

    /**
     * Process msg.
     *
     * @param channelHandler the channel handler
     * @param channelInfo the channel info
     * @param msgBuffer the msg buffer
     * @param error the error
     * @return the int
     */
    public int processMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, TransportBuffer msgBuffer, Error error)
    {
        return _processMsg.processMsg(channelHandler, channelInfo, msgBuffer, error);
    }

    /**
     * Process active channel.
     *
     * @param channelHandler the channel handler
     * @param channelInfo the channel info
     * @param error the error
     * @return the int
     */
    public int processActiveChannel(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, Error error)
    {
        TransportSession session = (TransportSession)channelInfo.userSpec;
        int ret;

        if (TransportPerfConfig.highWaterMark() > 0)
        {
            if (channelInfo.channel.ioctl(IoctlCodes.HIGH_WATER_MARK, TransportPerfConfig.highWaterMark(), error) != TransportReturnCodes.SUCCESS)
            {
                System.err.printf("Channel.Ioctl() of HIGH_WATER_MARK failed <%s>\n", error.text());
                return TransportReturnCodes.FAILURE;
            }
        }

        /* Record first connection time. */
        if (_connectTime == 0)
            _connectTime = System.nanoTime();

        /* retrieve and print out channel information */
        if ((ret = channelInfo.channel.info(_chnlInfo, error)) != TransportReturnCodes.SUCCESS)
        {
            System.err.printf("Channel.info() failed: %d <%s>\n", ret, error.text());
            return ret;
        } 
        System.out.printf("Channel active. " + _chnlInfo.toString() + "\n");

        if (channelInfo.channel.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
        {
               if (TransportThreadConfig.totalBuffersPerPack() > 1    &&
                       ((TransportThreadConfig.msgSize()) * TransportThreadConfig.totalBuffersPerPack() + ((TransportThreadConfig.totalBuffersPerPack() - 1) * 2)) > TransportPerfConfig.maxFragmentSize())
                   {
                       System.out.printf("Error(Channel %s): MaxBufferSize %d is too small for packing buffer size %d\n",
                               channelInfo.channel.selectableChannel(), TransportPerfConfig.maxFragmentSize(), 
                               ((TransportThreadConfig.msgSize()) * TransportThreadConfig.totalBuffersPerPack() + ((TransportThreadConfig.totalBuffersPerPack() - 1) * 2)));
                       System.exit(-1);
                   }
        }
        else if (TransportThreadConfig.totalBuffersPerPack() > 1    &&
            ((TransportThreadConfig.msgSize() + 8) * TransportThreadConfig.totalBuffersPerPack()) > _chnlInfo.maxFragmentSize())
        {
            System.out.printf("Error(Channel %s): MaxFragmentSize %d is too small for packing buffer size %d\n",
                    channelInfo.channel.selectableChannel(), _chnlInfo.maxFragmentSize(), 
                    ((TransportThreadConfig.msgSize() + 8) * TransportThreadConfig.totalBuffersPerPack()));
            System.exit(-1);
        }

        session.timeActivated(System.nanoTime());

        return TransportReturnCodes.SUCCESS;
    }
    
    /* (non-Javadoc)
     * @see java.lang.Thread#run()
     */
    public void run()
    {
        long currentTime, nextTickTime;

        /* handle client session initialization */
        handleClientSessionInit();

        
        nextTickTime = System.nanoTime();

        /* this is the main loop */
        while(!_shutdown)
        {
            currentTime = System.nanoTime();
            
            if (currentTime >= nextTickTime)
            {
                /* We've reached the next tick. Send a burst of messages out */
                nextTickTime += _nsecPerTick;
    
                if (!TransportPerfConfig.busyRead()) // not busy read, read channels and send messages
                {
                    if (_channelHandler.readChannels(nextTickTime, _error) < TransportReturnCodes.SUCCESS)
                    {
                        _shutdownCallback.shutdown();
                        break;
                    }
    
                    /* send messages for writer role (reflector role doesn't send messages) */
                    if((_sessionHandler.role() & TransportTestRole.WRITER) > 0)
                    {
                        for(ClientChannelInfo clientChannelInfo : _channelHandler.activeChannelList())
                        {
                            TransportSession session;
                            int ret;
    
                            session = (TransportSession)clientChannelInfo.userSpec;
    
                            /* The application corrects for ticks that don't finish before the time 
                             * that the next message burst should start.  But don't do this correction 
                             * for new channels. */
                            if (nextTickTime < session.timeActivated()) continue; 
    
                            /* Send burst of messages */
                            ret = session.sendMsgBurst(this, _error);
                            ++_currentTicks;
                            if (_currentTicks == TransportThreadConfig.ticksPerSec())
                                _currentTicks = 0;
                            if (ret < TransportReturnCodes.SUCCESS)
                            {
                                switch(ret)
                                {
                                    case TransportReturnCodes.NO_BUFFERS:
                                        _channelHandler.requestFlush(clientChannelInfo);
                                        // if channel no longer active, close channel
                                        if (clientChannelInfo.channel.state() != ChannelState.ACTIVE)
                                            _channelHandler.closeChannel(clientChannelInfo, _error);
                                        break;
                                    case TransportReturnCodes.FAILURE:
                                        System.out.printf("Failure while writing message bursts: %s (%d)\n", _error.text(), ret);
                                        _channelHandler.closeChannel(clientChannelInfo, _error);
                                        break;
                                    default:
                                        System.out.printf("Failure while writing message bursts: %s (%d)\n", _error.text(), ret);
                                        // if channel no longer active, close channel
                                        if (clientChannelInfo.channel.state() != ChannelState.ACTIVE)
                                            _channelHandler.closeChannel(clientChannelInfo, _error);
                                        break;
                                }
                            }
                            else if (ret > TransportReturnCodes.SUCCESS)
                            {
                                /* Need to flush */
                                _channelHandler.requestFlush(clientChannelInfo);
                            }
                        }
                    }
                }
                else // busy read, messages not sent in this mode
                {
                    for(ClientChannelInfo clientChannelInfo : _channelHandler.activeChannelList())
                    {
                        if (!_channelHandler.readChannel(clientChannelInfo, nextTickTime, _error))
                        {
                            _shutdownCallback.shutdown();
                            break;
                        }
                    }
    
                    for(ClientChannelInfo clientChannelInfo : _channelHandler.initializingChannelList())
                    {
                        if (_channelHandler.initializeChannel(clientChannelInfo, _error) < TransportReturnCodes.SUCCESS)
                        {
                            _shutdownCallback.shutdown();
                            break;
                        }
                    }
                }
    
                /* handle server accepted session initialization */
                handleAcceptedSessionInit();
    
                int ret;
                ret = _channelHandler.checkPings();
                if (ret < TransportReturnCodes.SUCCESS)
                {
                    for(ClientChannelInfo clientChannelInfo : _channelHandler.activeChannelList())
                    {
                    	_channelHandler.closeChannel(clientChannelInfo, _error);
                    }
                }
            }
        }
        
        if (_disconnectTime == 0)
        {
            _disconnectTime = System.nanoTime();
        }
    }
    
    /* handle client session initialization */
    private void handleClientSessionInit()
    {
        if (TransportPerfConfig.appType() == TransportPerfConfig.CLIENT)
        {
            do
            {
                int ret;
                Channel channel =  null;
                TransportSession session;
    
                if ((channel = startConnection()) == null)
                {
                    try
                    {
                        Thread.sleep(1000);
                    }
                    catch (InterruptedException e)
                    {
                        System.out.printf("Thread.sleep(1000) failed\n");
                        System.exit(-1);
                    }
                    continue;
                }
    
                session = new TransportSession(_channelHandler.addChannel(channel, TransportThreadConfig.checkPings()));
    
                if (session.channelInfo().channel.state() != ChannelState.ACTIVE)
                {
                    do
                    {
                        ret = _channelHandler.waitForChannelInit(session.channelInfo(), 100000, _error);
                    } while (!_shutdown && ret == TransportReturnCodes.CHAN_INIT_IN_PROGRESS);
        
                    if (ret < TransportReturnCodes.SUCCESS)
                    {
                        try
                        {
                            Thread.sleep(1000);
                        }
                        catch (InterruptedException e)
                        {
                            System.out.printf("Thread.sleep(1000) failed\n");
                            System.exit(-1);
                        }
                        continue;
                    }
                    else
                        break; /* Successful initialization. */
                }
                else
                    break;  /* Successful initialization. */
    
            } while (!_shutdown);
        }
    }
    
    /* handle server accepted session initialization */
    private void handleAcceptedSessionInit()
    {
        if (TransportPerfConfig.appType() == TransportPerfConfig.SERVER)
        {
            /* Check if there are any new connections. */
            _sessionHandler.handlerLock().lock();
            for (int i = 0; i < _sessionHandler.newChannelsList().size(); i++)
            {
                new TransportSession(_channelHandler.addChannel(_sessionHandler.newChannelsList().get(i), TransportThreadConfig.checkPings()));
                _sessionHandler.newChannelsList().remove(i);
            }
            _sessionHandler.handlerLock().unlock(); 
        }
    }   

    /* start a client connection */
    private Channel startConnection()
    {
        Channel chnl = null;
        _copts.clear();
        
        if (TransportPerfConfig.sAddr() || TransportPerfConfig.rAddr())
        {
            System.out.printf("\nAttempting segmented connect to server (send %s:%s,  recv %s:%s) unicastPort %s...\n", 
                TransportPerfConfig.sendAddr(), TransportPerfConfig.sendPort(), TransportPerfConfig.recvAddr(), TransportPerfConfig.recvPort(), TransportPerfConfig.unicastPort());
            _copts.segmentedNetworkInfo().recvAddress(TransportPerfConfig.recvAddr());
            _copts.segmentedNetworkInfo().recvServiceName(TransportPerfConfig.recvPort());
            _copts.segmentedNetworkInfo().sendAddress(TransportPerfConfig.sendAddr());
            _copts.segmentedNetworkInfo().sendServiceName(TransportPerfConfig.sendPort());
            _copts.segmentedNetworkInfo().interfaceName(TransportPerfConfig.interfaceName());
            _copts.segmentedNetworkInfo().unicastServiceName(TransportPerfConfig.unicastPort());    
        }
      else if (TransportPerfConfig.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
        {
            System.out.printf("\nAttempting to connect to group %s:%s...\n", TransportPerfConfig.hostName(), TransportPerfConfig.portNo());
            _copts.unifiedNetworkInfo().address(TransportPerfConfig.hostName());
            _copts.unifiedNetworkInfo().serviceName(TransportPerfConfig.portNo());
            _copts.unifiedNetworkInfo().interfaceName(TransportPerfConfig.interfaceName());
            _copts.unifiedNetworkInfo().unicastServiceName(TransportPerfConfig.unicastPort());  
        }
        else
        {
            System.out.printf("\nAttempting to connect to server %s:%s...\n", TransportPerfConfig.hostName(), TransportPerfConfig.portNo());
            _copts.unifiedNetworkInfo().address(TransportPerfConfig.hostName());
            _copts.unifiedNetworkInfo().serviceName(TransportPerfConfig.portNo());
            _copts.unifiedNetworkInfo().interfaceName(TransportPerfConfig.interfaceName());
            _copts.unifiedNetworkInfo().unicastServiceName(TransportPerfConfig.unicastPort());  
        }

        _copts.guaranteedOutputBuffers(TransportPerfConfig.guaranteedOutputBuffers());
        _copts.sysSendBufSize(TransportPerfConfig.sendBufSize());
        _copts.sysRecvBufSize(TransportPerfConfig.recvBufSize());
        _copts.majorVersion(0);
        _copts.minorVersion(0);
        _copts.protocolType(TEST_PROTOCOL_TYPE);
        _copts.connectionType(TransportPerfConfig.connectionType());
        _copts.tcpOpts().tcpNoDelay(TransportPerfConfig.tcpNoDelay());
        _copts.compressionType(TransportPerfConfig.compressionType());
        _copts.seqMCastOpts().maxMsgSize(TransportPerfConfig.maxFragmentSize());
		_copts.multicastOpts().tcpControlPort(TransportPerfConfig.tcpControlPort());
		_copts.multicastOpts().portRoamRange(TransportPerfConfig.portRoamRange());
        /* Multicast statistics are retrieved via Channel.info(),
         * so set the per-channel-lock when taking them. */
        if (TransportPerfConfig.takeMCastStats())
        {
            _copts.channelReadLocking(true);
            _copts.channelWriteLocking(true);
        }
                        
        if (TransportPerfConfig.connectionType() == ConnectionTypes.ENCRYPTED)          
        {          
            _copts.tunnelingInfo().tunnelingType("encrypted");
            setEncryptedConfiguration(_copts);
        }
        else if (TransportPerfConfig.connectionType() == ConnectionTypes.HTTP)
        {
            _copts.tunnelingInfo().tunnelingType("http");
            setHTTPconfiguration(_copts);
        }
        
        if (TransportPerfConfig.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
        {
        	_copts.guaranteedOutputBuffers(1);		// Only 1 output buffer currently in Sequenced MulticastConnection
        }

        if ((chnl = Transport.connect(_copts, _error)) == null)
        {
            System.out.printf("rsslConnect() failed: %d(%s)\n", _error.errorId(), _error.text());
            return null;
        }

        return chnl;
    }
    
    private void setEncryptedConfiguration(ConnectOptions options)
    {
        setHTTPconfiguration(options); 
        options.tunnelingInfo().KeystoreFile(TransportPerfConfig.keystoreFile());
        options.tunnelingInfo().KeystorePasswd(TransportPerfConfig.kestorePassword());
    }

    private void setHTTPconfiguration(ConnectOptions options)
    {
        if (TransportPerfConfig.hasProxy())
        {
            options.tunnelingInfo().HTTPproxy(true);
            options.tunnelingInfo().HTTPproxyHostName(TransportPerfConfig.proxyHost());
            options.tunnelingInfo().HTTPproxyPort(TransportPerfConfig.proxyPort());
        }

        // credentials
        if (options.tunnelingInfo().HTTPproxy())
        {
            setCredentials(options);
        }
    }
 
    private void setCredentials(ConnectOptions options)
    {
        options.credentialsInfo().HTTPproxyUsername(TransportPerfConfig.proxyUserName());
        options.credentialsInfo().HTTPproxyPasswd(TransportPerfConfig.proxyPassword());
        options.credentialsInfo().HTTPproxyDomain(TransportPerfConfig.proxyDomain());
        String localIPaddress = null;
        String localHostName = null;
        try
        {
            localIPaddress = InetAddress.getLocalHost().getHostAddress();
            localHostName = InetAddress.getLocalHost().getHostName();
        }
        catch (UnknownHostException e)
        {
            localHostName = localIPaddress;
        }
        options.credentialsInfo().HTTPproxyLocalHostname(localHostName);
        options.credentialsInfo().HTTPproxyKRB5configFile(TransportPerfConfig.proxyKBRConfigFile());
    }

            
    
    /**
     *  Submit a time record.
     *
     * @param recordQueue the record queue
     * @param startTime the start time
     * @param endTime the end time
     * @param ticks the ticks
     */
    public void timeRecordSubmit(TimeRecordQueue recordQueue, long startTime, long endTime, long ticks)
    {
        TimeRecord record;

        record = recordQueue.pool().poll();
        if (record == null)
        {
            record = new TimeRecord();
        }
        
        record.ticks(ticks);
        record.startTime(startTime);
        record.endTime(endTime);
        
        recordQueue.records().add(record);
    }

    TransportChannelHandler channelHandler()
    {
        return _channelHandler;
    }

    LatencyRandomArray latencyRandomArray()
    {
        return _latencyRandomArray;
    }

    long connectTime()
    {
        return _connectTime;
    }

    long disconnectTime()
    {
        return _disconnectTime;
    }

    ValueStatistics latencyStats()
    {
        return _latencyStats;
    }

    CountStat msgsSent()
    {
        return _msgsSent;
    }

    CountStat bytesSent()
    {
        return _bytesSent;
    }

    CountStat msgsReceived()
    {
        return _msgsReceived;
    }

    CountStat bytesReceived()
    {
        return _bytesReceived;
    }

    PrintWriter latencyLogFile()
    {
        return _latencyLogFile;
    }

    CountStat outOfBuffersCount()
    {
        return _outOfBuffersCount;
    }

    PrintWriter statsFile()
    {
        return _statsFile;
    }

    long currentTicks()
    {
        return _currentTicks;
    }

    /**
     *  Signals thread to shutdown.
     *
     * @param value the value
     */
    public void shutdown(boolean value)
    {
        _shutdown = value;
    }

    /**
     *  Acknowledges thread is shutdown.
     *
     * @return true, if successful
     */
    public boolean shutdownAck()
    {
        return _shutdownAck;
    }
}
