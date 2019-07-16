package com.thomsonreuters.upa.valueadd.reactor;

/**
 * Storage object for accessing ReactorChannel statistics. 
 * Used in conjunction with {@link ReactorChannel#getReactorChannelStats(ReactorChannelStats, ReactorErrorInfo)}
 * 
 * @see ReactorChannel
 */
public interface ReactorChannelStats {
	
	/**
	 * Accessor for number of ping messages received 
	 * 
	 * @return the number of pings received
	 */
	public int pingsReceived();
	
	/**
	 * Accessor for number of ping messages sent.
	 * 
	 * @return the number of pings sent
	 */
	public int pingsSent();
	
	/**
	 * Accessor for number of bytes written.
	 * 
	 * @return the number of bytes written
	 */
	public int bytesWritten();
	
	/**
	 * Accessor for number of uncompressed bytes written.
	 * 
	 * @return the number of uncompressed bytes written
	 */
	public int uncompressedBytesWritten();
	
	/**
	 * Accessor for number of bytes read
	 * 
	 * @return the number of bytes read
	 */
	public int bytesRead();
	
	/**
	 * Accessor for number of uncompressed bytes read.
	 * 
	 * @return the number of uncompressed bytes read
	 */
	public int uncompressedBytesRead();
	
	/**
	 * Mutator for number of ping messages received.
	 * 
	 * @param number of ping messages received
	 */
	public void pingsReceived(int pingsReceived);
	
	/**
	 * Mutator for number of ping messages sent.
	 * 
	 * @param the number of ping messages sent
	 */
	public void pingsSent(int pingsSent);
	
	/**
	 * Mutator for number of bytes written.
	 * 
	 * @param the number of bytes written
	 */
	public void bytesWritten(int bytesWritten);
	
	/**
	 * Mutator for number of uncompressed bytes written.
	 * 
	 * @param the number of uncompressed bytes written
	 */
	public void uncompressedBytesWritten(int uncompressedBytesWritten);
	
	/**
	 * Mutator for number of bytes read
	 * 
	 * @param the number of bytes read
	 */
	public void bytesRead(int bytesRead);
	
	/**
	 * Mutator for number of uncompressed bytes read.
	 * 
	 * @param the number of uncompressed bytes read
	 */
	public void uncompressedBytesRead(int uncompressedBytesRead);
	
	/**
	 * Reset all member variables to 0
	 * 
	 */
	public void clear();
}
