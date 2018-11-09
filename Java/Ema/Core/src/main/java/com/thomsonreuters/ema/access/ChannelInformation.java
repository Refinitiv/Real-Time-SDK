///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;

/**
 * The ChannelInformation interface provides channel information to an application
 *
 * For IProvider applications, this channel information is about channels used by
 * clients to connect to the IProvider application.
 *
 * For Consumer and NiProvider applications, this channel information is about the
 * out bound channel (e.g., the channel used by a Consumer application to connect to
 * an ADS) used to connect for receiving data (Consumer) or sending data (NiProvider).
 *
 * Examples of ChannelInformation usage are found in the examples:
 * (Consumer) Consumer/100_Series/170__MarketPrice__ChannelInfo
 * (IProvider) IProvider/100_Series/170__MarketPrice__ConnectedClientInfo
 * (NiProvider) NiProvider/100_Series/170__MarketPrice__ChannelInfo
 */
public interface ChannelInformation
{
	/** Clears the ChannelInformation
	 *  <p>invoking clear() resets all member variables to their default values</p>
	 */
	public void clear();

	/** Sets the ChannelInformation for a reactorChannel
	 *
	 * @param reactorChannel reactor channel
	 */
	public void set(ReactorChannel reactorChannel);

	/**
	 *  converts the ChannelInformation to a string.
	 *
	 * @return the string
	 */
	public String toString();

	/**
	 *  returns the connected component information as a string.
	 *
	 * @return the component information
	 */
	public String componentInformation();

	/**
	 *  returns the host name as a string.
	 *
	 * @return the string host name 
	 */
	public String hostname();

	/**
	 *  returns the IP address as a string.
	 *
	 * @return the string IP address
	 */
	public String ipAddress();

	/**
	 *  returns the connection type from the ConnectionTypes class.
	 *
	 * @return the connection type
	 */
	public int connectionType();

	/**
	 *  returns the reactor channel state from the ChannelState class.
	 *
	 * @return the reactor channel state
	 */
	public int channelState();

	/**
	 *  return the protocol type.
	 *
	 * @return the protocolType
	 */
	public int protocolType();

	/**
	 *  return the major version.
	 *
	 * @return the majorVersion
	 */
	public int majorVersion();

	/**
	 *  return the minor version.
	 *
	 * @return the minorVersion
	 */
	public int minorVersion();

	/**
	 *  return the pingTimeout.
	 *
	 * @return the ping timeout
	 */
	public int pingTimeout();

	/** set host name
	 *
	 * @param hostname is the host name associated with the channel
	 */
	public void hostname(String hostname);

	/** set ipAddress
	 *
	 * @param ipAddress is the ipAddress associated with the channel
	 */
	public void ipAddress(String ipAddress);

	/** set component info
	 *
	 * @param componentInfo is the component information associated with the channel
	 *
	 */
	public void componentInfo(String componentInfo);

	/** set channel state
	 *
	 * @param channelState is the state associated with the channel
	 *
	 */
	public void channelState(int channelState);

	/** set connection type
	 *
	 * @param connectionType is the connection type associated with the channel
	 *
	 */
	public void connectionType(int connectionType);

	/** set protocol type
	 *
	 * @param protocolType is the protocol type associated with the channel
	 *
	 */
	public void protocolType(int protocolType);

	/** The major version of the channel
	 *
	 * @param majorVersion is the major version associated with the channel
	 *
	 */
	public void majorVersion(int majorVersion);

	/** The minor version of the channel
	 *
	 * @param minorVersion is the minor version associated with the channel
	 *
	 */
	public void minorVersion(int minorVersion);

	/** set ping timeout
	 *
	 * @param pingTimeout is the ping timeout associated with the channel
	 *
	 */
	public void pingTimeout(int pingTimeout);
}
