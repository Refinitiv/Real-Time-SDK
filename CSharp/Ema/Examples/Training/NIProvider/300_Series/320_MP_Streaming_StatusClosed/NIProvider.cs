/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;

public class NIProvider
{

    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            provider = new OmmProvider(config.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL)
                    .UserName("user"));

            long sourceDirectoryHandle = 1;

            OmmArray capablities = new OmmArray();
            capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
            capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
            OmmArray dictionaryUsed = new OmmArray();
            dictionaryUsed.AddAscii("RWFFld");
            dictionaryUsed.AddAscii("RWFEnum");

            ElementList serviceInfoId = new ElementList();

            serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "NI_PUB");
            serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities.Complete());
            serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed.Complete());

            ElementList serviceStateId = new ElementList();
            serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);

            FilterList filterList = new FilterList();
            filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId.Complete());
            filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId.Complete());

            Map map = new Map();
            map.AddKeyUInt(2, MapAction.ADD, filterList.Complete());

            RefreshMsg refreshMsg = new RefreshMsg();
            provider.Submit(refreshMsg.DomainType(EmaRdm.MMT_DIRECTORY)
                .Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).Payload(map.Complete()), sourceDirectoryHandle);

            long itemHandle = 5;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), itemHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 20; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N").Payload(fieldList.Complete()), itemHandle);
                Thread.Sleep(1000);
            }

            provider.Submit(new StatusMsg().ServiceName("NI_PUB").Name("IBM.N").State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT,
                    OmmState.StatusCodes.NONE, "Stream Closed"), itemHandle);
            Thread.Sleep(20000);
        }
        catch (Exception excp)
		{
            Console.WriteLine(excp.Message);
        }

        finally
        {
            provider?.Uninitialize();
        }
    }
}
