/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public const int APP_DOMAIN = 200;

    public long ItemHandle = 0;
    long count = 1;

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            // APIQA
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            // END APIQA
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
            // APIQA
            case EmaRdm.MMT_LOGIN:
                ProcessLoginDomainGenericMsg(genericMsg, providerEvent);
                break;
            case EmaRdm.MMT_DIRECTORY:
                ProcessDirectoryDomainGenericMsg(genericMsg, providerEvent);
                break;
            // END APIQA
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

        Console.WriteLine();
        Console.WriteLine("Clone ReqMsg");
        RequestMsg cloneReqMsg = new RequestMsg(reqMsg);
        Console.WriteLine(cloneReqMsg);
        Console.WriteLine("Clone Req Msg Item Name: " + cloneReqMsg.Name());
        Console.WriteLine();
    }

    // APIQA
    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
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

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    // END APIQA
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

        Console.WriteLine();
        Console.WriteLine("Clone GenericMsg");
        GenericMsg cloneGenericMsg = new GenericMsg(genericMsg);
        Console.WriteLine(cloneGenericMsg);
        Console.WriteLine("Clone Generic Msg Item Name: " + cloneGenericMsg.Name());
        Console.WriteLine();

        ElementList elementList = new ElementList();
        elementList.AddInt("valueFromProvider", ++count);

        providerEvent.Provider.Submit(new GenericMsg().DomainType(APP_DOMAIN).Name("genericMsg").Payload(elementList.Complete()),
            providerEvent.Handle);

        Console.WriteLine();
    }

    // APIQA
    void ProcessLoginDomainGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine($"Received:    GenericMsg. login Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
        Console.WriteLine(genericMsg);

        Console.WriteLine();
        Console.WriteLine("Clone GenericMsg");
        GenericMsg cloneGenericMsg = new GenericMsg(genericMsg);
        Console.WriteLine(cloneGenericMsg);
        Console.WriteLine("Clone Generic Msg Item State: " + cloneGenericMsg.Name());
        Console.WriteLine();

        ElementList elementList = new ElementList();
        elementList.AddInt("valueFromProvider", 3);

        providerEvent.Provider.Submit(new GenericMsg().DomainType(EmaRdm.MMT_LOGIN).Name("genericMsgInGeneral").Payload(elementList.Complete()),
            providerEvent.Handle);

        Console.WriteLine();
    }

    void ProcessDirectoryDomainGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine($"Received:    GenericMsg. Dir Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
        Console.WriteLine(genericMsg);

        ElementList elementList = new ElementList();
        elementList.AddInt("valueFromProvider", 3);

        providerEvent.Provider.Submit(new GenericMsg().DomainType(EmaRdm.MMT_DIRECTORY).Name("genericMsgInGeneral").Payload(elementList.Complete()),
            providerEvent.Handle);

        Console.WriteLine();
    }
    // END APIQA
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
            int i = 0;
            var endTime = DateTime.Now + TimeSpan.FromMilliseconds(60_000);

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
//END APIQA
