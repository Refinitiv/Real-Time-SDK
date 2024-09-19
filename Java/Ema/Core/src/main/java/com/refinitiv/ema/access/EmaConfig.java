///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------


package com.refinitiv.ema.access;

/** This class contains enumerations and other configuration values used in the EMA configuration classes.
 */
public class EmaConfig
{
	 private EmaConfig()
     {
         throw new AssertionError();
     }
	
    /**
     * This enumeration defines the connection types available for the Channel ConnectionType configuration elements.
     */
    public static class ConnectionType
    {
        private ConnectionType()
        {
            throw new AssertionError();
        }

        /**
         * This connection type uses the TCP-based ETA Socket connection protocol.
         */
        public static final int SOCKET = 0;

        /**
         * This connection type uses a TLS encrypted connection.  Additional protocol configuration is defined in EncryptedProtocolTypeEnum.
         */
        public static final int ENCRYPTED = 1;

        /**
         * This connection type uses HTTP TCP-based socket connection protocol.
         */
        public static final int HTTP = 2;

        /**
         * This connection type uses TCP-based WebSocket connection protocol.
         */
        public static final int WEBSOCKET = 7;
    }

    /**
     * This enumeration defines the protocol type available for ENCRYPTED ConnectionType.
     */
    public static class EncryptedProtocolType
    {
        private EncryptedProtocolType()
        {
            throw new AssertionError();
        }
        /**
         *  This encrypted protocol type uses the TCP-based ETA Socket encrypted connection protocol.
         */
        public static final int SOCKET = 0;

        /**
         * This encrypted protocol type uses HTTP TCP-based socket encrypted connection protocol.
         */
        public static final int HTTP = 2;

        /**
         * This encrypted protocol type uses TCP-based WebSocket encrypted connection protocol.
         */
        public static final int WEBSOCKET = 7;
    }
}
