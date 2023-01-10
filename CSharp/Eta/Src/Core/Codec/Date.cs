/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.RegularExpressions;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// Represents bandwidth optimized date value containing month, day, and year information.
	/// </summary>
    sealed public class Date : IEquatable<Date>
	{
		internal int _day;
		internal int _month;
		internal int _year;

		internal static readonly string[] Months = new string[] {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

		// for value(String) method
		private string trimmedVal;
		private static readonly Regex datePattern1 = new Regex("(\\d+)/(\\d+)/(\\d+)");
		private static readonly Regex datePattern2 = new Regex("(\\d+)\\s(\\d+)\\s(\\d+)");
		private static readonly Regex datePattern3 = new Regex("(\\d+)\\s(\\w+)\\s(\\d+)");

        /// <summary>
		/// Creates <see cref="Date"/>.
		/// </summary>
		/// <seealso cref="Date"/>
        public Date()
        {
        }

		/// <summary>
		/// Sets all members in Date to 0. Because 0 represents a blank date value,
		/// this performs the same functionality as <see cref="Date.Blank()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			_day = 0;
			_month = 0;
			_year = 0;
		}

		/// <summary>
		///  This method will perform a deep copy of this Object to <paramref name="destDate"/>.
		/// </summary>
		/// <param name="destDate"> the destination Data Object.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if the <paramref name="destDate"/> is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Copy(Date destDate)
		{
			if (null == destDate)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destDate.Day(_day);
			destDate.Month(_month);
			destDate.Year(_year);

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Sets all members in Date to 0. Because 0 represents a blank date value,
		/// this performs the same functionality as <see cref="Date.Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Blank()
		{
			Clear();
		}

        /// <summary>
        /// Returns <c>true</c> if all members in <see cref="Time"/> are set to the values used to signify
        /// blank. A blank <see cref="Date"/> contains day, month and year values of 0. 
        /// </summary>
        /// <returns> <c>true</c> if <see cref="Date"/> is blank, <c>false</c> otherwise
        /// </returns>
		public bool IsBlank
		{
            get
            {
                return (_day == 0 && _month == 0 && _year == 0) ? true : false;
            }
		}

        /// <summary>
		/// Checks equality of two Date structures.
		/// </summary>
		/// <param name="thatDate"> the other date to compare to this one
		/// </param>
		/// <returns> <c>true</c> if dates are equal, <c>false</c> otherwise
		/// </returns>
		public bool Equals(Date thatDate)
		{
			return (thatDate != null) && (_day == (thatDate)._day) && (_month == (thatDate)._month) && (_year == (thatDate)._year);
		}

        /// <summary>
        /// Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.
        /// </summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.
        /// </param>
        /// <returns> <c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        /// otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Date)
                return Equals((Date)other);

            return false;
        }

        /// <summary>
        /// Serves as a hash function for a particular type.
        /// </summary>
        /// <returns>
        /// A hash code for the current <c>Object</c>.
        /// </returns>
        public override int GetHashCode()
        {
            return ToString().GetHashCode();
        }

		/// <summary>
		/// Decode date.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and
		///            appropriate version information set.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if success,
		///         <c>CodecReturnCode.INCOMPLETE_DATA</c> if failure,
		///         <c>CodecReturnCode.BLANK_DATA</c> if data is blank value.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="CodecReturnCode"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeDate(iter, this);
		}

		/// <summary>
		/// Used to encode a date into a buffer.
		/// </summary>
		/// <param name="iter"> <see cref="EncodeIterator"/> with buffer to encode into. Iterator
		///            should also have appropriate version information set.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if success,
		///         <c>CodecReturnCode.FAILURE</c> if failure,
		///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the encode buffer is
		///         too small
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="CodecReturnCode"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.PrimitiveEncoder.EncodeDate(iter, this);
		}

		internal bool LeapYear
		{
			get
			{
				if ((_year % 4 == 0) && ((_year % 100 != 0) || (_year % 400 == 0)))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}

        /// <summary>
		/// Verifies the contents of a populated <see cref="Date"/> object.<br/>
		/// <br/>
		/// Determines whether the specified day is valid within the specified month
		/// (e.g., a day greater than 31 is considered invalid for any month). This method
		/// uses the year member to determine leap year validity of day numbers for
		/// February.
		/// </summary>
		/// <returns> <c>true</c> if <see cref="Date"/> is blank or valid, <c>false</c> otherwise
		/// </returns>
		public bool IsValid
		{
			get
			{
				if (IsBlank)
				{
					return true;
				}
    
				/* month or date or year of 0 is valid because marketfeed can send it */
				switch (_month)
				{
					case 0:
					case 1:
					case 3:
					case 5:
					case 7:
					case 8:
					case 10:
					case 12:
						if (_day > 31)
						{
							return false;
						}
						break;
					case 4:
					case 6:
					case 9:
					case 11:
						if (_day > 30)
						{
							return false;
						}
						break;
					case 2:
						if (_day > 29)
						{
							return false;
						}
						else if ((_day == 29) && !LeapYear)
						{
							return false;
						}
						break;
					default:
						return false;
				}
    
				return true;
			}
		}

        /// <summary>
		/// Converts Date to a String. Returns a String as "DD MM YYYY".
		/// </summary>
		/// <returns> the string representation of this Date
		/// </returns>
		public override string ToString()
		{
			if (!IsBlank)
			{
				if (IsValid)
				{
					StringBuilder retStr = new StringBuilder(56);

					/* normal date */
					/* put this into the same format as marketfeed uses where if any portion is blank, it is represented as spaces */
					if (_day > 9)
					{
						retStr.Append(_day).Append(" ");
					}
					else if (_day > 0)
					{
						retStr.Append(" ").Append(_day).Append(" ");
					}
					else
					{
						retStr.Append("   ");
					}

					if (_month > 0)
					{
						retStr.Append(Months[_month - 1]).Append(" ");
					}
					else
					{
						retStr.Append("    ");
					}

					if (_year > 999)
					{
						retStr.Append(_year).Append(" ");
					}
					else if (_year > 99)
					{
						retStr.Append(" ").Append(_year).Append(" ");
					}
					else if (_year > 9)
					{
						retStr.Append("  ").Append(_year).Append(" ");
					}
					else
					{
						retStr.Append("    ");
					}

					return retStr.ToString();
				}
				else
				{
					return "Invalid date";
				}
			}
			else
			{
				return "";
			}
		}

		/// <summary>
		/// Converts string date from "DD MMM YYYY" (01 JUN 2003) or "MM/DD/YYYY" (6/1/2003) format to Date.
		/// </summary>
		/// <param name="value"> string containing an appropriately formatted string to
		///            convert from
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if value is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Value(string value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
			try
			{
				if (string.ReferenceEquals(value, null))
				{
					return CodecReturnCode.INVALID_ARGUMENT;
				}

				trimmedVal = value.Trim();
				if (trimmedVal.Length == 0)
				{
					// blank
					Blank();
					return CodecReturnCode.SUCCESS;
				}

				MatchCollection matcher = datePattern1.Matches(trimmedVal);

				if (matcher.Count > 0)
				{
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    if (group.Count == 4)
                    {
                        int a = int.Parse(group[1].ToString());
                        int b = int.Parse(group[2].ToString());
                        int c = int.Parse(group[3].ToString());

                        if (a > 255) // assume year here is greater than MAX UINT8
                        {
                            // 1974/04/14
                            ret = Day(c);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            ret = Month(b);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            ret = Year(a);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }
                        else if (c < 100) // assume year here is less than 100, then add 1900
                        {
                            // 04/14/74
                            ret = Day(b);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            ret = Month(a);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            ret = Year(c + 1900);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }
                        else
                        {
                            // 04/14/1974
                            ret = Day(b);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            ret = Month(a);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                            ret = Year(c);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }
                    }
					return CodecReturnCode.SUCCESS;
				}

				if (char.IsDigit(trimmedVal[3]))
				{
                    MatchCollection matcher2 = datePattern2.Matches(trimmedVal);

                    if (matcher2.Count > 0)
                    {
                        Match match = matcher2[0];
                        GroupCollection group = match.Groups;
                        int a = int.Parse(group[1].ToString());
						int b = int.Parse(group[2].ToString());
						int c = int.Parse(group[3].ToString());

						if (a > 255) // assume year here is greater than MAX UINT8
						{
							// 1974/04/14
							ret = Day(c);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Month(b);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Year(a);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
						}
						else if (c < 100) // assume year here is less than 100, then add 1900
						{
							// 04/14/74
							ret = Day(b);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Month(a);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Year(c + 1900);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
						}
						else
						{
							// 04/14/1974
							ret = Day(b);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Month(a);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Year(c);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
						}

						return CodecReturnCode.SUCCESS;
					}
					else
					{
						return CodecReturnCode.INVALID_ARGUMENT;
					}
				}
				else if (char.IsUpper(trimmedVal[3]) || char.IsLower(trimmedVal[3]))
				{
                    MatchCollection matcher3 = datePattern3.Matches(trimmedVal);

                    if (matcher3.Count > 0)
                    {
                        Match match = matcher3[0];
                        GroupCollection group = match.Groups;
                        int a = int.Parse(group[1].ToString());
						string strMon = group[2].ToString();
						int c = int.Parse(group[3].ToString());
						if (c < 100) // assume year here is less than 100, then add 1900
						{
							// 04/14/74
							ret = Day(a);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Month(TranslateMonth(strMon));
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Year(c + 1900);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
						}
						else
						{
							ret = Day(a);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Month(TranslateMonth(strMon));
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
							ret = Year(c);
							if (ret != CodecReturnCode.SUCCESS)
							{
								return ret;
							}
						}

						return CodecReturnCode.SUCCESS;
					}
					else
					{
						return CodecReturnCode.INVALID_ARGUMENT;
					}
				}
			}
			catch (Exception)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			return CodecReturnCode.SUCCESS;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		private int TranslateMonth(string monthStr)
		{
			int i , month = 0;

			for (i = 0; i < 12; i++)
			{
				if (monthStr.Equals(Months[i], StringComparison.CurrentCultureIgnoreCase))
				{
					month = i + 1;
					break;
				}
			}

			return month;
		}

		/// <summary>
		/// Represents the day of the month, where 0 indicates a blank entry. day
		/// allows for a range of 0 to 255, though the value typically does not exceed 31.
		/// </summary>
		/// <param name="day"> the day to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if day is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Day(int day)
		{
			if (day < 0 || day > 31)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_day = day;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Represents the day of the month, where 0 indicates a blank entry. day
		/// allows for a range of 0 to 255, though the value typically does not exceed 31.
		/// </summary>
		/// <returns> the day
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public int Day()
		{
			return _day;
		}

		/// <summary>
		/// Represents the month of the year, where 0 indicates a blank entry. month
		/// allows for a range of 0 to 255, though the value typically does not exceed 12.
		/// </summary>
		/// <param name="month"> the month to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="month"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Month(int month)
		{
			if (month < 0 || month > 12)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_month = month;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Represents the month of the year, where 0 indicates a blank entry. month
		/// allows for a range of 0 to 255, though the value typically does not exceed 12.
		/// </summary>
		/// <returns> the month </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public int Month()
		{
			return _month;
		}

		/// <summary>
		/// Represents the year, where 0 indicates a blank entry. You can use this
		/// member to specify a two or four digit year (where specific usage is
		/// indicated outside of ETA). year allows for a range of 0 to 65,535.
		/// </summary>
		/// <param name="year"> the year to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="year"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Year(int year)
		{
			if (year < 0 || year > 65535)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_year = year;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Represents the year, where 0 indicates a blank entry. You can use this
		/// member to specify a two or four digit year (where specific usage is
		/// indicated outside of ETA). year allows for a range of 0 to 65,535.
		/// </summary>
		/// <returns> the year
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public int Year()
		{
			return _year;
		}

	}

}