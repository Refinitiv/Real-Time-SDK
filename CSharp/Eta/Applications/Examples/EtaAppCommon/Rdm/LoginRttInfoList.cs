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
    public class LoginRttInfoList : IEnumerable<LoginRttInfo>
    {
        private List<LoginRttInfo> m_LoginRttInfoList = new List<LoginRttInfo>(ProviderSession.NUM_CLIENT_SESSIONS);

        public LoginRttInfoList()
        {
            for(int i = 0; i < ProviderSession.NUM_CLIENT_SESSIONS; i++)
            {
                m_LoginRttInfoList.Add(new LoginRttInfo());
            }
        }

        /// <summary>
        /// Initialize login rtt info watch list.
        /// </summary>
        public void Init()
        {
            foreach(LoginRttInfo info in m_LoginRttInfoList)
            {
                info.Clear();
            }
        }

        /// <summary>
        /// Gets a login rtt information for a channel with same stream id.
        /// </summary>
        /// <param name="channel">The channel to get the login request information structure for the message</param>
        /// <param name="loginRTT">The <see cret="LoginRTT"></cret> containing the stream id</param>
        /// <returns><see cref="LoginRttInfo"/> for the channel, <c>null</c> if login stream id is different for the same channel or
        /// maximum login limit is reached.
        /// </returns>
        public LoginRttInfo? Get(IChannel channel, LoginRTT loginRTT)
        {
            LoginRttInfo? loginRttInfo = null;
            foreach (LoginRttInfo loginRttInfoTmp in m_LoginRttInfoList)
            {
                if (loginRttInfoTmp.IsInUse && object.ReferenceEquals(loginRttInfoTmp.Channel,channel))
                {
                    loginRttInfo = loginRttInfoTmp;
                    if (loginRttInfo.LoginRtt.StreamId != loginRTT.StreamId)
                    {
                        return null;
                    }
                }
            }

            // get a new one if one is not already in use
            if (loginRttInfo == null)
            {
                foreach (LoginRttInfo loginRttInfoTmp in m_LoginRttInfoList)
                {
                    if (!loginRttInfoTmp.IsInUse)
                    {
                        loginRttInfo = loginRttInfoTmp;
                        loginRttInfo.Channel = channel;
                        loginRttInfo.IsInUse = true;
                        loginRTT.Copy(loginRttInfo.LoginRtt);
                        break;
                    }
                }
            }

            return loginRttInfo;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="channel"></param>
        /// <returns></returns>
        public LoginRttInfo? Get(IChannel channel)
        {
            return m_LoginRttInfoList.Find(x => x.IsInUse && ReferenceEquals(x.Channel, channel));
        }

        public LoginRttInfo? CreateFromRequest(IChannel channel, LoginRequest loginRequest)
        {
            LoginRttInfo? loginRttInfo = m_LoginRttInfoList.Find(x => !x.IsInUse);

            if(loginRttInfo != null)
            {
                loginRttInfo.LoginRtt.InitDefaultRTT(loginRequest.StreamId);
                loginRttInfo.Channel = channel;
                loginRttInfo.IsInUse =true;
            }

            return loginRttInfo;
        }

        public void ClearForStream(int streamId)
        {
            for(int index = 0;index < m_LoginRttInfoList.Count; index++)
            {
                LoginRttInfo loginRttInfo = m_LoginRttInfoList[index];
                if(loginRttInfo.IsInUse && loginRttInfo.LoginRtt.StreamId == streamId)
                {
                    m_LoginRttInfoList.RemoveAt(index);
                    break;
                }
            }
        }

        public IEnumerator<LoginRttInfo> GetEnumerator()
        {
            return m_LoginRttInfoList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
