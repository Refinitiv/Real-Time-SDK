/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Reflection;

namespace LSEG.Ema.Access
{
    internal static class ComponentVersion
    {
        private const string DefaultEmaVersion = "EMA C# Edition";

        internal static string ProductInternalVersion { get; } = GetProductInternalVersion();

        private static string GetProductInternalVersion()
        {
            Assembly? assembly = null;
            FileVersionInfo? fileVersionInfo = null;
            try
            {
                assembly = Assembly.GetExecutingAssembly();
                fileVersionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
            }
            catch (Exception) { }

            if (assembly != null && fileVersionInfo != null && fileVersionInfo.ProductVersion != null)
            {
                string[] versionNumbers = fileVersionInfo.ProductVersion.Split('.');

                string productVersion = string.Empty;

                if (versionNumbers.Length >= 3)
                {
                    productVersion = $"{versionNumbers[0]}.{versionNumbers[1]}.{versionNumbers[2]}";
                }

                return $"emacsharp{productVersion}.G2.all.rrg";
            }
            return DefaultEmaVersion;
        }
    }
}
