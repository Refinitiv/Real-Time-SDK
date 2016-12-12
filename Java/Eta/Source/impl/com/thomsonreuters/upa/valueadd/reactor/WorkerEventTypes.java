package com.thomsonreuters.upa.valueadd.reactor;

/* Internal class used with WorkerEvents, by the Reactor and Worker. */
enum WorkerEventTypes
{
    INIT,
    
    // sent from Reactor to Worker
    CHANNEL_INIT,
    // sent from Worker to Reactor
    CHANNEL_UP,
    // sent from either Reactor or Worker
    CHANNEL_DOWN,
    // sent from Worker to Reactor
    CHANNEL_READY,
    // sent from Reactor to Worker
    CHANNEL_CLOSE,
    // sent from Worker to Reactor
    CHANNEL_CLOSE_ACK,
    // sent from Worker to Reactor
    FLUSH_DONE,

    WARNING,
    
    // sent from Reactor to Worker
    FD_CHANGE,
    // sent from Reactor to Worker
    FLUSH,
    // sent from Reactor and possibly from Worker (in the case of an error)
    SHUTDOWN,
    // sent from Reactor to Worker
    START_DISPATCH_TIMER, 
    // sent from Worker to Reactor
    TUNNEL_STREAM_DISPATCH_TIMEOUT,
    // sent from Reactor to itself to force a tunnel stream dispatch
    TUNNEL_STREAM_DISPATCH_NOW,
    // sent from Reactor to itself to force a watchlist dispatch
    WATCHLIST_DISPATCH_NOW,
    // sent from Reactor to Worker
    START_WATCHLIST_TIMER,
    // sent from Worker to Reactor
    WATCHLIST_TIMEOUT;
    
    /* Returns a String representation of the specified WorkerEventTypes type. */
    static String toString(WorkerEventTypes type)
    {
        switch (type)
        {
            case INIT:
                return "ReactorChannelEventTypes.INIT";
            case CHANNEL_INIT:
                return "ReactorChannelEventTypes.CHANNEL_INIT";
            case CHANNEL_UP:
                return "ReactorChannelEventTypes.CHANNEL_UP";
            case CHANNEL_DOWN:
                return "ReactorChannelEventTypes.CHANNEL_DOWN";
            case CHANNEL_READY:
                return "ReactorChannelEventTypes.CHANNEL_READY";
            case CHANNEL_CLOSE:
                return "ReactorChannelEventTypes.CHANNEL_CLOSE";
            case CHANNEL_CLOSE_ACK:
                return "ReactorChannelEventTypes.CHANNEL_CLOSE_ACK";
            case WARNING:
                return "ReactorChannelEventTypes.WARNING";
            case FD_CHANGE:
                return "ReactorChannelEventTypes.FD_CHANGE";
            case FLUSH:
                return "ReactorChannelEventTypes.FLUSH";
            case FLUSH_DONE:
                return "ReactorChannelEventTypes.FLUSH_DONE";
            case SHUTDOWN:
                return "ReactorChannelEventTypes.SHUTDOWN";
            case START_DISPATCH_TIMER:
                return "ReactorChannelEventTypes.START_DISPATCH_TIMER";                
            case TUNNEL_STREAM_DISPATCH_TIMEOUT:
                return "ReactorChannelEventTypes.TUNNEL_STREAM_DISPATCH_TIMEOUT";                
            case TUNNEL_STREAM_DISPATCH_NOW:
                return "ReactorChannelEventTypes.TUNNEL_STREAM_DISPATCH_NOW";                
            case WATCHLIST_DISPATCH_NOW:
                return "ReactorChannelEventTypes.WATCHLIST_DISPATCH_NOW";
            case START_WATCHLIST_TIMER:
                return "ReactorChannelEventTypes.START_WATCHLIST_TIMER";
            case WATCHLIST_TIMEOUT:
                return "ReactorChannelEventTypes.WATCHLIST_TIMEOUT";
            default:
                return "ReactorChannelEventTypes " + type + " - undefined.";
        }
    }
}
