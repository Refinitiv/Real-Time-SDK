/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.NIProvider;

public class NIProvider {

	public static void Main(string[] args)
	{
		OmmProvider? provider = null;

		try
		{
		    //APIQA
			// NIProvider only publishes for 2 seconds
            for (int i = 0; i < 1_000_000; i++)
            {
				OmmNiProviderConfig config = new OmmNiProviderConfig();

				Console.WriteLine($"!!! CreateOmmProvider {i} !!!");
				provider = new OmmProvider(config.Host("localhost:14003").UserName("user"));

				long itemHandle = 5;

				FieldList fieldList = new FieldList();
				fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
				fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
				fieldList.AddReal(30, 9,  OmmReal.MagnitudeTypes.EXPONENT_0);
				fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.Complete();

				provider.Submit( new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
						.State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
						.Payload(fieldList).Complete(true),
                        itemHandle);

				Thread.Sleep(1000);

				for( int j = 0; j < 2; j++ )
				{
					fieldList.Clear();
					fieldList.AddReal(22, 3991 + j, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
					fieldList.AddReal(30, 10 + j, OmmReal.MagnitudeTypes.EXPONENT_0);
                    fieldList.Complete();

					provider.Submit( new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N").Payload( fieldList ), itemHandle );
					Thread.Sleep(1000);
				}
				Console.WriteLine($"!!! provider.Uninitialize() {i} !!!");

				provider.Uninitialize();
            }
			//END APIQA
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
