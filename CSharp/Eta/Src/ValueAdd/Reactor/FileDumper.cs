/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text;

namespace LSEG.Eta.ValueAdd.Reactor;

internal class FileDumper
{
    private readonly string m_xmlTraceFileName;
    private readonly bool m_xmlTraceToMultipleFiles;
    private readonly ulong m_xmlTraceMaxFileSize;

    private string m_CurrentFilePath;
    private TextWriter? m_OutStream;
    private bool m_FileSizeReached = false;
    private ulong m_CurrentFileSize = 0;

    private readonly int PROCESS_ID = System.Diagnostics.Process.GetCurrentProcess().Id;

    #region Exposed methods

    internal FileDumper(string xmlTraceFileName, bool xmlTraceToMultipleFiles, ulong xmlTraceMaxFileSize)
    {
        m_xmlTraceFileName = xmlTraceFileName;
        m_xmlTraceToMultipleFiles = xmlTraceToMultipleFiles;
        m_xmlTraceMaxFileSize = xmlTraceMaxFileSize;

        m_CurrentFilePath = GetNewTraceFileName();
        OpenFile();
    }

    internal void Dump(StringBuilder content)
    {
        if (m_FileSizeReached)
        {
            return;
        }
        else if (IsCurrentFileOverSize)
        {
            if (m_xmlTraceToMultipleFiles)
            {
                Close();
                m_CurrentFilePath = GetNewTraceFileName();
                OpenFile();

            }
            else
            {
                // single file
                m_FileSizeReached = true;
                Close();
                return;
            }
        }

        try
        {
            if (m_OutStream is not null)
            {
                m_OutStream.Write(content);
                m_OutStream.Flush();
                m_CurrentFileSize += (ulong)content.Length;
            }
        }
        catch (SystemException)
        {
            // SystemException likely to occur due to lack of system resources when this
            // debug is enabled; thus, ignored by design
        }
    }

    internal void Close()
    {
        try
        {
            m_OutStream?.Close();
        }
        catch (SystemException)
        {
            // SystemException likely to occur due to lack of system resources when this
            // debug is enabled; thus, ignored by design
        }

        m_CurrentFileSize = 0;
    }

    #endregion

    #region Private methods

    private string GetNewTraceFileName()
    {
        string fileName = m_xmlTraceFileName +
            "_" + PROCESS_ID +
            "_" + DateTimeOffset.Now.ToUnixTimeMilliseconds() + ".xml";

        return fileName;
    }

    private bool IsCurrentFileOverSize =>
    (
        m_CurrentFileSize >= m_xmlTraceMaxFileSize
    );

    private void OpenFile()
    {
        try
        {
            m_OutStream = new StreamWriter(m_CurrentFilePath, true);
        }
        catch (SystemException)
        {
            // SystemException likely to occur due to lack of system resources when this
            // debug is enabled; thus, ignored by design
        }
    }

    #endregion

}
