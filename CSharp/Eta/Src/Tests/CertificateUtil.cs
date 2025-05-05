/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using Xunit;

namespace LSEG.Eta.Tests
{
    internal static class CertificateUtil
    {
        /// <summary>
        /// Certificate file path. Uses optional environment variable.
        /// </summary>
        public static readonly string CERTIFICATE_CRT = GetExistingFilePathNameForCertificate("ETANET_CERTIFICATE_CRT", "certificate.test.crt", "Certificate");
        /// <summary>
        /// Certificate private key file path. Uses optional environment variable.
        /// </summary>
        public static readonly string CERTIFICATE_KEY = GetExistingFilePathNameForCertificate("ETANET_CERTIFICATE_KEY", "certificate.test.key", "Certificate key");

        private static string GetExistingFilePathNameForCertificate(string envVarName, string defaultValue, string title)
        {
            var filePathName = Path.GetFullPath(Environment.GetEnvironmentVariable(envVarName, EnvironmentVariableTarget.Process) ?? defaultValue);

            var fileName = Path.GetFileName(filePathName);
            var fileDirectory = Path.GetDirectoryName(filePathName);
            Assert.True(File.Exists(filePathName),
                $"{title} file \"{fileName}\" should be either copied to \"{fileDirectory}\" or should be pointed by {envVarName} containing full file path name.");

            return filePathName;
        }
    }
}
