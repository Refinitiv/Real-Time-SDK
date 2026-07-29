/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ConnectionTypes;

import java.util.Objects;

/**
 * Represents ETA channel information for warm standby events.
 *
 * @see Channel
 */
public final class ReactorWarmStandbyChannelDetails
{
    private final String hostname;
    private final int port;
    private final int connectionType;
    private final int encryptedConnectionType;
    private final int protocolType;
    private final Object userSpecObject;

    private final StringBuilder stringBuilder = new StringBuilder();

    private ReactorWarmStandbyChannelDetails(String hostname, int port, int connectionType, int encryptedConnectionType,
                                             int protocolType, Object userSpecObject)
    {
        this.hostname = hostname;
        this.port = port;
        this.connectionType = connectionType;
        this.encryptedConnectionType = encryptedConnectionType;
        this.protocolType = protocolType;
        this.userSpecObject = userSpecObject;
    }

    static ReactorWarmStandbyChannelDetails create(Channel channel)
    {
        ReactorWarmStandbyChannelDetails channelDetails = null;
        if (channel != null)
        {
            channelDetails = new ReactorWarmStandbyChannelDetails(channel.hostname(), channel.port(),
                    channel.connectionType(), channel.encryptedConnectionType(), channel.protocolType(),
                    channel.userSpecObject());
        }

        return channelDetails;
    }

    /**
     * Remote server host name associated with the {@link Channel}.
     *
     * @return the host name
     */
    public String hostname()
    {
        return hostname;
    }

    /**
     * Remote server port number associated with the {@link Channel}.
     * Relevant for Socket connection, zero for other
     *
     * @return the port number
     */
    public int port()
    {
        return port;
    }

    /**
     * The connection type associated with the {@link Channel}.
     *
     * @return the connection type
     *
     * @see ConnectionTypes
     */
    public int connectionType()
    {
        return connectionType;
    }

    /**
     * The encrypted connection type associated with the {@link Channel}.
     * This method is used when {@link #connectionType()} is {@link ConnectionTypes#ENCRYPTED}
     *
     * @return the encrypted connection type when {@link #connectionType()} is {@link ConnectionTypes#ENCRYPTED},
     * otherwise returns -1
     *
     * @see ConnectionTypes
     */
    public int encryptedConnectionType()
    {
        return encryptedConnectionType;
    }

    /**
     * The protocol type associated with the {@link Channel}.
     *
     * @return the protocolType
     */
    public int protocolType()
    {
        return protocolType;
    }

    /**
     * A user specified object, possibly a closure.This value can be set
     * directly or via the connection options and is not modified by the
     * transport. This information can be useful for coupling this
     * {@link Channel} with other user created information, such as a watch list
     * associated with this connection.
     *
     * @return the userSpecObject
     */
    public Object userSpecObject()
    {
        return userSpecObject;
    }

    @Override
    public boolean equals(Object o)
    {
        if (this == o)
            return true;
        if (!(o instanceof ReactorWarmStandbyChannelDetails))
            return false;
        ReactorWarmStandbyChannelDetails that = (ReactorWarmStandbyChannelDetails) o;

        return port == that.port && connectionType == that.connectionType
                && encryptedConnectionType == that.encryptedConnectionType && protocolType == that.protocolType
                && Objects.equals(hostname, that.hostname) && Objects.equals(userSpecObject, that.userSpecObject);
    }

    @Override
    public int hashCode() {
        return Objects.hash(hostname, port, connectionType, encryptedConnectionType, protocolType, userSpecObject);
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);

        stringBuilder.append('{')
                .append("hostName='")
                .append(hostname() == null ? "N/A" : hostname())
                .append('\'')
                .append(", port=")
                .append(port())
                .append(", connectionType='")
                .append(ConnectionTypes.toString(connectionType()))
                .append('\'')
                .append(", encryptedConnectionType='")
                .append(encryptedConnectionType() == -1 ? "N/A" : ConnectionTypes.toString(encryptedConnectionType()))
                .append('\'')
                .append(", protocolType=")
                .append(protocolType())
                .append('}');

        return stringBuilder.toString();
    }
}
