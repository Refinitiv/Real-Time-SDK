/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Example.Common;
public abstract class CommandLineArg<T>
{
    public abstract string Name { get; }
    public abstract string Description { get; }
    public CommandLineArg()
    {
        CommandLine.AddOption(Name, Description);
    }
    protected abstract T Parse(string? value);
    public T Value => Parse(CommandLine.Value(Name));
}
