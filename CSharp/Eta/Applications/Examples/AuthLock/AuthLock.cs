/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Example.AuthLock;

using LSEG.Eta.Dacs;

internal class AuthLock
{
    private string lastMethodString = string.Empty;
    private readonly List<uint> PEList;
    private readonly AuthorizationLockStatus retStatus;

    private static void Main()
    {
        AuthLock program = new();
        program.PEList.Add(62);
        AuthorizationLock authLock = new(5000, AuthorizationLock.OperatorEnum.OR, program.PEList);

        Console.WriteLine("\nCreate lockData");
        AuthorizationLockData lockData = new();

        program.MyGetLock(authLock, lockData);

        Console.WriteLine("\nCreate lockData1");
        AuthorizationLockData lockData1 = new();
        authLock.AppendPE(144);
        program.MyGetLock(authLock, lockData1);

        Console.WriteLine("\nCreate lockData2");
        AuthorizationLockData lockData2 = new();
        authLock.AppendPE(62);
        program.MyGetLock(authLock, lockData2);

        Console.WriteLine("\nCreate lockData3");
        AuthorizationLockData lockData3 = new();
        authLock.RemoveAllPEs();
        program.MyGetLock(authLock, lockData3);

        Console.WriteLine("\nCreate invalid lockData4");
        int invalidLockLen = lockData2.Size - 1;
        byte[] invalidLock = new byte[invalidLockLen];

        for (int i = 0; i < invalidLockLen; i++)
        {
            invalidLock[i] = lockData2.LockData[i];
        }

        AuthorizationLockData lockData4 = new(invalidLock, invalidLockLen);

        Console.WriteLine("\nCombine lockData, lockData1, and lockData2");
        AuthorizationLockData outLockData = new();

        Console.WriteLine("\nCombine a different lockData");
        AuthorizationLockData outLockData1 = new();
        List<AuthorizationLockData> lockDataList = new()
        {
            lockData,
            lockData1,
            lockData3
        };
        program.MyCombineLock(lockDataList, outLockData1);
        program.MyCompareLock(outLockData, outLockData1);      // identical

        Console.WriteLine("\nAn invalid lock in the combined lock");
        AuthorizationLockData outLockData2 = new();
        lockDataList.Add(lockData4);
        program.MyCombineLock(lockDataList, outLockData2);      // Failure
    }

    private AuthLock()
    {
        PEList = new List<uint>();
        retStatus = new AuthorizationLockStatus();
    }

    private void MyGetLock(AuthorizationLock authLock,
        AuthorizationLockData retLockData)
    {
        lastMethodString = "AuthorizationLock.GetLock()";
        LockResultEnum result = authLock.GetLock(retLockData, retStatus);

        if (result == LockResultEnum.LOCK_SUCCESS)
        {
            Console.WriteLine(lastMethodString.ToString() + " - Success ");
        }
        else
        {
            Console.WriteLine(lastMethodString.ToString() + " - Failure: " + retStatus.StatusText.ToString());
        }
    }

    private void MyCompareLock(AuthorizationLockData lockData1,
        AuthorizationLockData lockData2)
    {
        lastMethodString = "AuthorizationLockData.CompareLock()";
        LockResultEnum result = lockData1.CompareLock(lockData2, retStatus);

        if (result == LockResultEnum.LOCK_IDENTICAL)
        {
            Console.WriteLine(lastMethodString.ToString() + " - Two locks are identical");
        }
        else if (result == LockResultEnum.LOCK_DIFFERENT)
        {
            Console.WriteLine(lastMethodString.ToString() + " - Two locks are different.");
        }
        else
        {
            Console.WriteLine(lastMethodString.ToString() + " - Failure: " + retStatus.StatusText.ToString());
        }
    }

    private void MyCombineLock(List<AuthorizationLockData> lockDataList,
                            AuthorizationLockData retLockData)
    {
        lastMethodString = "AuthorizationLockData.CombineLock()";
        LockResultEnum result = retLockData.CombineLock(lockDataList, retStatus);

        if (result == LockResultEnum.LOCK_SUCCESS)
        {
            Console.WriteLine(lastMethodString.ToString() + " - Success");
        }
        else
        {
            Console.WriteLine(lastMethodString.ToString() + " - Failure: " + retStatus.StatusText.ToString());
        }
    }
}
