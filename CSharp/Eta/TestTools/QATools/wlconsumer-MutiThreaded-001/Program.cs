/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.WatchlistConsumer;

WatchlistConsumer consumer = new();
consumer.Init(args);
consumer.Run();
//APIQA
consumer.Shutdown();
//END APIQA