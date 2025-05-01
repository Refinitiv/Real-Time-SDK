/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.AcceptOptions;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.Server;

public interface Protocol 
{
	Channel channel(ConnectOptions opts, Error error);
	Server server(BindOptions opts, Error error);
	Channel channel(AcceptOptions options, Server server, Object object, Error error);
	void uninitialize();
}
