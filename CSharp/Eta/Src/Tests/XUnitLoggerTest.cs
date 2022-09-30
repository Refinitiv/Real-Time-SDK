/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;
using Refinitiv.Common.Logger;

namespace Refinitiv.Eta.Tests
{
    #region Integrate Into EtaLogger
    /// <summary>
    /// Test Etaglogger output appears in
    /// Output of tests after running
    /// </summary>
    public class XUnitLoggerTest : IDisposable
    {
        public XUnitLoggerTest(ITestOutputHelper output)
        {
            XUnitLoggerProvider.Instance.Output = output;
            EtaLoggerFactory.Instance.AddProvider(XUnitLoggerProvider.Instance);
        }

        public void Dispose()
        {
            XUnitLoggerProvider.Instance.Output = null;
        }

        [Fact]
        [Category("Manual")]
        public void XunitLogsInformation()
        {
            EtaLogger.Instance.Information("Test information XUnitLoggerProverder");            
        }

        [Fact]
        [Category("Manual")]
        public void XunitLogsError()
        {
            try
            {
                throw new Exception("Test error output");
            }
            catch (Exception ex)
            {
                EtaLogger.Instance.Error(ex.Message);
            }
        }

        [Fact]
        [Category("Manual")]
        public void XunitLogTrace()
        {
            EtaLogger.Instance.Trace("Some trace message");
        }
    }
    #endregion
}
