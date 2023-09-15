/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace LSEG.Ema.Access
{
    internal class ConfigError
    {
        public string Text { get; set; } = string.Empty;
        public LoggerLevel Level { get; set; } = LoggerLevel.ERROR;

        public ConfigError(string text, LoggerLevel level)
        {
            Text = text;
            Level = level;
        }
    }

    internal class ConfigErrorList
    {
        internal List<ConfigError> ErrorList { get; set; } = new List<ConfigError>();

        public ConfigErrorList()
        {
            ErrorList.Clear();
        }

        public void Clear()
        {
            ErrorList.Clear();
        }

        public void Add(string text, LoggerLevel level)
        {
            ConfigError error = new ConfigError(text, level);

            ErrorList.Add(error);
        }

        public int Count()
        {
            return ErrorList.Count;
        }

        // Method to populate the LoggerClient, once the LoggerClient exists in an OmmBaseImpl
        public void Log(ILoggerClient client, LoggerLevel minLevel)
        {
            foreach (ConfigError error in ErrorList)
            {
                if (error.Level <= minLevel)
                {
                    switch (error.Level)
                    {
                        case LoggerLevel.TRACE:
                            client.Trace("EmaConfig", error.Text);
                            break;
                        case LoggerLevel.DEBUG:
                            client.Debug("EmaConfig", error.Text);
                            break;
                        case LoggerLevel.INFO:
                            client.Info("EmaConfig", error.Text);
                            break;
                        case LoggerLevel.WARNING:
                            client.Warn("EmaConfig", error.Text);
                            break;
                        case LoggerLevel.ERROR:
                            client.Error("EmaConfig", error.Text);
                            break;
                    }
                }
            }
        }

        // This is used for unit testing to verify that the config has parsed everything.
        public void print()
        {
            if (ErrorList.Count == 0)
            {
                Console.WriteLine("no configuration errors found");
                return;
            }

            Console.WriteLine("begin configuration errors:");

            foreach (ConfigError error in ErrorList)
            {
                Console.WriteLine("\t" + ILoggerClient.LoggerLevelAsString(error.Level) + " " + error.Text);
            }

            Console.WriteLine("end configuration errors");
        }
    }
}