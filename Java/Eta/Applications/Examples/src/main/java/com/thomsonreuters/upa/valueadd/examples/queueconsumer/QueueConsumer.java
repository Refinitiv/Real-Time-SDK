package com.thomsonreuters.upa.valueadd.examples.queueconsumer;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Set;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerCallback;
import com.thomsonreuters.upa.valueadd.reactor.RDMDictionaryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMLoginMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDispatchOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;

/**
 * <p>
 * This is a main class to run the UPA Value Add Queue Consumer application.
 * </p>
 * <H2>Summary</H2>
 * <p>
 * The purpose of this application is to demonstrate consuming data from
 * a Queue Provider using Value Add components. It is a single-threaded
 * client application.
 * </p>
 * <p>
 * The consumer application implements callbacks that process information
 * received by the provider. It creates the Reactor, creates the desired
 * connections, then dispatches from the Reactor for events and messages.
 * </p>
 * <p>
 * This application is intended as a basic usage example. Some of the design choices
 * were made to favor simplicity and readability over performance. This application 
 * is not intended to be used for measuring performance. This application uses
 * Value Add and shows how using Value Add simplifies the writing of UPA
 * applications. Because Value Add is a layer on top of UPA, you may see a
 * slight decrease in performance compared to writing applications directly to
 * the UPA interfaces.
 * </p> 
 * <H2>Running the application:</H2>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runQueueConsumer -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runQueueConsumer -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-c specifies a connection to open:
 * <ul>
 *  <li>hostname:        Hostname of provider to connect to
 *  <li>port:            Port of provider to connect to
         <br>Example Usage: -c localhost:14002 
 *  </li>
 * </ul>
 * </li>
 * <li>-uname changes the username used when logging into the provider
 * 
 * <li>-qServiceName specifies the service name for queue messages
 *
 * <li>-qSourceName specifies the source name for queue messages (if specified, configures consumer to receive queue messages)
 *
 * <li>-qDestName specifies the destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified)
 *
 * <li>-x provides an XML trace of messages
 * 
 * <li>-runtime adjusts the running time of the application
 *
 * <li>-tsAuth (optional) specifies that consumer will request authentication when opening the tunnel stream.
 * 
 * <li>-tsDomain (optional) specifies the domain that consumer will use when opening the tunnel stream.
 * 
 * </ul>
 */
public class QueueConsumer implements ConsumerCallback
{    
    private final int MAX_QUEUE_DESTINATIONS = 10; 
    
    private Reactor reactor;
    private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
    private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
    private ConsumerCmdLineParser consumerCmdLineParser = new ConsumerCmdLineParser();
    private Selector selector;

    private long runtime;
               
    private Error error;    // error information
    
	private long closetime;
	private long closeRunTime; 
	boolean closeHandled;
    
    ArrayList<ChannelInfo> chnlInfoList = new ArrayList<ChannelInfo>();

    private QueueMsgHandler queueMsgHandler;
    
    public QueueConsumer()
    {
        error = TransportFactory.createError();
        dispatchOptions.maxMessages(1);
        closetime = 10; // 10 sec 
        try
        {
            selector = Selector.open();
        }
        catch (Exception e)
        {
            System.out.println("Selector.open() failed: " + e.getLocalizedMessage());
            System.exit(ReactorReturnCodes.FAILURE);
        }
    }

    /* Initializes the Value Add queue consumer application. */
    private void init(String[] args)
    {
        // parse command line
        if (!consumerCmdLineParser.parseArgs(args))
        {
            System.err.println("\nError loading command line arguments:\n");
            consumerCmdLineParser.printUsage();
            System.exit(CodecReturnCodes.FAILURE);
        }

        if (consumerCmdLineParser.connection() == null ||
            consumerCmdLineParser.connection().port() == null ||
            consumerCmdLineParser.connection().hostname() == null)
        {
            System.out.println(" No Connection is provided");
             System.exit(CodecReturnCodes.FAILURE);
        }

        System.out.println("Consumer initializing...");

        runtime = System.currentTimeMillis() + consumerCmdLineParser.runtime() * 1000;
        closeRunTime = System.currentTimeMillis() + (consumerCmdLineParser.runtime() + closetime) * 1000;
                        
        // enable Reactor XML tracing if specified
        if (consumerCmdLineParser.enableXmlTracing())
        {
            reactorOptions.enableXmlTracing();
        }

        // create reactor
        reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
        if (errorInfo.code() != ReactorReturnCodes.SUCCESS)
        {
            System.out.println("createReactor() failed: " + errorInfo.toString());
            System.exit(ReactorReturnCodes.FAILURE);            
        }
        
        // register selector with reactor's reactorChannel.
        try
        {
            reactor.reactorChannel().selectableChannel().register(selector,
                                                                SelectionKey.OP_READ,
                                                                reactor.reactorChannel());
        }
        catch (ClosedChannelException e)
        {
            System.out.println("selector register failed: " + e.getLocalizedMessage());
            System.exit(ReactorReturnCodes.FAILURE);
        }
        
        // create channel info, initialize channel info
        ConnectArg connectArg = consumerCmdLineParser.connection();       
       
        ChannelInfo chnlInfo = new ChannelInfo();
        chnlInfo.connectArg = connectArg;
            
        // initialize channel info
        initChannelInfo(chnlInfo);
    
        // connect channel
        int ret;
        if ((ret = reactor.connect(chnlInfo.connectOptions, chnlInfo.consumerRole, errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("Reactor.connect failed with return code: " + ret + " error = " + errorInfo.error().text());
            System.exit(ReactorReturnCodes.FAILURE);
        }
            
        // add to ChannelInfo list
        chnlInfoList.add(chnlInfo);
        
    }

    /* Runs the Value Add queue consumer application. */
    private void run()
    {
        int selectRetVal, selectTime = 1000;
        while (true)
        {
            Set<SelectionKey> keySet = null;
            try
            {
                selectRetVal = selector.select(selectTime);
                if (selectRetVal > 0)
                {
                    keySet = selector.selectedKeys();
                }
            }
            catch (IOException e)
            {
                System.out.println("select failed: " + e.getLocalizedMessage());
                System.exit(ReactorReturnCodes.FAILURE);
            }

            // nothing to read
            if (keySet != null)
            {
                Iterator<SelectionKey> iter = keySet.iterator();
                int ret = ReactorReturnCodes.SUCCESS;
                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    iter.remove();
                    try
                    {
                        if (key.isReadable())
                        {
                            // retrieve associated reactor channel and dispatch on that channel 
                            ReactorChannel reactorChnl = (ReactorChannel)key.attachment();

                            // dispatch until no more messages
                            while ((ret = reactorChnl.dispatch(dispatchOptions, errorInfo)) > 0) {}
                            if (ret == ReactorReturnCodes.FAILURE)
                            {
                                if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
                                        reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
                                {
                                    System.out.println("ReactorChannel dispatch failed");
                                    uninitialize();
                                    System.exit(ReactorReturnCodes.FAILURE);
                                }
                            }
                        }
                    }
                    catch (CancelledKeyException e)
                    {
                    } // key can be canceled during shutdown
                }
            }
	           // Handle run-time
            if (System.currentTimeMillis() >= runtime && !closeHandled)
            {
            	System.out.println("Consumer run-time expired, close now...");
            	handleClose();
            	closeHandled = true;                
            }
            else if (System.currentTimeMillis() >= closeRunTime )
            {
            	System.out.println("Consumer closetime expired, shutdown reactor.");
            	break;
            }
	        if (!closeHandled)
	        {
	        	 handleQueueMessaging();
	        }   
	        if(closeHandled &&
	           (queueMsgHandler == null || queueMsgHandler._chnlInfo == null || queueMsgHandler._chnlInfo.tunnelStream == null)) 
	        {
	        	break;
	        }
        }
    }

    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        
        switch(event.eventType())
        {
            case ReactorChannelEventTypes.CHANNEL_UP:
            {
                // register selector with channel event's reactorChannel
                try
                {
                    event.reactorChannel().selectableChannel().register(selector,
                                                                        SelectionKey.OP_READ,
                                                                        event.reactorChannel());
                }
                catch (ClosedChannelException e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                break;
            }
            case ReactorChannelEventTypes.FD_CHANGE:
            {
                System.out.println("Channel Change - Old Channel: "
                        + event.reactorChannel().oldSelectableChannel() + " New Channel: "
                        + event.reactorChannel().selectableChannel());
                
    	        // cancel old reactorChannel select
                SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();
    
                // register selector with channel event's new reactorChannel
                try
                {
                    event.reactorChannel().selectableChannel().register(selector,
                                                                    SelectionKey.OP_READ,
                                                                    event.reactorChannel());
                }
                catch (Exception e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_READY:
            {
                // set ReactorChannel on ChannelInfo
                chnlInfo.reactorChannel = event.reactorChannel();
                
                if (isRequestedServiceUp(chnlInfo))
                {
                    if (queueMsgHandler != null)
                    {
                        if (queueMsgHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                        {
                            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
                            {
                                uninitialize();
                                System.exit(ReactorReturnCodes.FAILURE);
                            }
                        }
                    }
                }
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
            {
    			if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down reconnecting: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down reconnecting");
    
                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");
                            
                // allow Reactor to perform connection recovery
                
                // unregister selectableChannel from Selector
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    if (key != null)
                        key.cancel();
                }
                // reset hasServiceInfo flag
                chnlInfo.hasServiceInfo = false;
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_DOWN:
            {
    			if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down");
    
                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");
                
                // unregister selectableChannel from Selector
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    if (key != null)
                        key.cancel();
                }

                // close ReactorChannel
                if (chnlInfo.reactorChannel != null)
                {
                    chnlInfo.reactorChannel.close(errorInfo);
                }
                break;
            }
            case ReactorChannelEventTypes.WARNING:
                System.out.println("Received ReactorChannel WARNING event\n");
                break;
            default:
            {
                System.out.println("Unknown channel event!\n");
                return ReactorCallbackReturnCodes.SUCCESS;
            }
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int defaultMsgCallback(ReactorMsgEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        Msg msg = event.msg();
        
        if (msg == null)
        {
            /* The message is not present because an error occurred while decoding it. Print 
             * the error and close the channel. If desired, the un-decoded message buffer
             * is available in event.transportBuffer(). */
            System.out.printf("defaultMsgCallback: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());
            // unregister selectableChannel from Selector
            if (event.reactorChannel().selectableChannel() != null)
            {
                SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();
            }

            // close ReactorChannel
            if (chnlInfo.reactorChannel != null)
            {
                chnlInfo.reactorChannel.close(errorInfo);
            }
            return ReactorCallbackReturnCodes.SUCCESS;
        }

        // set response message
        chnlInfo.responseMsg = msg;
        
        // set-up decode iterator if message has message body
        if (msg.encodedDataBody() != null && msg.encodedDataBody().data() != null)
        {
            // clear decode iterator
            chnlInfo.dIter.clear();
    
            // set buffer and version info
            chnlInfo.dIter.setBufferAndRWFVersion(msg.encodedDataBody(),
                                                event.reactorChannel().majorVersion(),
                                                event.reactorChannel().minorVersion());
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        LoginMsgType msgType = event.rdmLoginMsg().rdmMsgType();

        switch (msgType)
        {
            case REFRESH:
                System.out.println("Received Login Refresh for Username: " + ((LoginRefresh)event.rdmLoginMsg()).userName());
                System.out.println(event.rdmLoginMsg().toString());
                
                // save loginRefresh
                ((LoginRefresh)event.rdmLoginMsg()).copy(chnlInfo.loginRefresh);
                break;
            case STATUS:
                LoginStatus loginStatus = (LoginStatus)event.rdmLoginMsg();
                System.out.println("Received Login StatusMsg");
                if (loginStatus.checkHasState())
                {
                    System.out.println("    " + loginStatus.state());
                }
                break;
            default:
                System.out.println("Received Unhandled Login Msg Type: " + msgType);
                break;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        DirectoryMsgType msgType = event.rdmDirectoryMsg().rdmMsgType();

        switch (msgType)
        {
            case REFRESH:
                DirectoryRefresh directoryRefresh = (DirectoryRefresh)event.rdmDirectoryMsg();
                processServiceRefresh(directoryRefresh, chnlInfo);
                if (chnlInfo.serviceInfo.action() == MapEntryActions.DELETE)
                {
                    error.text("rdmDirectoryMsgCallback(): DirectoryRefresh Failed: directory service is deleted");
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                break;
            case UPDATE:
                DirectoryUpdate directoryUpdate = (DirectoryUpdate)event.rdmDirectoryMsg();
                processServiceUpdate(directoryUpdate, chnlInfo);
                if (chnlInfo.serviceInfo.action() == MapEntryActions.DELETE)
                {
                    error.text("rdmDirectoryMsgCallback(): DirectoryUpdate Failed: directory service is deleted");
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                if (isRequestedServiceUp(chnlInfo))
                {
                    if (queueMsgHandler != null)
                    {
                        if (queueMsgHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                        {
                            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
                            {
                                System.out.println(errorInfo.error().text());
                                uninitialize();
                                System.exit(ReactorReturnCodes.FAILURE);
                            }
                        }
                    }
                }
                break;
            case CLOSE:
                System.out.println("Received Source Directory Close");
                break;
            case STATUS:
                DirectoryStatus directoryStatus = (DirectoryStatus)event.rdmDirectoryMsg();
                System.out.println("\nReceived Source Directory StatusMsg");
                if (directoryStatus.checkHasState())
                {
                    System.out.println("    " + directoryStatus.state());
                }
                break;
            default:
                System.out.println("Received Unhandled Source Directory Msg Type: " + msgType);
                break;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
        return ReactorCallbackReturnCodes.SUCCESS;
    }
    
    private void processServiceRefresh(DirectoryRefresh directoryRefresh, ChannelInfo chnlInfo)
    {
        String serviceName = chnlInfo.connectArg.service();
        System.out.println("Received Source Directory Refresh");
        System.out.println(directoryRefresh.toString());
        for (Service service : directoryRefresh.serviceList())
        {
            if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId() ) 
            {
                chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
            }
            
            if(service.info().serviceName().toString() != null)
            {
                System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
                // cache service requested by the application
                if (service.info().serviceName().toString().equals(serviceName))
                {
                    // save serviceInfo associated with requested service name
                    if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
                    {
                        System.out.println("Service.copy() failure");
                        uninitialize();
                        System.exit(ReactorReturnCodes.FAILURE);                    
                    }
                    chnlInfo.hasServiceInfo = true;
                }
            }
        }
        if (!chnlInfo.hasServiceInfo)
        {
        	System.out.println("Problem with input serviceName.");
        }
    }
    
    private void processServiceUpdate(DirectoryUpdate directoryUpdate, ChannelInfo chnlInfo)
    {
        String serviceName = chnlInfo.connectArg.service();
        System.out.println("Received Source Directory Update");
        System.out.println(directoryUpdate.toString());
        for (Service service : directoryUpdate.serviceList())
        {
            if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId() ) 
            {
                chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
            }
            
            boolean updateServiceInfo = false;
            if(service.info().serviceName().toString() != null)
            {
                System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
                // update service cache - assume cache is built with previous refresh message
                if (service.info().serviceName().toString().equals(serviceName) ||
                    service.serviceId() == chnlInfo.serviceInfo.serviceId())
                {
                    updateServiceInfo = true;
                }
            }
            else if (service.serviceId() == chnlInfo.serviceInfo.serviceId())
            {
                updateServiceInfo = true;
            }
            if (updateServiceInfo)
            {
                // update serviceInfo associated with requested service name
                if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Service.copy() failure");
                    uninitialize();
                    System.exit(ReactorReturnCodes.FAILURE);                    
                }
                chnlInfo.hasServiceInfo = true;                
            }
        }
    }
    
    public boolean isRequestedServiceUp(ChannelInfo chnlInfo)
    {
        return  chnlInfo.hasServiceInfo &&
			chnlInfo.serviceInfo.checkHasState() && (!chnlInfo.serviceInfo.state().checkHasAcceptingRequests() ||
                chnlInfo.serviceInfo.state().acceptingRequests() == 1) && chnlInfo.serviceInfo.state().serviceState() == 1;
    
    }
    

    private void handleQueueMessaging()
    {
        for (ChannelInfo chnlInfo : chnlInfoList)
        {
            if (chnlInfo.loginRefresh == null ||
                chnlInfo.serviceInfo == null ||
                chnlInfo.reactorChannel == null ||
                chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
            {
                continue;
            }
            
            if (queueMsgHandler != null)
                queueMsgHandler.sendQueueMsg(chnlInfo.reactorChannel);
        }
    }

    
    private void initChannelInfo(ChannelInfo chnlInfo)
    {
        // set up consumer role   
        chnlInfo.consumerRole.defaultMsgCallback(this);
        chnlInfo.consumerRole.channelEventCallback(this);
        chnlInfo.consumerRole.loginMsgCallback(this);
        chnlInfo.consumerRole.directoryMsgCallback(this);

        // use command line login user name if specified
        if (consumerCmdLineParser.userName() != null && !consumerCmdLineParser.userName().equals(""))
        {
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            // use zero for stream id so default from role will override
            loginRequest.initDefaultRequest(0);
            loginRequest.userName().data(consumerCmdLineParser.userName());
            chnlInfo.consumerRole.rdmLoginRequest(loginRequest);
        }
        
        // initialize consumer role to default
        chnlInfo.consumerRole.initDefaultRDMLoginRequest();
        chnlInfo.consumerRole.initDefaultRDMDirectoryRequest();
 
        // set up reactor connect options
        chnlInfo.connectOptions.reconnectAttemptLimit(-1); // attempt to recover forever
        chnlInfo.connectOptions.reconnectMinDelay(1000); // 1 second minimum
        chnlInfo.connectOptions.reconnectMaxDelay(60000); // 60 second maximum
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().connectionType(chnlInfo.connectArg.connectionType());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(chnlInfo.connectArg.port());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(chnlInfo.connectArg.hostname());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().userSpecObject(chnlInfo);

        // handle queue messaging configuration
        if (consumerCmdLineParser.qSourceName() != null && !consumerCmdLineParser.qSourceName().equals(""))
        {
            if (consumerCmdLineParser.qDestNames().size() <= MAX_QUEUE_DESTINATIONS)
            {
                queueMsgHandler = new QueueMsgHandler(consumerCmdLineParser.qSourceName(),
                                                      consumerCmdLineParser.qDestNames(), chnlInfo.connectArg.tunnelAuth(), chnlInfo.connectArg.tunnelDomain());
            }
            else // exit if too many queue destinations entered
            {
                System.err.println("\nError: Example only supports " + MAX_QUEUE_DESTINATIONS + " queue destination names.\n");
                consumerCmdLineParser.printUsage();
                System.exit(CodecReturnCodes.FAILURE);
            }
        }
    }
    
    /* Uninitializes the Value Add queue consumer application. */
    private void uninitialize()
    {
        System.out.println("Consumer unitializing and exiting...");
        
        for (ChannelInfo chnlInfo : chnlInfoList)
        {   
            // close queue messaging streams
            if (queueMsgHandler != null &&
                chnlInfo.reactorChannel != null)
            {
                if (queueMsgHandler.closeStreams(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("queueMsgHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }
    
            // close ReactorChannel
            if (chnlInfo.reactorChannel != null)
            {
                chnlInfo.reactorChannel.close(errorInfo);
            }
        }
        
        // shutdown reactor
        reactor.shutdown(errorInfo);
    }

    private void handleClose()
    {
        System.out.println("Consumer handle close...");
        
        for (ChannelInfo chnlInfo : chnlInfoList)
        {   
            // close queue messaging streams
            if (queueMsgHandler != null &&
                chnlInfo.reactorChannel != null)
            {
                if (queueMsgHandler.closeStreams(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("queueMsgHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }
        }         
    }    
    
    
    public static void main(String[] args) throws Exception
    {
        QueueConsumer qcons = new QueueConsumer();
        qcons.init(args);
        qcons.run();
        qcons.uninitialize();
        System.exit(0);
    }
}
