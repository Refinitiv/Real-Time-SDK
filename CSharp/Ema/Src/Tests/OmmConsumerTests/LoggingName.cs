/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Xunit.v3;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{

    public class LogTestNameAttribute : BeforeAfterTestAttribute
    {
        public override void Before(MethodInfo methodUnderTest, IXunitTest test)
        {
            Console.WriteLine($">>>>>>>>>>>>>>>>>>>>>>>>>>> Starting test: {methodUnderTest.Name}");
        }

        public override void After(MethodInfo methodUnderTest, IXunitTest test)
        {
            Console.WriteLine($">>>>>>>>>>>>>>>>>>>>>>>>> Finished test: {methodUnderTest.Name}");
        }
    }

}
