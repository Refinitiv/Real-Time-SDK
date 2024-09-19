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
    public long ItemHandle = 0;

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        RefreshMsg refreshMsg = new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name())
            .NameType(EmaRdm.USER_NAME)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Complete(true).Solicited(true);
        providerEvent.Provider.Submit(refreshMsg, providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        FieldList fieldListAttribs = new FieldList();
        fieldListAttribs.AddRmtes(548, new EmaBuffer(new byte[] { 0x54, 0xbb, 0x53, 0xac }));
        fieldListAttribs.AddAscii(254, "SN#12345");

        RefreshMsg refreshMsg = new RefreshMsg()
            .DomainType(EmaRdm.MMT_MARKET_PRICE)
            .Name(reqMsg.Name()).NameType(50)
            .ServiceId(reqMsg.ServiceId())
            .Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Id(60)
            .Filter(70)
            .SeqNum(80)
            .PartNum(90)
            .PublisherId(100, 110)
            .ClearCache(true)
            .PrivateStream(true)
            .DoNotCache(true)
            .Attrib(fieldListAttribs.Complete())
            .Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.JUST_IN_TIME_CONFLATED)
            .ExtendedHeader(new EmaBuffer(new byte[] { 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x45, 0x4e, 0x44 }))
            .ItemGroup(new EmaBuffer(new byte[] { 0x58, 0x59, 60, 77, 77, 77, 77, 77 }))
            .PermissionData(new EmaBuffer(new byte[] { 0x03, 0x01, 0x2c, 0x56, 0x25, 0xc0 }))
            .Payload(fieldList.Complete())
            .Complete(true);

        providerEvent.Provider.Submit(refreshMsg, providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{
    public static void Main(string[] _)
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            //sends 3 messages, 1st - updateMsg, 2nd - statusMsg, 3rd - genericMsg
            for (int i = 1; i <= 4; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.Complete();

                FieldList fieldListAttribs = new FieldList();
                fieldListAttribs.AddRmtes(548, new EmaBuffer(new byte[] { 0x54, 0xbb, 0x53, 0xac, 0xac, 0xac, 0xac }));
                fieldListAttribs.AddAscii(254, "NO#54321");
                fieldListAttribs.Complete();

                UpdateMsg updateMsgShort = new UpdateMsg()
                    .Payload(fieldList);

                UpdateMsg updateMsg = new UpdateMsg()
                    .DomainType(EmaRdm.MMT_MARKET_PRICE)
                    .NameType(50 + i)
                    .Id(60 + i)
                    .Filter(70 + i)
                    .ExtendedHeader(new EmaBuffer(new byte[] { (byte)(0x61 + i), 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x45, 0x4e, 0x44 }))
                    .PermissionData(new EmaBuffer(new byte[] { (byte)(0x03 + i), 0x01, 0x2c, 0x56, 0x25, 0xc0 }))
                    .Attrib(fieldListAttribs)
                    .Payload(fieldList);

                StatusMsg statusMsg = new StatusMsg()
                    .DomainType(EmaRdm.MMT_LOGIN)
                    .Name("dummyName")
                    .NameType(EmaRdm.USER_NAME)
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
                    .Id(20)
                    .Filter(10)
                    .PublisherId(110, 100)
                    .ClearCache(true)
                    .PrivateStream(true)
                    .Attrib(fieldListAttribs)
                    .ExtendedHeader(new EmaBuffer(new byte[] { 10, 20, 30 }))
                    .ItemGroup(new EmaBuffer(new byte[] { 30, 40 }))
                    .PermissionData(new EmaBuffer(new byte[] { 50, 51, 52, 53 }))
                    .ServiceId(10)//?
                    .Payload(fieldList);

                GenericMsg genericMsg = new GenericMsg().DomainType(EmaRdm.MMT_MARKET_PRICE)
                    .Name("genericDummyName")
                    .NameType(EmaRdm.INSTRUMENT_NAME_RIC)
                    .Id(20)
                    .Filter(10)
                    .Attrib(fieldListAttribs)
                    .ExtendedHeader(new EmaBuffer(new byte[] { 10, 20, 30 }))
                    .PermissionData(new EmaBuffer(new byte[] { 50, 51, 52, 53 }))
                    .SeqNum(33)
                    .PartNum(44)
                    .Complete(true)
                    .SecondarySeqNum(77)
                    .ServiceId(10)//?
                    .Payload(fieldList);
                ;

                switch (i)
                {
                    case 1:
                        provider.Submit(updateMsg, appClient.ItemHandle);
                        break;
                    case 2:
                        provider.Submit(genericMsg, appClient.ItemHandle);
                        break;
                    case 3:
                        provider.Submit(statusMsg, appClient.ItemHandle);
                        break;
                    case 4:
                        provider.Submit(updateMsgShort, appClient.ItemHandle);
                        break;
                }

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
