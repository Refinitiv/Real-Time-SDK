/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.AcceptOptionsImpl;
import com.refinitiv.eta.transport.BindOptionsImpl;
import com.refinitiv.eta.transport.ChannelInfoImpl;
import com.refinitiv.eta.transport.ConnectOptionsImpl;
import com.refinitiv.eta.transport.ErrorImpl;
import com.refinitiv.eta.transport.InProgInfoImpl;
import com.refinitiv.eta.transport.InitArgsImpl;
import com.refinitiv.eta.transport.ReadArgsImpl;
import com.refinitiv.eta.transport.ServerInfoImpl;
import com.refinitiv.eta.transport.WriteArgsImpl;

/** Factory for Transport package objects */
public class TransportFactory
{
    // TransportFactory class cannot be instantiated
    private TransportFactory()
    {
        throw new AssertionError();
    }

    /**
     * Creates {@link AcceptOptions}.
     * 
     * @return {@link AcceptOptions} object
     * 
     * @see AcceptOptions
     */
    public static AcceptOptions createAcceptOptions()
    {
        return new AcceptOptionsImpl();
    }

    /**
     * Creates {@link BindOptions}.
     * 
     * @return {@link BindOptions} object
     * 
     * @see BindOptions
     */
    public static BindOptions createBindOptions()
    {
        return new BindOptionsImpl();
    }

    /**
     * Creates {@link WriteArgs}.
     * 
     * @return {@link WriteArgs} object
     * 
     * @see WriteArgs
     */
    public static WriteArgs createWriteArgs()
    {
        return new WriteArgsImpl();
    }

    /**
     * Creates {@link ChannelInfo}.
     * 
     * @return {@link ChannelInfo} object
     * 
     * @see ChannelInfo
     */
    public static ChannelInfo createChannelInfo()
    {
        return new ChannelInfoImpl();
    }

    /**
     * Creates {@link ConnectOptions}.
     * 
     * @return {@link ConnectOptions} object
     * 
     * @see ConnectOptions
     */
    public static ConnectOptions createConnectOptions()
    {
        return new ConnectOptionsImpl();
    }

    /**
     * Creates {@link Error}.
     * 
     * @return {@link Error} object
     * 
     * @see Error
     */
    public static Error createError()
    {
        return new ErrorImpl();
    }

    /**
     * Creates {@link InProgInfo}.
     * 
     * @return {@link InProgInfo} object
     * 
     * @see InProgInfo
     */
    public static InProgInfo createInProgInfo()
    {
        return new InProgInfoImpl();
    }

    /**
     * Creates {@link ReadArgs}.
     * 
     * @return {@link ReadArgs} object
     * 
     * @see ReadArgs
     */
    public static ReadArgs createReadArgs()
    {
        return new ReadArgsImpl();
    }

    /**
     * Creates {@link ServerInfo}.
     * 
     * @return {@link ServerInfo} object
     * 
     * @see ServerInfo
     */
    public static ServerInfo createServerInfo()
    {
        return new ServerInfoImpl();
    }

    /**
     * Creates {@link InitArgs}.
     * 
     * @return {@link InitArgs} object
     * 
     * @see InitArgs
     */
    public static InitArgs createInitArgs()
    {
        return new InitArgsImpl();
    }
    
    /**
     * Creates {@link ComponentInfo}.
     * 
     * @return {@link ComponentInfo} object
     * 
     * @see ComponentInfo
     */
    public static ComponentInfo createComponentInfo()
    {
        return new ComponentInfoImpl();
    }
    
    /**
     * Creates {@link MCastStats}.
     * 
     * @return {@link MCastStats} object
     * 
     * @see MCastStats
     */
    public static MCastStats createMCastStats()
    {
        return new MCastStatsImpl();
    }
    
    /**
     * Creates {@link EncryptDecryptHelpers}.
     * 
     * @return {@link EncryptDecryptHelpers} object
     * 
     * @see EncryptDecryptHelpers
     */
    public static EncryptDecryptHelpers createEncryptDecryptHelpers()
    {
        return new EncryptionDecryptionSL164Impl();
    }
    
}
