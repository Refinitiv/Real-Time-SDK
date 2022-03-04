/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.training.common;

import java.beans.Introspector;
import java.beans.PropertyDescriptor;
import java.nio.channels.SelectableChannel;
import java.util.Arrays;

public class TrainingModuleUtils {

    public static int getFDValueOfSelectableChannel(SelectableChannel selectableChannel) {
        try {
            return (Integer) Arrays.stream(Introspector.getBeanInfo(selectableChannel.getClass()).getPropertyDescriptors())
                    .filter(pd -> pd.getDisplayName().equals("FDVal"))
                    .map(PropertyDescriptor::getReadMethod)
                    .findFirst()
                    .orElse(null)
                    .invoke(selectableChannel);
        } catch (Exception e) {
            return -1;
        }
    }

}
