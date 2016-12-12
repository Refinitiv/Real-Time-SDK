package com.thomsonreuters.upa.test.network.replay;

/**
 * Creates {@link NetworkReplay} instances
 */
public final class NetworkReplayFactory
{
    /**
     * This class is not instantiated
     */
    private NetworkReplayFactory()
    {
        throw new AssertionError();
    }
    
    /**
     * Creates a new implementation of a {@link NetworkReplay}
     * 
     * @return a new implementation of a {@link NetworkReplay}
     */
    public static NetworkReplay create()
    {
        return new NetworkReplayImpl();
    }
}
