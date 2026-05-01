/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

global using Xunit;
global using LSEG.Eta.Tests.Common.Xunit.v3;
global using static System.Environment;
global using static LSEG.Eta.Tests.Common.Xunit.CollectionAssertUtil;
global using Thread = LSEG.Eta.Tests.Common.Xunit.ThreadWithExceptionHandling;
global using Buffer = LSEG.Eta.Codec.Buffer;
global using Directory = LSEG.Eta.Rdm.Directory;
[assembly: LSEG.Eta.Tests.Common.Xunit.CatchExceptionAspect]
[assembly: LSEG.Eta.Tests.Xunit.BeforeAfter]