/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;

namespace LSEG.Eta.Transports.Tests
{
    [Category("StreamIdManagerTest")]
    public class WlStreamIdManagenentTest
    {
        [Fact]
        public void StreamIdManagerTest()
        {
            ISet<int> existingIds = new HashSet<int>();
            WlStreamIdManager manager = new WlStreamIdManager(1200000);

            for (int i = 0; i < 600000; i++)
            {
                int streamId = manager.GetStreamID();
                manager.WlStreamsByIds.Add(streamId, null);
                existingIds.Add(streamId);
                streamId = manager.GetProviderStreamID();
                manager.WlRequestsByIds.Add(streamId, null);
                existingIds.Add(streamId);
            }

            Assert.True(existingIds.Count == 1200000);
            for (int i = 3; i < 600003; i++)
            {
                Assert.Contains(i, existingIds);
                Assert.Contains(-i, existingIds);
            }
        }
    }
}
