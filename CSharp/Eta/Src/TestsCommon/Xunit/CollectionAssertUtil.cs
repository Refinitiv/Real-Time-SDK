/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Tests.Common.Xunit
{
    public static class CollectionAssertUtil
    {
        /// <summary>
        /// Similar to <see cref="Assert.Collection{T}(IEnumerable{T}, Action{T}[])"/> but doesn't take order of <paramref name="elementInspections"/> into account.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="collection"></param>
        /// <param name="elementInspections"></param>
        public static void AssertCollectionUnordered<T>(ICollection<T> collection, params Action<T>[] elementInspections)
        {
            if (elementInspections.Length == 0)
            {
                throw new ArgumentException($"No element inspections passed to the method {nameof(AssertCollectionUnordered)}. At lease one inspection is needed.", nameof(elementInspections));
            }
            if (collection.Count != elementInspections.Length)
            {
                Assert.Fail($"Collection & its element inspections have different lengths ({collection.Count} vs {elementInspections.Length}).");
            }

            var inspectionList = elementInspections
                .Select((inspection, i) => (Index: i, Inspection: inspection, Exceptions: new List<Exception>()))
                .ToList();
            foreach (var item in collection)
            {
                var inspectionIndex = 0;
                while (inspectionList.Count > 0 && inspectionIndex < inspectionList.Count)
                {
                    var (_, inspection, exceptions) = inspectionList[inspectionIndex];
                    if (IsSuccessfulInspection(item, inspection, exceptions))
                    {
                        inspectionList.RemoveAt(inspectionIndex);
                        break;
                    }
                    else
                    {
                        inspectionIndex++;
                    }
                }
            }
            if (inspectionList.Count > 0)
            {
                Assert.Fail($"Collection has unsatisfied inspections:{NewLine}" +
                    string.Join(
                        NewLine,
                        inspectionList.Select(x =>
                            $"#{x.Index} inspection failures: ============================={NewLine}" +
                            string.Join($"{NewLine}-----------------------------{NewLine}", x.Exceptions))));
            }

            bool IsSuccessfulInspection(T item, Action<T> inspector, ICollection<Exception> exceptions)
            {
                try
                {
                    inspector(item);
                    return true;
                }
                catch (Exception e)
                {
                    exceptions.Add(e);
                    return false;
                }
            }
        }
    }
}
