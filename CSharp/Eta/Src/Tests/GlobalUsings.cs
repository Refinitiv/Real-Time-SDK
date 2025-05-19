/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

global using static System.Environment;
global using Thread = LSEG.Eta.Tests.Xunit.ThreadWithExceptionHandling;
[assembly: LSEG.Eta.Tests.Xunit.CatchExceptionAspect]
[assembly: LSEG.Eta.Tests.Xunit.BeforeAfter]