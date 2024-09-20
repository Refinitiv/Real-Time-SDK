/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.NIProvider;

public class NIProvider
{
    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new();

            provider = new(config.UserName("user"));

            int itemNumber = 1000;
            int sleepTime = 1000;

            FieldList fieldList = new();

            fieldList.AddInt(1, 6560);
            fieldList.AddInt(2, 66);
            fieldList.AddInt(3855, 52832001);
            fieldList.AddRmtes(296, new EmaBuffer(Encoding.ASCII.GetBytes("BOS")));
            fieldList.AddTime(375, 21, 0);
            fieldList.AddTime(1025, 14, 40, 32);
            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.Complete();

            DateTime start = DateTime.Now;

            for (int handle = 0; handle < itemNumber; ++handle)
            {
                provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("RTR" + handle + ".N")
                        .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                        .Payload(fieldList).Complete(true), handle);
            }

            DateTime end = DateTime.Now;

            TimeSpan timeSpent = end - start;

            Console.WriteLine($"total refresh count = {itemNumber}" +
                    $"\ntotal time = {timeSpent.TotalSeconds} sec" +
                    $"\nrefresh rate = {itemNumber / timeSpent.TotalMilliseconds / 1000} refresh per sec");

            DateTime midpoint = end = start = DateTime.Now;
            int updateCount = 0;
            UpdateMsg updateMsg = new();

            while (start + TimeSpan.FromMilliseconds(300000) > end)
            {
                for (int handle = 0; handle < itemNumber; ++handle)
                {
                    fieldList.Clear();
                    fieldList.AddTime(1025, 14, 40, 32);
                    fieldList.AddInt(3855, 52832001);
                    fieldList.AddReal(22, 14400 + (((handle & 0x1) == 1) ? 1 : 10), OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                    fieldList.AddReal(30, 10 + (((handle & 0x1) == 1) ? 10 : 20), OmmReal.MagnitudeTypes.EXPONENT_0);
                    fieldList.AddRmtes(296, new EmaBuffer(Encoding.ASCII.GetBytes("NAS")));
                    fieldList.Complete();

                    updateMsg.Clear().ServiceName("TEST_NI_PUB").Name("RTR" + handle + ".N").Payload(fieldList);
                    provider.Submit(updateMsg, handle);
                    ++updateCount;
                }

                Thread.Sleep(sleepTime);

                end = DateTime.Now;

                if (end >= midpoint + TimeSpan.FromMilliseconds(1000))
                {
                    timeSpent = end - midpoint;

                    Console.WriteLine($"update count = {updateCount}" +
                            $"\nupdate rate = {updateCount / timeSpent.TotalMilliseconds / 1000} update per sec");

                    updateCount = 0;
                    midpoint = end;
                }
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