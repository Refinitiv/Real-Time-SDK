package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.codec.Codec;

class IpcProtocolManager
{
    private int _startingVersion = Ripc.ConnectionVersions.VERSION14;
    private IpcProtocol _ripc11Protocol = new Ripc11Protocol();
    private IpcProtocol _ripc12Protocol = new Ripc12Protocol();
    private IpcProtocol _ripc13Protocol = new Ripc13Protocol();
    private IpcProtocol _ripc14Protocol = new Ripc14Protocol();

    IpcProtocol nextProtocol(Channel channel, IpcProtocol currentProtocol, ConnectOptions connectOptions)
    {
        IpcProtocol retProtocol = null;

        if (_startingVersion == Ripc.ConnectionVersions.VERSION14)
        {
            if (currentProtocol == null)
                retProtocol = _ripc14Protocol;
            else if (currentProtocol.connectionVersion() == Ripc.ConnectionVersions.VERSION14)
            {
                /* If we are rolling back beyond version 14, we need to unset key exchange */
                currentProtocol._protocolOptions._keyExchange = false;
                retProtocol = _ripc13Protocol;
            }
            else if (currentProtocol.connectionVersion() == Ripc.ConnectionVersions.VERSION13)
                retProtocol = _ripc12Protocol;
            else if (currentProtocol.connectionVersion() == Ripc.ConnectionVersions.VERSION12
                    && connectOptions.protocolType() == Codec.protocolType())
                // apps using version 11 shouldn't be using anything others than RWF
                retProtocol = _ripc11Protocol;
        }
        else if (_startingVersion == Ripc.ConnectionVersions.VERSION13)
        {
            if (currentProtocol == null)
                retProtocol = _ripc13Protocol;
            else if (currentProtocol.connectionVersion() == Ripc.ConnectionVersions.VERSION13)
                retProtocol = _ripc12Protocol;
            else if (currentProtocol.connectionVersion() == Ripc.ConnectionVersions.VERSION12
                    && connectOptions.protocolType() == Codec.protocolType())
                // apps using version 11 shouldn't be using anything others than RWF
                retProtocol = _ripc11Protocol;
        }
        // for JUnit testing so method returns something lower than Ripc13Protocol()
        else if (_startingVersion == Ripc.ConnectionVersions.VERSION12)
        {
            if (currentProtocol == null)
                retProtocol = _ripc12Protocol;
            else if (currentProtocol.connectionVersion() == Ripc.ConnectionVersions.VERSION12
                    && connectOptions.protocolType() == Codec.protocolType())
                // apps using version 11 shouldn't be using anything others than RWF
                retProtocol = _ripc11Protocol;
        }
        else if (_startingVersion == Ripc.ConnectionVersions.VERSION11)
        {
            if (currentProtocol == null)
                retProtocol = _ripc11Protocol;
        }

        if (retProtocol != null)
            retProtocol.channel(channel);

        return retProtocol;
    }
    
    IpcProtocol determineProtocol(Channel channel, int connectionVersion)
    {
        IpcProtocol retProtocol = null;

        if (connectionVersion == Ripc.ConnectionVersions.VERSION14)
            retProtocol = _ripc14Protocol;
        else if (connectionVersion == Ripc.ConnectionVersions.VERSION13)
            retProtocol = _ripc13Protocol;
        else if (connectionVersion == Ripc.ConnectionVersions.VERSION12)
            retProtocol = _ripc12Protocol;
        else if (connectionVersion == Ripc.ConnectionVersions.VERSION11)
            retProtocol = _ripc11Protocol;

        if (retProtocol != null)
            retProtocol.channel(channel);

        return retProtocol;
    }

    // for JUnit testing
    void startingVersion(int version)
    {
        _startingVersion = version;
    }
}
