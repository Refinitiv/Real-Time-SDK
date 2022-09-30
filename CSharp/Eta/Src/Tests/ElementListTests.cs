/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;

using Refinitiv.Common.Logger;
using Refinitiv.Eta.Codec;

namespace Refinitiv.Eta.Tests
{
    public class ElementListTests
    {
        ElementList _elementList;

        public ElementListTests(ITestOutputHelper output)
        {
            XUnitLoggerProvider.Instance.Output = output;
            EtaLoggerFactory.Instance.AddProvider(XUnitLoggerProvider.Instance);

            _elementList = new ElementList();
        }

        [Fact]
        [Category("Unit")]
        public void ApplyHasInfoOk()
        {
            var elementList = new ElementList();

            elementList.ApplyHasInfo();

            Assert.True(elementList.CheckHasInfo());
        }

        [Fact]
        [Category("Unit")]
        public void ApplyHasInfoNotStandardData()
        {
            var elementList = new ElementList();

            elementList.ApplyHasInfo();

            Assert.False(elementList.CheckHasStandardData());
        }


        [Fact]
        [Category("Unit")]
        public void ApplyHasStandardDataOk()
        {
            var elementList = new ElementList();

            elementList.ApplyHasStandardData();

            Assert.True(elementList.CheckHasStandardData());
        }


        [Fact]
        [Category("Unit")]
        public void ApplyHasStandardDataAndHasInfoOk()
        {
            var elementList = new ElementList();

            elementList.ApplyHasStandardData();
            elementList.ApplyHasInfo();

            Assert.True(elementList.CheckHasStandardData());
            Assert.True(elementList.CheckHasInfo());
        }

    }
}
