/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using global::Xunit.v3;

namespace LSEG.Eta.Tests.Common.Xunit.v3
{
    /// <summary>
    /// Apply this attribute to your test method to specify a category.
    /// </summary>
    [AttributeUsage(AttributeTargets.Assembly | AttributeTargets.Class | AttributeTargets.Method, AllowMultiple = true)]
    public class CategoryAttribute(string category) : Attribute, ITraitAttribute
    {
        // Note that one trait attribute can provide as many traits as it needs to; you're not limited
        // to just one trait from one attribute.
        public IReadOnlyCollection<KeyValuePair<string, string>> GetTraits() =>
        [
            new("Category", category),
            new("Categorized", "true"),
        ];
    }
}
