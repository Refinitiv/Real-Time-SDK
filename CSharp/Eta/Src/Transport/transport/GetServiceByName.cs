/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using System.Linq;
using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Transports
{
    internal static class GetServiceByName
    {
        private static string _servicesFile;

        private static void FindFile()
        {
            if (File.Exists("/etc/services"))
            {
                _servicesFile = "/etc/services";
                return;
            }

            string winDirServices = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "drivers\\etc\\services");
            if (File.Exists(winDirServices))
            {
                _servicesFile = winDirServices;
            }
        }

        private static int GetPort(string tcpipService)
        {
            // line in services file
            // # <service name>  <port number>/<protocol>  [aliases...]   [#<comment>]
            // chargen            19/tcp    ttytst source          #Character generator

            try
            {
                var query = (from line in File.ReadLines(_servicesFile)
                    let parts = line.Split(' ')
                    where parts.Length >= 2
                    let serviceName = parts[0].Trim()
                    let portProtocol = parts[1].Trim().Split('/')
                    where serviceName.ICmp(tcpipService) && portProtocol.Length == 2
                    select int.Parse(portProtocol[0]))
                    .ToList();
                return query.Any() ? query.First() : -1;

            }
            catch (Exception)
            {
                return -1;
            }
        }

        public static int Get(string tcpipService)
        {
            if (string.IsNullOrEmpty(_servicesFile))
            {
                FindFile();
            }

            if (!string.IsNullOrEmpty(_servicesFile))
            {
                int port = GetPort(_servicesFile);
                if (port > 0)
                    return port;
            }

            // Get well known port by service
            if (tcpipService.ICmp("triarch_sink"))
                return 8101;
            if (tcpipService.ICmp("triarch_src"))
                return 8102;
            if (tcpipService.ICmp("triarch_dbms"))
                return 8103;
            if (tcpipService.ICmp("rmds_ssl_sink"))
                return 8101;
            if (tcpipService.ICmp("rmds_ssl_source"))
                return 8103;
            if (tcpipService.ICmp("ssl_consumer"))
                return 8101;
            if (tcpipService.ICmp("ssl_provider"))
                return 8102;
            if (tcpipService.ICmp("_consumer"))
                return 14002;
            if (tcpipService.ICmp("_provider"))
                return 14003;
            return -1;
        }
    }
}


