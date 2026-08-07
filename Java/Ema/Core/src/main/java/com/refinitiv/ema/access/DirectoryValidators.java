/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.rdm.EmaRdm;

/**
 * Provides validation helpers for directory-related state and RDM values.
 *
 * <p>This utility class centralizes checks used when verifying service directory
 * content prior to encoding or processing.</p>
 */
class DirectoryValidators 
{
    /**
     * Prevents instantiation of this utility class.
     */
    DirectoryValidators()
    {
        throw new AssertionError();
    }
    
    /**
     * Verifies that the specified stream state is a supported {@link OmmState.StreamState}
     * value for directory status.
     *
     * @param streamState stream state to validate
     * @return {@code true} if the stream state is valid; otherwise {@code false}
     */
    static boolean isValidStatusStreamState(int streamState)
    {
        switch (streamState)
        {
            case OmmState.StreamState.OPEN:
            case OmmState.StreamState.NON_STREAMING:
            case OmmState.StreamState.CLOSED_RECOVER:
            case OmmState.StreamState.CLOSED:
            case OmmState.StreamState.CLOSED_REDIRECTED:
                return true;
            default:
                return false;
        }
    }

    /**
     * Verifies that the specified data state is a supported {@link OmmState.DataState}
     * value for directory status.
     *
     * @param dataState data state to validate
     * @return {@code true} if the data state is valid; otherwise {@code false}
     */
    static boolean isValidStatusDataState(int dataState)
    {
        switch (dataState)
        {
            case OmmState.DataState.NO_CHANGE:
            case OmmState.DataState.OK:
            case OmmState.DataState.SUSPECT:
                return true;
            default:
                return false;
        }
    }

    /**
     * Verifies that the specified status code is a supported {@link OmmState.StatusCode}
     * value for directory status.
     *
     * @param statusCode status code to validate
     * @return {@code true} if the status code is valid; otherwise {@code false}
     */
    static boolean isValidStatusCode(int statusCode)
    {
        switch (statusCode)
        {
            case OmmState.StatusCode.NONE:
            case OmmState.StatusCode.NOT_FOUND:
            case OmmState.StatusCode.TIMEOUT:
            case OmmState.StatusCode.NOT_AUTHORIZED:
            case OmmState.StatusCode.INVALID_ARGUMENT:
            case OmmState.StatusCode.USAGE_ERROR:
            case OmmState.StatusCode.PREEMPTED:
            case OmmState.StatusCode.JUST_IN_TIME_CONFLATION_STARTED:
            case OmmState.StatusCode.TICK_BY_TICK_RESUMED:
            case OmmState.StatusCode.FAILOVER_STARTED:
            case OmmState.StatusCode.FAILOVER_COMPLETED:
            case OmmState.StatusCode.GAP_DETECTED:
            case OmmState.StatusCode.NO_RESOURCES:
            case OmmState.StatusCode.TOO_MANY_ITEMS:
            case OmmState.StatusCode.ALREADY_OPEN:
            case OmmState.StatusCode.SOURCE_UNKNOWN:
            case OmmState.StatusCode.NOT_OPEN:
            case OmmState.StatusCode.NON_UPDATING_ITEM:
            case OmmState.StatusCode.UNSUPPORTED_VIEW_TYPE:
            case OmmState.StatusCode.INVALID_VIEW:
            case OmmState.StatusCode.FULL_VIEW_PROVIDED:
            case OmmState.StatusCode.UNABLE_TO_REQUEST_AS_BATCH:
            case OmmState.StatusCode.NO_BATCH_VIEW_SUPPORT_IN_REQ:
            case OmmState.StatusCode.EXCEEDED_MAX_MOUNTS_PER_USER:
            case OmmState.StatusCode.ERROR:
            case OmmState.StatusCode.DACS_DOWN:
            case OmmState.StatusCode.USER_UNKNOWN_TO_PERM_SYS:
            case OmmState.StatusCode.DACS_MAX_LOGINS_REACHED:
            case OmmState.StatusCode.DACS_USER_ACCESS_TO_APP_DENIED:
            case OmmState.StatusCode.GAP_FILL:
            case OmmState.StatusCode.APP_AUTHORIZATION_FAILED:
            case OmmState.StatusCode.PREFERRED_HOST_COMPLETE:
            case OmmState.StatusCode.PREFERRED_HOST_START_FALLBACK:
            case OmmState.StatusCode.PREFERRED_HOST_NO_FALLBACK:
                return true;
            default:
                return false;
        }
    }

    /**
     * Verifies that the specified service link code matches one of the supported
     * {@link EmaRdm#SERVICE_LINK_CODE_NONE} family constants.
     *
     * @param linkCode link code to validate
     * @return {@code true} if the link code is valid; otherwise {@code false}
     */
    static boolean isValidLinkCode(long linkCode)
    {
        return linkCode == EmaRdm.SERVICE_LINK_CODE_NONE ||
                linkCode == EmaRdm.SERVICE_LINK_CODE_OK ||
                linkCode == EmaRdm.SERVICE_LINK_CODE_RECOVERY_STARTED ||
                linkCode == EmaRdm.SERVICE_LINK_CODE_RECOVERY_COMPLETED;
    }

    /**
     * Verifies that the specified service link state matches one of the supported
     * {@link EmaRdm.LinkStates} constants.
     *
     * @param linkState link state to validate
     * @return {@code true} if the link state is valid; otherwise {@code false}
     */
    static boolean isValidLinkState(long linkState)
    {
        return linkState == EmaRdm.LinkStates.DOWN || linkState == EmaRdm.LinkStates.UP;
    }

    /**
     * Verifies that the specified source mirroring mode matches one of the supported
     * {@link EmaRdm.SourceMirroringMode} constants.
     *
     * @param sourceMirroringMode source mirroring mode to validate
     * @return {@code true} if the source mirroring mode is valid; otherwise {@code false}
     */
    static boolean isValidSourceMirroringMode(long sourceMirroringMode)
    {
        return sourceMirroringMode == EmaRdm.SourceMirroringMode.ACTIVE_NO_STANDBY ||
                sourceMirroringMode == EmaRdm.SourceMirroringMode.ACTIVE_WITH_STANDBY ||
                sourceMirroringMode == EmaRdm.SourceMirroringMode.STANDBY;
    }

    /**
     * Verifies that the specified warm standby mode matches one of the supported
     * {@link EmaRdm.WarmStandbyDirectoryServiceTypes} constants.
     *
     * @param warmStandbyMode warm standby mode to validate
     * @return {@code true} if the warm standby mode is valid; otherwise {@code false}
     */
    static boolean isValidWarmStandbyMode(long warmStandbyMode)
    {
        return warmStandbyMode == EmaRdm.WarmStandbyDirectoryServiceTypes.ACTIVE ||
                warmStandbyMode == EmaRdm.WarmStandbyDirectoryServiceTypes.STANDBY;
    }

    /**
     * Verifies that the specified filter entry action matches one of the supported
     * {@link FilterEntry.FilterAction} constants.
     *
     * @param action filter entry action to validate
     * @return {@code true} if the filter entry action is valid; otherwise {@code false}
     */
    static boolean isValidFilterEntryAction(int action)
    {
        return action == FilterEntry.FilterAction.SET || action == FilterEntry.FilterAction.UPDATE ||
                action == FilterEntry.FilterAction.CLEAR;
    }

    /**
     * Verifies that the specified map entry action matches one of the supported
     * {@link MapEntry.MapAction} constants.
     *
     * @param action map entry action to validate
     * @return {@code true} if the map entry action is valid; otherwise {@code false}
     */
    static boolean isValidMapEntryAction(int action)
    {
        return action == MapEntry.MapAction.UPDATE || action == MapEntry.MapAction.ADD ||
                action == MapEntry.MapAction.DELETE;
    }

    /**
     * Validates the supplied directory status components.
     *
     * @param streamState stream state to validate
     * @param dataState data state to validate
     * @param statusCode status code to validate
     * @throws OmmInvalidUsageException if any supplied value is not supported
     */
    static void validateStatus(int streamState, int dataState, int statusCode)
    {
        if (!isValidStatusStreamState(streamState))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid streamState value of " + streamState,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (!isValidStatusDataState(dataState))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid dataState value of " + dataState,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (!isValidStatusCode(statusCode))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid statusCode value of " + statusCode,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Verifies that the specified directory data type is within the supported
     * encoded range for directory payload usage.
     *
     * <p>This validator accepts values from {@code 0} to {@code 1023}
     * inclusive, which includes all currently defined {@link EmaRdm.DataTypes}
     * constants.</p>
     *
     * @param type data type to validate
     * @return {@code true} if the data type is in the inclusive range
     *         {@code 0..1023}; otherwise {@code false}
     */
    static boolean isValidDataType(int type)
    {
        return type >= 0 && type <= 1023;
    }
    
    /**
     * Verifies that the specified service identifier is within the supported
     * unsigned 16-bit range.
     *
     * @param serviceId service identifier to validate
     * @return {@code true} if the service identifier is valid; otherwise {@code false}
     */
    static boolean isValidServiceId(int serviceId)
    {
        return serviceId >= 0 && serviceId <= 65535;
    }
}
