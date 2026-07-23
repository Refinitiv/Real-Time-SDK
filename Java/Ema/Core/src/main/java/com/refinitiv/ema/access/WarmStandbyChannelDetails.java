/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelDetails;

import static com.refinitiv.ema.access.ChannelInformation.*;

/**
 * Represents connection details for a single warm standby channel reported by EMA.
 * <p>
 * Instances of this class are populated from the underlying ETA warm standby event and
 * describe the remote endpoint, transport characteristics, and any user-specified object
 * associated with that connection.
 *
 * @see ReactorWarmStandbyChannelDetails
 * @see WarmStandbyLoginBasedChannelInformation
 * @see WarmStandbyServiceBasedChannelInformation
 */

public final class WarmStandbyChannelDetails
{
    private final String hostname;
    private final String channelName;
    private final int port;
    private final int connectionType;
    private final int encryptedConnectionType;
    private final int protocolType;
    private final Object userSpecObject;

    private final StringBuilder stringBuilder = new StringBuilder();

    private WarmStandbyChannelDetails(String hostname, int port, int connectionType, int encryptedConnectionType,
                                      int protocolType, Object userSpecObject)
    {
        this.hostname = hostname;
        this.port = port;
        this.connectionType = connectionType;
        this.encryptedConnectionType = encryptedConnectionType;
        this.protocolType = protocolType;
        this.userSpecObject = userSpecObject;
        if (userSpecObject instanceof ChannelInfo)
        {
            this.channelName = ((ChannelInfo) userSpecObject).name();
        }
        else
        {
            this.channelName = null;
        }

    }

    static WarmStandbyChannelDetails create(ReactorWarmStandbyChannelDetails details)
    {
        WarmStandbyChannelDetails channelDetails = null;
        if (details != null)
        {
            channelDetails = new WarmStandbyChannelDetails(details.hostname(), details.port(), details.connectionType(),
                    details.encryptedConnectionType(), details.protocolType(), details.userSpecObject());
        }

        return channelDetails;
    }

    /**
     * Returns the remote server host name associated with this warm standby channel.
     * <p>
     * The returned value is copied from the underlying transport channel details and may be
     * unavailable for some connection types or states.
     *
     * @return the remote host name, or {@code null} if unavailable
     */
    public String hostname()
    {
        return hostname;
    }

    /**
     * Returns the configured EMA channel name associated with this warm standby channel.
     * <p>
     * It typically corresponds to the configured channel name used by EMA,
     * rather than the remote host name reported by the transport.
     *
     * @return the configured channel name, or {@code null} if unavailable
     */
    public String channelName()
    {
        return channelName;
    }

    /**
     * Returns the remote server port number associated with this warm standby channel.
     * <p>
     * This value is typically meaningful for socket-based connections. For connection types
     * where a remote TCP port is not applicable, the value may be {@code 0}.
     *
     * @return the remote port number, or {@code 0} if not applicable
     */
    public int port()
    {
        return port;
    }

    /**
     * Returns the transport connection type associated with this warm standby channel.
     * <p>
     * The value corresponds to the underlying ETA transport connection type.
     *
     * @return the connection type
     * @see ConnectionType
     */
    public int connectionType()
    {
        return connectionType;
    }

    /**
     * Returns the encrypted transport connection type associated with this warm standby channel.
     * <p>
     * The value corresponds to the underlying ETA transport encrypted connection type.
     *
     * @return the encrypted connection type when {@link #connectionType()} is {@link ConnectionType#ENCRYPTED},
     * otherwise returns {@link ConnectionType#UNIDENTIFIED}
     *
     * @see ConnectionType
     */
    public int encryptedConnectionType()
    {
        return encryptedConnectionType;
    }

    /**
     * Returns the protocol type associated with this warm standby channel.
     * <p>
     * This identifies the underlying transport protocol or sub-protocol in use for the
     * connection.
     *
     * @return the protocol type for the connection
     * @see ProtocolType
     */
    public int protocolType()
    {
        return protocolType;
    }

    /**
     * Returns the user-specified object associated with this warm standby channel.
     * <p>
     * This value is carried from the underlying connection or connection options and is not
     * modified by the transport layer.
     *
     * @return the user-specified object, or {@code null} if none was set
     */
    Object userSpecObject()
    {
        return userSpecObject;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);

        stringBuilder.append('{')
                .append("channelName='")
                .append(channelName() == null ? "N/A" : channelName())
                .append('\'')
                .append(", hostName='")
                .append(hostname() == null ? "N/A" : hostname())
                .append('\'')
                .append(", port=")
                .append(port())
                .append(", connectionType='")
                .append(ConnectionTypes.toString(connectionType()))
                .append('\'')
                .append(", encryptedConnectionType='")
                .append(encryptedConnectionType() == ConnectionType.UNIDENTIFIED ?
                        "N/A" : ConnectionTypes.toString(encryptedConnectionType()))
                .append('\'')
                .append(", protocolType=")
                .append(protocolType())
                .append('}');

        return stringBuilder.toString();
    }
}
