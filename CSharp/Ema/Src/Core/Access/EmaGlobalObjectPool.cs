/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    internal class EmaGlobalObjectPool
    {
        private static EmaObjectManager m_globalEmaObjectManager = new EmaObjectManager(EmaObjectManager.INITIAL_POOL_SIZE, true);

        public static EmaObjectManager Instance { get => m_globalEmaObjectManager; }
    }
}
