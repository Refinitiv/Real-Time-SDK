/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidDomainRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (!"user".Equals(reqMsg.Name()))
        {
            providerEvent.Provider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_LOGIN)
                .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
                .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_AUTHORIZED, "Login denied"),
                providerEvent.Handle);
        }
        else
        {
            RefreshMsg loginRefresh = new RefreshMsg();
            ElementList refreshAttributes = new ElementList();

            if (reqMsg.Attrib().DataType == DataType.DataTypes.ELEMENT_LIST)
            {
                bool setRefreshAttrib = false;
                ElementList reqAttributes = reqMsg.Attrib().ElementList();

                foreach (ElementEntry reqAttrib in reqAttributes)
                {
                    string name = reqAttrib.Name;

                    if (name.Equals(EmaRdm.ENAME_ALLOW_SUSPECT_DATA)
                        || name.Equals(EmaRdm.ENAME_SINGLE_OPEN))
                    {
                        setRefreshAttrib = true;
                        refreshAttributes.AddUInt(name, reqAttrib.UIntValue());
                    }
                    else if (name.Equals(EmaRdm.ENAME_APP_ID)
                        || name.Equals(EmaRdm.ENAME_POSITION))
                    {
                        setRefreshAttrib = true;
                        refreshAttributes.AddAscii(name, reqAttrib.OmmAsciiValue().ToString());
                    }
                }

                if (setRefreshAttrib)
                    loginRefresh.Attrib(refreshAttributes.Complete());
            }

            providerEvent.Provider.Submit(loginRefresh.DomainType(EmaRdm.MMT_LOGIN)
                .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
                .Complete(true).Solicited(true)
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
                providerEvent.Handle);
        }
    }

    void ProcessInvalidDomainRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg()
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Domain not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);

            Thread.Sleep(60_000);
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            provider?.Uninitialize();
        }
    }
}
