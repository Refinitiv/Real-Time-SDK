package com.rtsdk.eta.transport;

import com.rtsdk.eta.transport.Protocol;

interface ProtocolInt extends Protocol
{
    Pool getPool(int poolSpec);
}
