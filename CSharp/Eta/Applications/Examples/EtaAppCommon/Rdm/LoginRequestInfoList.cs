/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System.Collections;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    public class LoginRequestInfoList : IEnumerable<LoginRequestInfo>
    {
        private List<LoginRequestInfo> m_LoginRequestInfoList = new List<LoginRequestInfo>(ProviderSession.NUM_CLIENT_SESSIONS);

        public LoginRequestInfoList()
        {
            for(int i = 0; i < ProviderSession.NUM_CLIENT_SESSIONS; i++)
            {
                m_LoginRequestInfoList.Add(new LoginRequestInfo());
            }
        }

        public void Init()
        {
            foreach(LoginRequestInfo loginRequestInfo in m_LoginRequestInfoList)
            {
                loginRequestInfo.Clear();
            }
        }

        public LoginRequestInfo? Get(IChannel channel, LoginRequest loginRequest)
        {
            LoginRequestInfo? loginRequestInfo = null;
            foreach (LoginRequestInfo loginRequestInfoTmp in m_LoginRequestInfoList)
            {
                if (loginRequestInfoTmp.IsInUse && object.ReferenceEquals(loginRequestInfoTmp.Channel,channel))
                {
                    loginRequestInfo = loginRequestInfoTmp;
                    if (loginRequestInfo.LoginRequest.StreamId != loginRequest.StreamId)
                        return null;
                }
            }

            // get a new one if one is not already in use
            if (loginRequestInfo is null)
            {
                foreach (LoginRequestInfo loginRequestInfoTmp in m_LoginRequestInfoList)
                {
                    if (!loginRequestInfoTmp.IsInUse)
                    {
                        loginRequestInfo = loginRequestInfoTmp;
                        loginRequestInfo.Channel = channel;
                        loginRequestInfo.IsInUse = true;
                        loginRequest.Copy(loginRequestInfo.LoginRequest);
                        break;
                    }
                }
            }

            return loginRequestInfo;
        }

        public LoginRequestInfo? Get(IChannel chnl)
        {
            foreach (LoginRequestInfo loginRequestInfo in m_LoginRequestInfoList)
            {
                if (loginRequestInfo.IsInUse && object.ReferenceEquals(loginRequestInfo.Channel,chnl))
                {
                    return loginRequestInfo;
                }
            }

            return null;
        }

        public IEnumerator<LoginRequestInfo> GetEnumerator()
        {
            return m_LoginRequestInfoList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
