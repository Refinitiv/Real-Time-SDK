/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Globalization;
using System.Net;
using System.Text;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorRestLogginHandler
    {
        StreamWriter StreamWriter { get; set; }
        ReactorOptions ReactorOptions { get; set; }

        StringBuilder m_StringBuilder = new StringBuilder(5000);

        public ReactorRestLogginHandler(ReactorOptions reactorOptions)
        {
            Stream stream = reactorOptions.RestLogOutputStream != null ? reactorOptions.RestLogOutputStream
                : Console.OpenStandardOutput();

            StreamWriter = new StreamWriter(stream);
            StreamWriter.AutoFlush = true;
            ReactorOptions = reactorOptions;
        }

        public void LogRestRequest(ReactorRestConnectOptions restConnetOptions, HttpRequestMessage requestMessage)
        {
            if(ReactorOptions.EnableRestLogStream)
            {
                try
                {
                    m_StringBuilder.Length = 0;
                    m_StringBuilder.AppendLine($"{NewLine}------ REST REQUEST ------");
                    m_StringBuilder.AppendLine($"Time stamp: {DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff", CultureInfo.InvariantCulture)}");

                    if(!string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyHostName) && !string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyPort))
                    {
                        m_StringBuilder.AppendLine("Proxy connect options: ");
                        m_StringBuilder.AppendLine($"\tHostName: {restConnetOptions.ProxyOptions.ProxyHostName}");
                        m_StringBuilder.AppendLine($"\tPort: {restConnetOptions.ProxyOptions.ProxyPort}");

                        if (!string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyUserName) && !string.IsNullOrEmpty(restConnetOptions.ProxyOptions.ProxyPassword))
                        {
                            m_StringBuilder.AppendLine($"\tUserName: {restConnetOptions.ProxyOptions.ProxyUserName}");
                            m_StringBuilder.AppendLine($"\tPassword: **************");
                        }
                    }

                    m_StringBuilder.AppendLine(requestMessage.ToString());
                    
                    if(requestMessage.Content != null)
                    {
                        var task = requestMessage.Content.ReadAsStringAsync();
                        task.Wait();

                        if(!string.IsNullOrEmpty(task.Result))
                        {
                            m_StringBuilder.AppendLine("Content: ");

                            int index = task.Result.IndexOf(ReactorRestClient.CLIENT_SECRET);
                            if (index != -1)
                            {
                                m_StringBuilder.Append(task.Result.Remove(index, task.Result.Length - index));
                                m_StringBuilder.AppendLine("client_secret=***********");
                            }
                            else
                            {
                                m_StringBuilder.AppendLine(task.Result);
                            }
                        }
                    }

                    m_StringBuilder.AppendLine($"------     END      ------");

                    StreamWriter.WriteLine(m_StringBuilder.ToString());
                }
                catch(Exception)
                {
                    // Do nothing
                }
            }
        }

        public void LogRestResponse(HttpResponseMessage responseMessage, RestResponse restResponse)
        {
            if (ReactorOptions.EnableRestLogStream)
            {
                try
                {
                    m_StringBuilder.Length = 0;
                    m_StringBuilder.AppendLine($"{NewLine}------ REST RESPONSE ------");
                    m_StringBuilder.AppendLine($"Time stamp: {DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff", CultureInfo.InvariantCulture)}");
                    m_StringBuilder.AppendLine(responseMessage.ToString());

                    if (restResponse != null)
                    {
                        if (!string.IsNullOrEmpty(restResponse.Content))
                        {
                            m_StringBuilder.AppendLine("Content: ");
                            m_StringBuilder.AppendLine(restResponse.Content);
                        }
                    }

                    m_StringBuilder.AppendLine($"------     END      ------");

                    StreamWriter.WriteLine(m_StringBuilder.ToString());
                }
                catch (Exception)
                {
                    // Do nothing
                }
            }
        }

        /// <summary>
        /// Finalizer for ReactorRestLogginHandler
        /// </summary>
        ~ReactorRestLogginHandler()
        {
            StreamWriter.Dispose();
        }
    }
}
