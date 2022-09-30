/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;
using System.Text.RegularExpressions;

namespace Refinitiv.Eta.Codec
{
    /// <summary>
	/// The ETA Time type allows for bandwidth optimized representation of a time
	/// value containing hour, minute, second, millisecond, microsecond, and nanosecond information.
	/// 
	/// <seealso cref="Time"/> is always represented as GMT unless otherwise specified.
	/// </summary>
	sealed public class Time : IEquatable<Time>
	{
		internal const int BLANK_HOUR = 255;
		internal const int BLANK_MINUTE = 255;
		internal const int BLANK_SECOND = 255;
		internal const int BLANK_MILLI = 65535;
		internal const int BLANK_MICRO_NANO = 2047;

		internal int _hour;
		internal int _minute;
		internal int _second;
		internal int _millisecond;
		internal int _microsecond;
		internal int _nanosecond;

		// for value(String) method
        
		private string trimmedVal;
		private MatchCollection matcher;
		private static readonly Regex timePattern1 = new Regex("(\\d+):(\\d+):(\\d+):(\\d+)");
		private static readonly Regex timePattern2 = new Regex("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
		private static readonly Regex timePattern3 = new Regex("(\\d+):(\\d+):(\\d+)");
		private static readonly Regex timePattern4 = new Regex("(\\d+):(\\d+)");
		private static readonly Regex timePattern5 = new Regex("(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");
		private static readonly Regex timePattern6 = new Regex("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
		private static readonly Regex timePattern7 = new Regex("(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");
		private static readonly Regex timePattern8 = new Regex("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");

        /// <summary>
        /// Creates <see cref="Time"/>.
        /// </summary>
        /// <returns> Time object
        /// </returns>
        /// <seealso cref="Time"/>
        public Time()
        {
        }

        /// <summary>
		/// Clears <seealso cref="Time"/>.
		/// </summary>
		public void Clear()
		{
			_hour = 0;
			_minute = 0;
			_second = 0;
			_millisecond = 0;
			_microsecond = 0;
			_nanosecond = 0;
		}

        /// <summary>
        /// Returns true if all members in <seealso cref="Time"/> are set to the values used to signify
        /// blank. A blank <seealso cref="Time"/> contains hour, minute, and second values of 255 
        /// and a millisecond value of 65535.
        /// </summary>
        /// <returns> true if blank, false otherwise </returns>
		public bool IsBlank
		{
            get
            {
                return (_hour == BLANK_HOUR && _minute == BLANK_MINUTE && _second == BLANK_SECOND && _millisecond == BLANK_MILLI && _microsecond == BLANK_MICRO_NANO && _nanosecond == BLANK_MICRO_NANO) ? true : false;
            }
		}

        /// <summary>
		/// Verifies contents of populated <seealso cref="Time"/> structure. Validates the ranges
		/// of the hour, minute, second, and millisecond members.
		/// If <seealso cref="Time"/> is blank or valid, true is returned; false otherwise.
		/// </summary>
		/// <returns> true if valid, false otherwise </returns>
		public bool IsValid
		{
			get
			{
				if (IsBlank)
				{
					return true;
				}
    
				/* month or date or year of 0 is valid because marketfeed can send it */
				if (_hour != BLANK_HOUR && (_hour < 0 || _hour > 23))
				{
					return false;
				}
    
				if (_minute != BLANK_MINUTE && (_minute < 0 || _minute > 59))
				{
					return false;
				}
    
				if (_second != BLANK_SECOND && (_second < 0 || _second > 60))
				{
					return false;
				}
    
				if (_millisecond != BLANK_MILLI && (_millisecond < 0 || _millisecond > 999))
				{
					return false;
				}
    
				if (_microsecond != BLANK_MICRO_NANO && (_microsecond < 0 || _microsecond > 999))
				{
					return false;
				}
    
				if (_nanosecond != BLANK_MICRO_NANO && (_nanosecond < 0 || _nanosecond > 999))
				{
					return false;
				}
    
				if (_nanosecond == BLANK_MICRO_NANO)
				{
					if (_microsecond == BLANK_MICRO_NANO)
					{
						if (_millisecond == BLANK_MILLI)
						{
							if (_second == BLANK_SECOND)
							{
								if (_minute == BLANK_MINUTE)
								{
									return true;
								}
								else
								{
									if (_hour == BLANK_HOUR)
									{
										return false;
									}
									else
									{
										return true;
									}
								}
							}
							else
							{
								if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE))
								{
									return false;
								}
								else
								{
									return true;
								}
							}
						}
						else
						{
							if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE) || (_second == BLANK_SECOND))
							{
								return false;
							}
							else
							{
								return true;
							}
						}
					}
					else
					{
						if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE) || (_second == BLANK_SECOND) || (_millisecond == BLANK_MILLI))
						{
							return false;
						}
						else
						{
							return true;
						}
					}
				}
				else if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE) || (_second == BLANK_SECOND) || (_millisecond == BLANK_MILLI) || (_microsecond == BLANK_MICRO_NANO))
				{
					return false;
				}
    
				return true;
			}
		}

        /// <summary>
		///  This method will perform a deep copy of this Object to destTime.
		/// </summary>
		///  <param name="destTime"> the destination Time Object.
		/// </param>
		///  <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if the destTime is null. 
		///  </returns>
		public CodecReturnCode Copy(Time destTime)
		{
			if (null == destTime)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destTime.Hour(_hour);
			destTime.Minute(_minute);
			destTime.Second(_second);
			destTime.Millisecond(_millisecond);
			destTime.Microsecond(_microsecond);
			destTime.Nanosecond(_nanosecond);

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Convert <seealso cref="Time"/> to a string.
		/// Returns a string in "hour:minute:second:milli" format (e.g. 15:24:54:627).
		/// </summary>
		/// <returns> the string representation of this <seealso cref="Time"/> </returns>
		public override string ToString()
		{
			if (!IsBlank)
			{
				if (IsValid)
				{
					StringBuilder retStr = new StringBuilder(64);

					if (_hour > 9)
					{
						retStr.Append(_hour);
					}
					else
					{
						retStr.Append("0").Append(_hour);
					}

					if (_minute == BLANK_MINUTE)
					{
						return retStr.ToString();
					}
					if (_minute > 9)
					{
						retStr.Append(":").Append(_minute);
					}
					else
					{
						retStr.Append(":").Append("0").Append(_minute);
					}

					if (_second == BLANK_SECOND)
					{
						return retStr.ToString();
					}
					if (_second > 9)
					{
						retStr.Append(":").Append(_second);
					}
					else
					{
						retStr.Append(":").Append("0").Append(_second);
					}

					if (_millisecond == BLANK_MILLI)
					{
						return retStr.ToString();
					}
					if (_millisecond > 99)
					{
						retStr.Append(":").Append(_millisecond);
					}
					else if (_millisecond > 9)
					{
						retStr.Append(":").Append("0").Append(_millisecond);
					}
					else
					{
						retStr.Append(":").Append("00").Append(_millisecond);
					}

					if (_microsecond == BLANK_MICRO_NANO)
					{
						return retStr.ToString();
					}
					if (_microsecond > 99)
					{
						retStr.Append(":").Append(_microsecond);
					}
					else if (_microsecond > 9)
					{
						retStr.Append(":").Append("0").Append(_microsecond);
					}
					else
					{
						retStr.Append(":").Append("00").Append(_microsecond);
					}

					if (_nanosecond == BLANK_MICRO_NANO)
					{
						return retStr.ToString();
					}
					if (_nanosecond > 99)
					{
						retStr.Append(":").Append(_nanosecond);
					}
					else if (_nanosecond > 9)
					{
						retStr.Append(":").Append("0").Append(_nanosecond);
					}
					else
					{
						retStr.Append(":").Append("00").Append(_nanosecond);
					}

					return retStr.ToString();
				}
				else
				{
					return "Invalid time";
				}
			}
			else
			{
				return "";
			}
		}

        /// <summary>
		/// Converts string time from "HH:MM" (13:01) or "HH:MM:SS" (15:23:54) format to <seealso cref="Time"/>.
		/// </summary>
		/// <param name="value"> string containing an appropriately formatted string to convert from
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if value is invalid.  </returns>
		public CodecReturnCode Value(string value)
		{
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

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

            if (trimmedVal.IndexOf('-') != -1)
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }

			try
			{
                // hh:mm:ss:lll:uuu:nnn
                matcher = timePattern7.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Second(int.Parse(group[3].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Millisecond(int.Parse(group[4].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Microsecond(int.Parse(group[5].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Nanosecond(int.Parse(group[6].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

                // hh mm ss lll uuu nnn
                matcher = timePattern8.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Second(int.Parse(group[3].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Millisecond(int.Parse(group[4].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Microsecond(int.Parse(group[5].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Nanosecond(int.Parse(group[6].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

                // hh:mm:ss:lll:uuu
                matcher = timePattern5.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Second(int.Parse(group[3].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Millisecond(int.Parse(group[4].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Microsecond(int.Parse(group[5].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

                // hh mm ss lll uuu nnn
                matcher = timePattern6.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Second(int.Parse(group[3].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Millisecond(int.Parse(group[4].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Microsecond(int.Parse(group[5].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

                matcher = timePattern1.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Second(int.Parse(group[3].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Millisecond(int.Parse(group[4].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

                matcher = timePattern2.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Second(int.Parse(group[3].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Millisecond(int.Parse(group[4].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

                matcher = timePattern3.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Second(int.Parse(group[3].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

                matcher = timePattern4.Matches(trimmedVal);

                if (matcher.Count > 0)
                {
                    Match match = matcher[0];
                    GroupCollection group = match.Groups;
                    ret = Hour(int.Parse(group[1].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
					ret = Minute(int.Parse(group[2].ToString()));
					if (ret != CodecReturnCode.SUCCESS)
					{
						return ret;
					}

					return CodecReturnCode.SUCCESS;
				}

				return CodecReturnCode.INVALID_ARGUMENT;
			}
			catch (Exception)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
        /// Sets all members in <seealso cref="Time"/> to the values used to signify blank. A blank
        /// <seealso cref="Time"/> contains hour, minute, and second values of 255 and a millisecond value of 65535.
        /// </summary>
        public void Blank()
		{
			_hour = BLANK_HOUR;
			_minute = BLANK_MINUTE;
			_second = BLANK_SECOND;
			_millisecond = BLANK_MILLI;
			_microsecond = BLANK_MICRO_NANO;
			_nanosecond = BLANK_MICRO_NANO;
		}

        /// <summary>
		/// Checks equality of two <seealso cref="Time"/> objects.
		/// </summary>
		/// <param name="thatTime"> the other time to compare to this one
		/// </param>
		/// <returns> true if times are equal, false otherwise </returns>
		public bool Equals(Time thatTime)
		{
			return ((thatTime != null) && (_second == thatTime._second) && (_minute == thatTime._minute) && (_hour == thatTime._hour) && (_millisecond == thatTime._millisecond) && (_microsecond == thatTime._microsecond) && (_nanosecond == thatTime._nanosecond));
		}

        /// <summary>Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.</summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.</param>
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        /// otherwise, <c>false</c>.</returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Time)
                return Equals((Time)other);

            return false;
        }

        /// <summary>Serves as a hash function for a particular type.</summary>
        /// <returns>A hash code for the current <c>Object</c>.</returns>
        public override int GetHashCode()
        {
            return ToString().GetHashCode();
        }

        /// <summary>
		/// Used to encode a time into a buffer.
		/// </summary>
		/// <param name="iter"> <see cref="EncodeIterator"/> with buffer to encode into. Iterator
		///            should also have appropriate version information set.
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if success,
		///         <see cref="CodecReturnCode.FAILURE"/> if failure,
		///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the encode buffer is too small
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="CodecReturnCode"/>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.PrimitiveEncoder.EncodeTime((EncodeIterator)iter, this);
		}

        /// <summary>
		/// Decode time.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and
		///            appropriate version information set.
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if success,
		///         <see cref="CodecReturnCode.INCOMPLETE_DATA"/> if failure,
		///         <see cref="CodecReturnCode.BLANK_DATA"/> if data is blank value
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="CodecReturnCode"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeTime(iter, this);
		}

        /// <summary>
		/// The hour of the day (0 - 23 where 255 indicates blank).
		/// </summary>
		/// <param name="hour"> the hour to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if hour is invalid.  </returns>
		public CodecReturnCode Hour(int hour)
		{
			if (hour != BLANK_HOUR && (hour < 0 || hour > 23) )
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_hour = hour;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// The hour of the day (0 - 23 where 255 indicates blank).
		/// </summary>
		/// <returns> the hour </returns>
		public int Hour()
		{
			return _hour;
		}

        /// <summary>
		/// The minute of the hour (0 - 59 where 255 indicates blank).
		/// </summary>
		/// <param name="minute"> the minute to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if minute is invalid.  </returns>
		public CodecReturnCode Minute(int minute)
		{
			if (minute != BLANK_MINUTE && (minute < 0 || minute > 59))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_minute = minute;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// The minute of the hour (0 - 59 where 255 indicates blank).
		/// </summary>
		/// <returns> the minute </returns>
		public int Minute()
		{
			return _minute;
		}

        /// <summary>
        /// The second of the minute (0 - 59 where 255 indicates blank).
        /// </summary>
        /// <param name="second"> the second to set
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
        ///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if second is invalid.  </returns>
        public CodecReturnCode Second(int second)
		{
			if (second != BLANK_SECOND && (second < 0 || second > 60))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_second = second;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
        /// The second of the minute (0 - 60 where 255 indicates blank and 60 is to account for leap second).
        /// </summary>
        /// <returns> the second </returns>
        public int Second()
		{
			return _second;
		}

        /// <summary>
		/// The millisecond of the second (0 - 999 where 65535 indicates blank).
		/// </summary>
		/// <param name="millisecond"> the millisecond to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if millisecond is invalid.  </returns>
		public CodecReturnCode Millisecond(int millisecond)
		{
			if (millisecond != BLANK_MILLI && (millisecond < 0 || millisecond > 999))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_millisecond = millisecond;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// The millisecond of the second (0 - 999 where 65535 indicates blank).
		/// </summary>
		/// <returns> the millisecond </returns>
		public int Millisecond()
		{
			return _millisecond;
		}

        /// <summary>
		/// The microsecond of the millisecond (0 - 999 where 2047 indicates blank).
		/// </summary>
		/// <param name="microsecond"> the microsecond to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if microsecond is invalid.  </returns>
		public CodecReturnCode Microsecond(int microsecond)
		{
			if (microsecond != BLANK_MICRO_NANO && (microsecond < 0 || microsecond > 999))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_microsecond = microsecond;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// The microsecond of the millisecond (0 - 999 where 2047 indicates blank).
		/// </summary>
		/// <returns> the microsecond </returns>
		public int Microsecond()
		{
			return _microsecond;
		}

        /// <summary>
		/// The nanosecond of the microsecond (0 - 999 where 2047 indicates blank).
		/// </summary>
		/// <param name="nanosecond"> the nanosecond to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if nanosecond is invalid.  </returns>
		public CodecReturnCode Nanosecond(int nanosecond)
		{
			if (nanosecond != BLANK_MICRO_NANO && (nanosecond < 0 || nanosecond > 999))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_nanosecond = nanosecond;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// The nanosecond of the microsecond (0 - 999 where 2047 indicates blank).
		/// </summary>
		/// <returns> the nanosecond </returns>
		public int Nanosecond()
		{
			return _nanosecond;
		}
	}

}