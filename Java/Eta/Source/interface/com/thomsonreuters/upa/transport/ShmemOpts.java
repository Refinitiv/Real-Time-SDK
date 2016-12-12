package com.thomsonreuters.upa.transport;

/**
 * Options used for configuring shared memory specific transport options
 * ({@link ConnectionTypes#UNIDIR_SHMEM}).
 * 
 * @see ConnectOptions
 */
public interface ShmemOpts
{
    /**
     * Maximum number of messages that the client can have waiting to read.
     * If the client "lags" the server by more than this amount, UPA will
     * disconnect the client.
     * 
     * @param maxReaderLag the maxReaderLag to set
     */
    public void maxReaderLag(long maxReaderLag);

    /**
     * Maximum number of messages that the client can have waiting to read.
     * If the client "lags" the server by more than this amount, UPA will
     * disconnect the client.
     * 
     * @return the maxReaderLag
     */
    public long maxReaderLag();
}
