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
using LSEG.Ema.Domain.Login;
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
        LoginReq loginRequest = new LoginReq(reqMsg);

        if (!"user".Equals(loginRequest.Name()))
        {
            LoginStatus loginStatus = new LoginStatus();

            if (loginRequest.HasNameType)
                loginStatus.NameType(loginRequest.NameType());

            providerEvent.Provider.Submit(loginStatus.Name(loginRequest.Name())
                .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_AUTHORIZED, "Login denied")
                .Message(),
                providerEvent.Handle);
        }
        else
        {
            LoginRefresh loginRefresh = new LoginRefresh();

            if (loginRequest.HasAllowSuspectData)
                loginRefresh.AllowSuspectData(loginRequest.AllowSuspectData());

            if (loginRequest.HasSingleOpen)
                loginRefresh.SingleOpen(loginRequest.SingleOpen());

            if (loginRequest.HasPosition)
                loginRefresh.Position(loginRequest.Position());

            if (loginRequest.HasApplicationId)
                loginRefresh.ApplicationId(loginRequest.ApplicationId());

            if (loginRequest.HasNameType)
                loginRefresh.NameType(loginRequest.NameType());

            providerEvent.Provider.Submit(loginRefresh.Name(loginRequest.Name())
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
                .Message(),
                providerEvent.Handle);
        }
    }

    void ProcessInvalidDomainRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        LoginStatus loginStatus = new LoginStatus();

        loginStatus.Name(reqMsg.Name());

        if (reqMsg.HasNameType)
            loginStatus.NameType(reqMsg.NameType());

        loginStatus.State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_AUTHORIZED, "Invalid domain");

        providerEvent.Provider.Submit(loginStatus.Message(), providerEvent.Handle);
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
