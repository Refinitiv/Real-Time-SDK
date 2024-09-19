/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

class AppClient : IOmmProviderClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("event channel info (refresh)\n" + providerEvent.ChannelInformation() + "\n"
            + refreshMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("event channel info (status)\n" + providerEvent.ChannelInformation() + "\n" + statusMsg);
    }
}

public class NIProvider
{
    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();
            OmmNiProviderConfig config = new OmmNiProviderConfig("EmaConfig.xml");
            ChannelInformation ci = new ChannelInformation();

            provider = new OmmProvider(config.UserName("user"));
            provider.ChannelInformation(ci);
            Console.WriteLine("channel information (niprovider): " + ci);

            provider.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), appClient);

            long itemHandle = 5;

            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.Complete();

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList).Complete(true), itemHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.Complete();

                try
                {
                    provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N").Payload(fieldList), itemHandle);
                }
                catch (OmmException e)
                {
                    // allows one to stop/start the adh and see the status showing channel going down/up
                    Console.WriteLine("submit update message threw exception: " + e.Message);
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
