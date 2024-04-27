/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using System;
using System.Threading;

using LSEG.Ema.Access;

public class NIProvider
{
    static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            provider = new OmmProvider(new OmmNiProviderConfig().UserName("user"));
            long itemHandle = 5;

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(new FieldList()
                    .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 0, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .Complete())
                .Complete(true), itemHandle);


            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N")
                    .Payload(new FieldList()
                        .AddReal(22, 3391 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                        .AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0)
                        .Complete()), itemHandle);
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