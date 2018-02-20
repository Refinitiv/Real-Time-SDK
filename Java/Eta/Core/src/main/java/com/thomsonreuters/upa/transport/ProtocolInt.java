package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.Protocol;

interface ProtocolInt extends Protocol
{
    Pool getPool(int poolSpec);
}
