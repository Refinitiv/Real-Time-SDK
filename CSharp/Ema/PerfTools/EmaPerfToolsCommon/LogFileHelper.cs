/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    public static class LogFileHelper
    {
        public static LogFileInfo InitFile(string name)
        {
            LogFileInfo fileInfo = new LogFileInfo();
            fileInfo.FileName = name;
            try
            {
                fileInfo.Writer = new StreamWriter(fileInfo.FileName);
                fileInfo.SupportedWriting = true;
            }
            catch (Exception e)
            {
                Console.Error.WriteLine($"Error: Failed to open summary file '{fileInfo.FileName}'. Original error message: {e.Message}");
                Environment.Exit(-1);
            }
            return fileInfo;
        }

        /// <summary>
        /// Safe method for writing data to the file.
        /// </summary>
        /// <param name="fileInfo">Information about file where information should be written to.</param>
        /// <param name="value">Information which must be written to the file</param>
        public static void WriteFile(this LogFileInfo? fileInfo, string value)
        {
            if (fileInfo != null && fileInfo.Writer != null && fileInfo.SupportedWriting)
            {
                fileInfo.Writer.Write(value);
                fileInfo.Writer.Flush();
            }
        }

        /// <summary>
        /// Safe method for closing the file.
        /// </summary>
        /// <param name="fileInfo">Information about file to be closed.</param>
        public static void Close(this LogFileInfo? fileInfo)
        {
            if (fileInfo != null && fileInfo.SupportedWriting && fileInfo.Writer != null)
            {
                fileInfo.Writer.Close();
            }
        }
    }
}
