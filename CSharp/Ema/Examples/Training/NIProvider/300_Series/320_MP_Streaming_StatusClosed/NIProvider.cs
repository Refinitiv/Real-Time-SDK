/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Directory;
using LSEG.Ema.Rdm;
using System;
using System.Threading;

public class NIProvider
{

    public static void Main()
    {
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            using OmmProvider provider = new OmmProvider(config.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL)
                    .UserName("user"));

            long sourceDirectoryHandle = 1;

            provider.Submit(
                new DirectoryRefreshMsg()
                    .Filter(DirectoryFilters.SERVICE_INFO_FILTER | DirectoryFilters.SERVICE_STATE_FILTER)
                    .ServiceList(sl => sl
                        .Add(new DirectoryService()
                            .ServiceId(2)
                            .Action(DirectoryMapAction.ADD)
                            .Info(i => i
                                .ServiceName("NI_PUB")
                                .CapabilitiesList(cl => cl
                                    .Add(EmaRdm.MMT_MARKET_PRICE)
                                    .Add(EmaRdm.MMT_MARKET_BY_PRICE))
                                .DictionariesUsedList(dul => dul
                                    .Add("RWFFld")
                                    .Add("RWFEnum")))
                            .State(s => s
                                .IsServiceUp(true)))),
                sourceDirectoryHandle);

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
    }
}
