package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.Protocol;

interface ProtocolInt extends Protocol
{
    Pool getPool(int poolSpec);
}
