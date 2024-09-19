/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.NIProvider;

public class NIProvider
{
    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            provider = new OmmProvider(config.UserName("user"));

            long itemHandle = 5;

            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(fieldList.Complete()).Complete(true),
                itemHandle);

            PackedMsg packedMsg = new PackedMsg(provider);
            UpdateMsg updateMsg = new UpdateMsg();
            
            for (int i = 0; i < 60; i++)
            {
                packedMsg.InitBuffer();

                // Each message packs 10 individual messages before submitting the packed buffer.
                for (int j = 0; j < 10; j++)
                {
                    fieldList.Clear();
                    fieldList.AddReal(22, 3991 + j, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                    fieldList.AddReal(30, 10 + j, OmmReal.MagnitudeTypes.EXPONENT_0);

                    updateMsg.Clear();
                    updateMsg.ServiceName("NI_PUB").Name("IBM.N").Payload(fieldList.Complete());

                    packedMsg.AddMsg(updateMsg, itemHandle);
                }

                provider.Submit(packedMsg);

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
