package com.thomsonreuters.upa.examples.edfexamples.edfconsumer;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.shared.CommandLine;
import com.thomsonreuters.upa.shared.PingHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession.ChannelInfo;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.GapFillSession.GapFillSessionState;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.GapRequestSession.GapRequestSessionState;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.RealTimeSession.RealTimeSessionState;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.RefDataSession.RefDataSessionState;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.SnapshotSession.SnapshotSessionState;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;


/**
 * 
 * This Elektron Direct Feed (EDF) example demonstrates consuming market data
 * from the feed, requesting images from the snapshot server and processing
 * updates on items of interest as they are received from the realtime feed. 
 *
 * This application provides an example mechanism for synchronizing messages
 * received from the snapshot server and realtime feed.
 * 
 * <p>
 * This is a main class to run UPA EDF Consumer application.
 * </p>
 * <em>Summary</em>
 * <p>
 * This class is responsible for the following:
 * <ul>
 * <li>Initialize and set command line options.
 * <li>Initalize all session classes (Ref Data Session, Snapshot Session, Real Time Session,
 * Gap Request Session, Gap Fill Session).
 * <li>Register all connections from session classes onto a selector.
 * <li>Read messages from selector which process within the EDF Watchlist.
 * <li>Cleanup.
 * </ul>
 * <p>
 * This application is intended as a basic usage example. Some of the design
 * choices were made to favor simplicity and readability over performance. It is
 * not intended to be used for measuring performance. This application uses
 * Value Add and shows how using Value Add simplifies the writing of UPA
 * applications. Because Value Add is a layer on top of UPA, you may see a
 * slight decrease in performance compared to writing applications directly to
 * the UPA interfaces.
 * <p>
 * <em>Setup Environment</em>
 * <p>
 * The RDMFieldDictionary and enumtype.def files could be located in the
 * directory of execution or this application will request dictionary from
 * provider.
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Applications/Examples</i> directory and run <i>ant</i> to
 * build.
 * <p>
 * java -cp ./bin;../../Libs/upaValueAdd.jar;../../Libs/upa.jar
 * com.thomsonreuters.upa.examples.edfexamples.edfconsumer.EDFConsumer [-mp marketPriceItemName] 
 * [-mbo marketByOrderItemName] [-mbp marketByPriceItemName] [-rtda realTimeServerAddress]
 * [-rtdp realTimeServerPort] [-rtif realTimeNetworkInterface] [-ssa snapshotServerAddress]
 * [-ssp snapshotServerPort] [-grsa gapRequestServerAddress] [-grsp gapRequestServerPort]
 * [-gfsa gapFillServerAddress] [-gfsp gapFillServerPort] [-gfif gapFillNetworkInterface]
 * [-rdsa refDataServerAddress] [-rfsp refDataServerPort] [-serviceId serviceId] 
 * [-setDefDictName globalSetDefDictionaryName]
 * [-runtime runTime] [-x]
 * 
 * </p>
 * <ul>
 * <li>-mp Market Price domain item name. The user can
 * specify multiple -mp instances, where each occurrence is associated with a
 * single item. For example, specifying -mp TRI -mp GOOG will provide content
 * for two MarketPrice items.
 * <li>-mbo Market By Order domain item name. The user can specify
 * multiple -mbo instances, where each occurrence is associated with a single
 * item.
 * <li>-mbp market By Price domain item name. The user can specify
 * multiple -mbp instances, where each occurrence is associated with a single
 * item.
 * <li>-rtda specifies the address of the Realtime Data feed.
 * <li>-rtdp specifies the port of the Realtime Data feed.
 * <li>-rtif specifies the network interface address of the Realtime data feed.
 * <li>-ssa specifies the address of the Snapshot server.
 * <li>-ssp specifies the port of the Snapshot server.
 * <li>-grsa specifies the address of the Gap Request server.
 * <li>-grsp specifies the port of the Gap Request server.
 * <li>-gfsa specifies the address of the Gap Fill server.
 * <li>-gfsp specifies the port of the Gap Fill server.
 * <li>-gfif specifies the network interface address of the Gap Fill data feed.
 * <li>-rdsa specifies the address of the Ref Data server.
 * <li>-rdsp specifies the port of the Ref Data server.
 * <li>-serviceId specifies the id of the service to be used
 * <li>-setDefDictName specifies the global set definition dictionary name to be
 * requested from the Ref Data server.
 * <li>-x Provides XML tracing of messages.
 * <li>-runtime run time. Default is 600 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * 
 * </ul>
 * 
 * @see GapFillSession
 * @see GapRequestSession
 * @see RealTimeSession
 * @see RefDataSession
 * @see SnapshotSession
 * @see EDFSymbolListHandler
 * @see EDFChannelSession
 */

public class EDFConsumer
{
    /* Time at which application will exit. */
    private long runtime;
    
    // RefDataSession run-time in seconds
    static final int defaultRuntime = 600;
    
    /* Time in seconds it takes before gap request session sends a forced gap request */
    private long gapRequestTimer = 5;
    
    /* When gap request session timer starts counting from */
    private long gapRequestTimerStart;
    
    private static Error error;

    private EDFChannelSession channelSession;
    private RefDataSession refDataSession;
    private SnapshotSession snapshotSession;
    private RealTimeSession realTimeSession;
    private GapFillSession gapFillSession;
    private GapRequestSession gapRequestSession;
    private PingHandler pingHandler;
    private EDFWatchList watchlist;
    
    /**
     * Instantiates a new EDF consumer.
     */
    public EDFConsumer()
    {
        error = TransportFactory.createError();    // error information
        watchlist = new EDFWatchList();
        channelSession = new EDFChannelSession();
        refDataSession = new RefDataSession(watchlist);
        snapshotSession = new SnapshotSession(watchlist);
        realTimeSession = new RealTimeSession(watchlist);
        gapFillSession = new GapFillSession(watchlist);
        gapRequestSession = new GapRequestSession();
        pingHandler = new PingHandler();
    }

    /**
     * Adds the command line args.
     */
    public static void addCommandLineArgs()
    {
        CommandLine.programName("EDFConsumer");
        CommandLine.addOption("mp", "For each occurrence, requests item using Market Price domain.");
        CommandLine.addOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
        CommandLine.addOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");
        CommandLine.addOption("rtda", "specifies the address of the Realtime data feed.");
        CommandLine.addOption("rtdp", "specifies the port of the Realtime data feed.");
        CommandLine.addRequiredOption("rtif", "specifies the network interface address of the Realtime data feed.");
        CommandLine.addOption("ssa", "specifies the address of the Snapshot server.");
        CommandLine.addOption("ssp", "specifies the port of the Snapshot server.");
        CommandLine.addOption("grsa", "specifies the address of the Gap Request server.");
        CommandLine.addOption("grsp", "specifies the port of the Gap Request server.");
        CommandLine.addOption("gfsa", "specifies the address of the Gap Fill server.");
        CommandLine.addOption("gfsp", "specifies the port of the Gap Fill server.");
        CommandLine.addRequiredOption("gfif", "specifies the network interface address of the Gap Fill data feed.");
        CommandLine.addRequiredOption("rdsa", "specifies the address of the Ref Data server.");
        CommandLine.addRequiredOption("rdsp", "specifies the port of the Ref Data server.");
        CommandLine.addOption("serviceId", "specifies the id of the service to be used.");
        CommandLine.addOption("setDefDictName", "specifies the global set definition dictionary name to be requested from Ref Data Server.");
        CommandLine.addOption("runtime", defaultRuntime, "Program runtime in seconds");
        CommandLine.addOption("x", "Provides XML tracing of messages.");
    }

    /**
     * Initializes EDF consumer application.
     * 
     * It is responsible for: Initializing command line options used by the
     * application. Parsing command line arguments.
     * 
     * @param args
     */
    private void init(String[] args)
    {
        // Initialize EDFConsumer
        addCommandLineArgs();
        
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
        
    }
    
    /**
     * Main loop that handles session initialization, registering into selector, 
     * reading, and processing inside of the sessions from their channels.
     *
     * @param args the args
     */
    public void run(String[] args)
    {
        refDataSession.init(args);
        
        if (CommandLine.booleanValue("x"))
        {
            channelSession.enableXmlTrace(refDataSession.dictionaryHandler().dictionary());
        }
    
        pingHandler.initPingHandler(refDataSession.channelSession().channel().pingTimeout());
        
        channelSession.initTransport(false, error);
        
        channelSession.selectTime(refDataSession.channelSession().selectTime());
        channelSession.addAndRegisterChannelInfo(refDataSession.channelSession().channelInfo());
        
        // Finished initializing parts
        
        // Main processing loop
        // Handles initialization of all sessions and processing of all requests and responses inside of sessions
        do
        {
            channelSession.read(pingHandler, error);        
            
            if (refDataSession.state == RefDataSessionState.REF_DATA_STATE_FINISHED &&
                    snapshotSession.state == SnapshotSessionState.SNAPSHOT_STATE_START)
            {
                snapshotSession.init(args, refDataSession);
                channelSession.addAndRegisterChannelInfo(snapshotSession.channelSession().channelInfo());
            }
            
            if (snapshotSession.state == SnapshotSessionState.SNAPSHOT_STATE_FINISHED &&
                    realTimeSession.state == RealTimeSessionState.REALTIME_STATE_START)
            {
                realTimeSession.init(args,  refDataSession, snapshotSession);
                for (EDFChannelSession rtChannel : realTimeSession.channelSessions())
                {
                    channelSession.addAndRegisterChannelInfo(rtChannel.channelInfo());
                }
            }
            
            if (snapshotSession.state == SnapshotSessionState.SNAPSHOT_STATE_FINISHED &&
                    gapFillSession.state == GapFillSessionState.GAPFILL_STATE_START)
            {
                gapFillSession.init(args,  refDataSession, snapshotSession);
                for (EDFChannelSession gfChannel : gapFillSession.channelSessions())
                {
                    channelSession.addAndRegisterChannelInfo(gfChannel.channelInfo());
                }
            }
            
            if (snapshotSession.state == SnapshotSessionState.SNAPSHOT_STATE_FINISHED &&
                    gapRequestSession.state == GapRequestSessionState.GAPREQUEST_STATE_START)
            {
                gapRequestSession.init(args, refDataSession);
                channelSession.addAndRegisterChannelInfo(gapRequestSession.channelSession().channelInfo());
                gapRequestTimerStart = System.currentTimeMillis() + gapRequestTimer * 1000;
            }
            
            if (   ( (  snapshotSession.state == SnapshotSessionState.SNAPSHOT_STATE_FINISHED &&
                    gapRequestSession.state == GapRequestSessionState.GAPREQUEST_STATE_FINISHED) &&
                    (System.currentTimeMillis() >= gapRequestTimerStart ||
                    realTimeSession.gapDetected() ) ) )
            {
                // Make sure our request came from the RealTimeServer
                
                for (ChannelInfo chanInfo : channelSession.channels())
                {
                    if (chanInfo.connectOptions().unifiedNetworkInfo().address() == channelSession.channelInfo().connectOptions().unifiedNetworkInfo().address() &&
                             chanInfo.connectOptions().unifiedNetworkInfo().serviceName() == channelSession.channelInfo().connectOptions().unifiedNetworkInfo().serviceName())
                    {
                        if (chanInfo.callback().getClass() == realTimeSession.getClass() || !realTimeSession.gapDetected())
                        {
                            
                            gapRequestSession.sendRequests(gapRequestSession.channelSession(), channelSession, realTimeSession.gapDetected());
                            
                            realTimeSession.gapDetected(false);
                        
                            gapRequestTimerStart = System.currentTimeMillis() + gapRequestTimer * 1000;
                        }
                        
                        break;
                    }
                }

            }
        } while (System.currentTimeMillis() <= runtime);
        
        System.out.println("EDFConsumer run-time expired...");
        
    }
    
    /**
     * Closes all streams for each of the sessions.
     */
    public void uninitialize()
    {
        refDataSession.uninitialize();
        snapshotSession.uninitialize();
        realTimeSession.uninitialize();
        gapFillSession.uninitialize();
        gapRequestSession.uninitialize();
     }

    /**
     * The main method.
     *
     * @param args the arguments
     */
    public static void main(String[] args) 
    {
        EDFConsumer edfConsumer = new EDFConsumer();
        edfConsumer.init(args);
        edfConsumer.run(args);
        edfConsumer.uninitialize();
        System.out.println("Exiting application.");
     }
}
