package com.thomsonreuters.upa.valueadd.reactor;

/** Guaranteed messaging options for the tunnel stream. */
public class GuaranteedMessagingOptions
{
    String _persistenceFilePath;
    boolean _persistLocally;
    boolean _enableTracing;

    public GuaranteedMessagingOptions()
    {
        _persistLocally = true;
    }
    
    /**
     * Set the path for the guaranteed messaging persistence file.
     * If not specified, the current working directory is used.
     */
    public void persistenceFilePath(String persistenceFilePath)
    {
        _persistenceFilePath = persistenceFilePath;
    }

    /**
     * Returns the persistence file path for guaranteed messaging.
     */
    public String persistenceFilePath()
    {
        return _persistenceFilePath;
    }

    /**
     * Returns whether guaranteed messaging will create a local persistence files.
     */
    public boolean persistLocally()
    {
        return _persistLocally;
    }

    /**
     * Enable or disable local file persistence. Default: Enabled.
     */
    public void persistLocally(boolean persistLocally)
    {
        _persistLocally = persistLocally;
    }
    
    /**
     * Returns whether guaranteed messaging tracing is enabled.
     */
    public boolean enableTracing()
    {
        return _enableTracing;
    }

    /**
     * Enable or disable guaranteed messaging tracing.
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
