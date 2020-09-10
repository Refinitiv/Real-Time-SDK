package com.rtsdk.eta.valueadd.reactor;

/** Guaranteed messaging options for the tunnel stream. */
public class GuaranteedMessagingOptions
{
    String _persistenceFilePath;
    boolean _persistLocally;
    boolean _enableTracing;

    /**
     * Instantiates a new guaranteed messaging options.
     */
    public GuaranteedMessagingOptions()
    {
        _persistLocally = true;
    }
    
    /**
     * Set the path for the guaranteed messaging persistence file.
     * If not specified, the current working directory is used.
     *
     * @param persistenceFilePath the persistence file path
     */
    public void persistenceFilePath(String persistenceFilePath)
    {
        _persistenceFilePath = persistenceFilePath;
    }

    /**
     * Returns the persistence file path for guaranteed messaging.
     *
     * @return the string
     */
    public String persistenceFilePath()
    {
        return _persistenceFilePath;
    }

    /**
     * Returns whether guaranteed messaging will create a local persistence files.
     *
     * @return true, if successful
     */
    public boolean persistLocally()
    {
        return _persistLocally;
    }

    /**
     * Enable or disable local file persistence. Default: Enabled.
     *
     * @param persistLocally the persist locally
     */
    public void persistLocally(boolean persistLocally)
    {
        _persistLocally = persistLocally;
    }
    
    /**
     * Returns whether guaranteed messaging tracing is enabled.
     *
     * @return true, if successful
     */
    public boolean enableTracing()
    {
        return _enableTracing;
    }

    /**
     * Enable or disable guaranteed messaging tracing.
     *
     * @param enableTracing the enable tracing
     */
    public void enableTracing(boolean enableTracing)
    {
        _enableTracing = enableTracing;
    }
    
    /**
     * Clears the object for re-use.
     */
    public void clear()
    {
        _persistenceFilePath = null;
        _persistLocally = true;
        _enableTracing = false;
    }
}
