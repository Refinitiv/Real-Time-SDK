/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal class DirectoryHandler
    {
        ProviderTest m_ProviderTest;

        private const int REJECT_MSG_SIZE = 1024;
        private const int STATUS_MSG_SIZE = 1024;
        private const int REFRESH_MSG_SIZE = 1024;

        private DirectoryClose m_DirectoryClose = new DirectoryClose();
        private DirectoryStatus m_DirectoryStatus = new DirectoryStatus();
        private DirectoryRefresh m_DirectoryRefresh = new DirectoryRefresh();
        private DirectoryRequest m_DirectoryRequest = new DirectoryRequest();
        private EncodeIterator m_EncodeIter = new EncodeIterator();

        public const int OPEN_LIMIT = 10;

        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        public DirectoryHandler(ProviderTest providerTest)
        {
            m_ProviderTest = providerTest;
        }

        public ReactorReturnCode HandleDirectoryMsgEvent(RDMDirectoryMsgEvent reactorEvent)
        {
            DirectoryMsg directoryMsg = reactorEvent.DirectoryMsg!;
            ReactorChannel? reactorChannel = reactorEvent.ReactorChannel;

            if (directoryMsg == null)
            {
                if (reactorEvent.Msg != null)
                {
                    Assert.True(false);
                    return ReactorReturnCode.SUCCESS;
                }
                else
                {
                    Assert.True(false);
                    return ReactorReturnCode.SUCCESS;
                }
            }

            switch (directoryMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REQUEST:
                    {
                        DirectoryRequest directoryRequest = directoryMsg.DirectoryRequest!;

                        // Reject any request that does not request at least the Info, State, and Group filters
                        if (((directoryRequest.Filter & ServiceFilterFlags.INFO) == 0) ||
                            ((directoryRequest.Filter & ServiceFilterFlags.STATE) == 0) ||
                            ((directoryRequest.Filter & ServiceFilterFlags.GROUP) == 0))
                        {
                            Assert.True(false);
                            break;
                        }

                        // send source directory response
                        if (SendRefresh(reactorChannel!, directoryRequest, out _) != ReactorReturnCode.SUCCESS)
                        {
                            return ReactorReturnCode.FAILURE;
                        }
                        break;
                    }
                case DirectoryMsgType.CLOSE:
                    {
                        break;
                    }
                default:
                    break;
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode SendRefresh(ReactorChannel reactorChannel, DirectoryRequest srcDirReqInfo, 
            out ReactorErrorInfo? reactorErrorInfo)
        {
            reactorErrorInfo = null;

            // get a buffer for the source directory request
            ITransportBuffer? msgBuf = reactorChannel.GetBuffer(REFRESH_MSG_SIZE, false, out reactorErrorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            // encode source directory request
            m_DirectoryRefresh.Clear();
            m_DirectoryRefresh.StreamId = srcDirReqInfo.StreamId;

            // clear cache
            m_DirectoryRefresh.ClearCache = true;
            m_DirectoryRefresh.Solicited = true;

            // state information for response message
            m_DirectoryRefresh.State.Clear();
            m_DirectoryRefresh.State.StreamState(StreamStates.OPEN);
            m_DirectoryRefresh.State.DataState(DataStates.OK);
            m_DirectoryRefresh.State.Code(StateCodes.NONE);
            m_DirectoryRefresh.State.Text().Data("Source Directory Refresh Completed");

            // attribInfo information for response message
            m_DirectoryRefresh.Filter = srcDirReqInfo.Filter;

            Service service = new();
            Service service2 = new();
            ProviderTest.DefaultService.Copy(service);
            ProviderTest.DefaultService2.Copy(service2);

            // Apply OpenWindow to the service if one is specified.
            if (m_ProviderTest.ProviderSessionOptions.OpenWindow >= 0)
            {
                service.HasLoad = true;
                service.Load.HasOpenWindow = true;
                service.Load.OpenWindow = m_ProviderTest.ProviderSessionOptions.OpenWindow;
            }

            m_DirectoryRefresh.ServiceList.Add(service);

            if (m_ProviderTest.ProviderSessionOptions.SetupSecondDefaultDirectoryStream)
            {
                m_DirectoryRefresh.ServiceList.Add(service2);
            }

            m_SubmitOptions.Clear();

            // encode directory request
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, 
                reactorChannel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                reactorErrorInfo = new ReactorErrorInfo();
                reactorErrorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            ret = m_DirectoryRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                reactorErrorInfo = new ReactorErrorInfo();
                reactorErrorInfo.Error.Text = "DirectoryRefresh.Encode() failed";
                return ReactorReturnCode.FAILURE;
            }

            // send source directory request
            return reactorChannel.Submit(msgBuf, m_SubmitOptions, out reactorErrorInfo);
        }
    }
}
