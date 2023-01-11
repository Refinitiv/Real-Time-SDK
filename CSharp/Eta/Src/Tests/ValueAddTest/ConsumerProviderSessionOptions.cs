﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Transports;

namespace LSEG.Eta.ValuedAdd.Tests;

public class ConsumerProviderSessionOptions
{
    /**  Type of connection the session will use. */
    public ConnectionType ConnectionType { get; set; }

    public int ProtocolType { get; set; }

    public string ProtocolList { get; set; }

    public int ReconnectAttemptLimit { get; set; }

    public TimeSpan ReconnectMinDelay { get; set; } = TimeSpan.FromSeconds(1);

    public TimeSpan ReconnectMaxDelay { get; set; } = TimeSpan.FromSeconds(1);

    /** Whether a default login stream will be setup.
	 * If set to true, the consumer's reactor role must have a preset login request. */
    public bool SetupDefaultLoginStream { get; set; }

    /** Whether a default directory stream will be setup.
	 *
	 * If set to true, either the consumer's reactor role must have a preset directory
	 * request, or its watchlist is enabled (or both). */
    public bool SetupDefaultDirectoryStream { get; set; }

    /** Sets whether a second default directory stream will be setup.
	 *
	 * If set to true, either the consumer's reactor role must have a preset directory
	 * request, or its watchlist is enabled (or both). */
    public bool SetupSecondDefaultDirectoryStream { get; set; }

    /*  Number of status events received before the CHANNEL_READY event. Used for
     *  watchlist channel open callback submit status messages. */
    public int NumStatusEvents { get; set; }

    /** pingTimeout the consumer and provider will use. */
    public TimeSpan PingTimeout { get; set; } = TimeSpan.FromSeconds(60);

    public TimeSpan ConsumerChannelInitTimeout { get; set; } = TimeSpan.FromSeconds(60);

    /** openWindow the provider will use on its service. By default, value is -1 and no
     * OpenWindow is defined. */
    public long OpenWindow { get; set; } = -1;

    public int NumOfGuaranteedBuffers { get; set; } = 50;

    public CompressionType CompressionType { get; set; } = CompressionType.NONE;
}