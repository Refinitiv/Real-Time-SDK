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
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Attrib(new ElementList().Complete()),
            providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        FieldList fieldList = new FieldList();

        fieldList.AddAscii(3, reqMsg.Name());
        fieldList.AddEnumValue(15, 840);
        fieldList.AddReal(21, 3900, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().ServiceName(reqMsg.ServiceName()).Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Solicited(true).Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .DomainType(reqMsg.DomainType())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }

}

public class IProvider
{

    //APIQA
    static Map CreateProgrammaticConfig(string dictConfigPath)
    {
        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        innerElementList.AddEnum("DictionaryType", EmaConfig.DictionaryTypeEnum.FILE);
        innerElementList.AddAscii("RdmFieldDictionaryFileName", dictConfigPath + "/RDMFieldDictionary");
        innerElementList.AddAscii("EnumTypeDefFileName", dictConfigPath + "/enumtype.def");
        innerElementList.AddAscii("RdmFieldDictionaryItemName", "RWFFld3");
        innerElementList.AddAscii("EnumTypeDefItemName", "RWFEnum3");

        innerMap.AddKeyAscii("Dictionary_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("DictionaryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerMap.AddKeyAscii("Logger_1", MapAction.ADD,
            new ElementList()
                .AddEnum("LoggerType", EmaConfig.LoggerTypeEnum.STDOUT)
                .AddAscii("FileName", "logFile")
                .AddEnum("LoggerSeverity", EmaConfig.LoggerLevelEnum.INFO)
                .Complete());

        elementList.AddMap("LoggerList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("LoggerGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        return configMap.Complete();
    }

    //END APIQA
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;

        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();
            UpdateMsg updateMsg = new UpdateMsg();

            string dictConfigPath;

            if (args.Length > 0 && args[0] != null)
            {
                dictConfigPath = args[0];
            }
            else
            {
                dictConfigPath = ".";
            }

            provider = new OmmProvider(new OmmIProviderConfig().Config(CreateProgrammaticConfig(dictConfigPath))
                .OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.ItemHandle == 0)
            {
                provider.Dispatch(1000);
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                provider.Dispatch(1000);

                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(25, 3994 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(updateMsg.Clear().Payload(fieldList.Complete()), appClient.ItemHandle);

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
