/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    long count = 1;
    public const int APP_DOMAIN = 200;

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case APP_DOMAIN:
                ProcessAppDomainRequest(reqMsg, providerEvent);
                break;
            default:
                Console.WriteLine($"Received invalid Request msg. Item Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
                break;
        }
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
    {
        switch (genericMsg.DomainType())
        {
            case APP_DOMAIN:
                ProcessAppDomainGenericMsg(genericMsg, providerEvent);
                break;
            default:
                Console.WriteLine($"Received invalid Generic msg. Item Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessAppDomainRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            Console.WriteLine($"Received invalid Request msg. Item Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
            return;
        }

        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().DomainType(reqMsg.DomainType())
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessAppDomainGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle == 0 || ItemHandle != providerEvent.Handle)
        {
            Console.WriteLine($"Received invalid Generic msg. Item Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
            return;
        }

        Console.WriteLine($"Received Generic. Item Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");

        ElementList elementList = new ElementList();
        elementList.AddInt("valueFromProvider", ++count);
        providerEvent.Provider.Submit(new GenericMsg().DomainType(APP_DOMAIN).Name("genericMsg").Payload(elementList.Complete()),
            providerEvent.Handle);

        Console.WriteLine();
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
            provider = new OmmProvider(new OmmIProviderConfig().OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.ItemHandle == 0)
            {
                provider.Dispatch(1000);
            }

            FieldList fieldList = new FieldList();
            DateTime endTime = DateTime.Now + TimeSpan.FromSeconds(60);
            int i = 0;

            while (DateTime.Now < endTime)
            {
                provider.Dispatch(1000);

                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i++, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().DomainType(AppClient.APP_DOMAIN).Payload(fieldList.Complete()),
                    appClient.ItemHandle);

                Thread.Sleep(1000);
            }
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
