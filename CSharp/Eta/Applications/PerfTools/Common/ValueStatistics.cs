/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Class for calculating running statistics for a given value(such as latency).
    /// </summary>
    public class ValueStatistics
    {
        private double m_Sum;   // Used in calculating variance.
        private double m_SumOfSquares; // Used in calculating variance.

        /// <summary>
        /// Gets or sets total number of samples
        /// </summary>
        public long Count { get; set; }

        /// <summary>
        /// Gets or sets current mean of samples
        /// </summary>
        public double Average { get; set; }

        /// <summary>
        /// Gets or sets current variance of samples.
        /// </summary>
        public double Variance { get; set; }
        
        /// <summary>
        /// Gets or sets highest sample value.
        /// </summary>
        public double MaxValue { get; set; }

        /// <summary>
        /// Gets or sets lowest sample value.
        /// </summary>
        public double MinValue { get; set; }

        /// <summary>
        /// Instantiates a new value statistics.
        /// </summary>
        public ValueStatistics()
        {
            MaxValue = -double.MaxValue;
            MinValue = double.MaxValue;
        }

        /// <summary>
        /// Clears ValueStatistics.
        /// </summary>
        public void Clear()
        {
            Count = 0;
            Average = 0;
            Variance = 0;
            MaxValue = -double.MaxValue;
            MinValue = double.MaxValue;
            m_Sum = 0;
            m_SumOfSquares = 0;
        }

        /// <summary>
        /// Recalculates stats based on new value.
        /// </summary>
        /// <param name="newValue">The new value</param>
        public void Update(double newValue)
        {
            ++Count;

            // Check against max & min
            if (newValue > MaxValue) MaxValue = newValue;
            if (newValue < MinValue) MinValue = newValue;

            /* Average and variance are calculated using online algorithms.
             * - Average: http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average 
             * - Variance: Devore,Farnum. "Applied Statistics for Engineers and Scientists(Second Edition)", p. 73,74 */

            Average = (newValue + Average * (Count - 1)) / Count;

            m_Sum += newValue;
            m_SumOfSquares += newValue * newValue;
            Variance = Count > 1 ?
                (m_SumOfSquares - m_Sum * m_Sum / Count) / (Count - 1) : 0;
        }

        /// <summary>
        /// Prints a line containing all calculated statistics.
        /// </summary>
        /// <param name="valueStatsName">The value stats name</param>
        /// <param name="countUnitName">The count unit name</param>
        /// <param name="displayThousandths">The display thousandths</param>
        public void Print(string valueStatsName, string countUnitName, bool displayThousandths)
        {
            string strFormat = displayThousandths ?
                       "{0}: Avg:{1,8:F3} StdDev:{2,8:F3} Max:{3,8:F3} Min:{4,8:F3}, {5}: {6:D}"
                     : "{0}: Avg:{1,6:F1} StdDev:{2,6:F1} Max:{3,6:F1} Min:{4,6:F1}, {5}: {6:D}";

            Console.WriteLine(strFormat, valueStatsName, Average, Math.Sqrt(Variance), MaxValue, MinValue, countUnitName, Count);
        }
    }
}
