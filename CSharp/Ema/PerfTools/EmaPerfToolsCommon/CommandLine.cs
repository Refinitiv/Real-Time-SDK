/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.PerfTools.Common
{
    public class CommandLine
    {
        private static IDictionary<string, List<string>> Parameters = new Dictionary<string, List<string>>();

        // for random option lookup when parsing command line
        private static IDictionary<string, Option> Options = new Dictionary<string, Option>();

        // maintains order so help text can be constructed Can contain either
        // Options or strings
        private static List<Object> OptionsAndHelpText = new List<Object>();

        private static string ArgumentPrefix = "-";
        private static string ProgramName = " ";

        static CommandLine()
        {
            AddOption("help", false, "Display help information and exit");
        }

        /// <summary>
        /// Specify a string command line option (e.g. -option value) without a default value.
        /// </summary>
        /// <param name="arg">the option name</param>
        /// <param name="description">the description of the option</param>
        public static void AddOption(string arg, string description)
        {
            if (!Options.ContainsKey(arg))
            {
                Option option = new Option(arg, null, description, false);
                Options.Add(arg, option);
                OptionsAndHelpText.Add(option);
            }
        }

        /// <summary>
        /// Specify a string command line option (e.g. -option value).
        /// </summary>
        /// <param name="arg">option name</param>
        /// <param name="defaultValue">string default</param>
        /// <param name="description">explanation of the option</param>
        public static void AddOption(string arg, string defaultValue, string description)
        {
            if (!Options.ContainsKey(arg))
            {
                Option option = new Option(arg, new string[] { defaultValue! }, description, false);
                Options.Add(arg, option);
                OptionsAndHelpText.Add(option);
            }
        }

        /// <summary>
        /// Adds the option.
        /// </summary>
        /// <param name="arg">the argument name</param>
        /// <param name="defaultValues">the default values</param>
        /// <param name="description">the description of the option</param>
        public static void AddOption(string arg, string[] defaultValues, string description)
        {
            if (!Options.ContainsKey(arg))
            {
                Option option = new Option(arg, defaultValues, description, false);
                Options.Add(arg, option);
                OptionsAndHelpText.Add(option);
            }
        }

        /// <summary>
        /// Adds the required option.
        /// </summary>
        /// <param name="arg">the argument name</param>
        /// <param name="description">the description of the option</param>
        public static void AddRequiredOption(string arg, string description)
        {
            if (!Options.ContainsKey(arg))
            {
                Option option = new Option(arg, null, description, true);
                Options.Add(arg, option);
                OptionsAndHelpText.Add(option);
            }
        }

        /// <summary>
        /// Specify an integer command line option (e.g. -option 10).
        /// </summary>
        /// <param name="arg">option name</param>
        /// <param name="defaultValue">int default</param>
        /// <param name="description">explanation of the option</param>
        public static void AddOption(string arg, int defaultValue, string description)
        {
            AddOption(arg, defaultValue.ToString(), description);
        }

        /// <summary>
        /// Specify a bool command line option (e.g. -option true).
        /// </summary>
        /// <param name="arg">option name</param>
        /// <param name="defaultValue">bool default</param>
        /// <param name="description">explanation of the option</param>
        public static void AddOption(string arg, bool defaultValue, string description)
        {
            AddOption(arg, defaultValue.ToString(), description);
        }

        /// <summary>
        /// help string.
        /// </summary>
        /// <returns>option help text generated from all added options</returns>
        public static string OptionHelpString()
        {
            StringBuilder helpstring = new StringBuilder(OptionsAndHelpText.Count * 80);
            helpstring.Append("Usage: ");
            helpstring.Append(ProgramName);
            helpstring.Append(" <Options>\n");
            helpstring.Append("Valid Options:\n");
            foreach (var entry in OptionsAndHelpText)
            {
                helpstring.Append(entry.ToString());
            }
            return helpstring.ToString();
        }

        /// <summary>
        /// Checks whether the argument is present.
        /// </summary>
        /// <param name="varName">the name of the variable</param>
        /// <returns>true if the argument is present</returns>
        public static bool HasArg(string varName)
        {
            return Parameters.ContainsKey(varName);
        }

        /// <summary>
        /// Parses the arguments.
        /// </summary>
        /// <param name="argv">array of arguments.</param>
        /// <exception cref="Exception"> thrown in case of failed parsing</exception>
        public static void ParseArgs(string[] argv)
        {
            ArgumentToken.Clear();
            ArgumentToken? current = ArgumentToken.Next(argv);
            while (current != null)
            {
                if (!current.HasArgPrefix) // all options start with prefix e.g. "-"
                {
                    throw new Exception("Unknown option: " + current.ParsedString);
                }

                if (!current.IsKnownOption) // option must match the known options
                {
                    throw new Exception("Unknown option: " + current.ParsedString);
                }

                ArgumentToken? next = ArgumentToken.Next(argv);
                if (next == null || (next != null && next.HasArgPrefix && next.IsKnownOption))
                {
                    // Next token is a new -option (or end-of-line), so treat current 
                    // -option as a bool with true value
                    List<string> parameters = new List<string>();
                    parameters.Add("true");
                    Parameters.Add(current.ParsedString!, parameters);

                    current = next;
                    continue;
                }

                List<string>? paramValues;
                Parameters.TryGetValue(current.ParsedString!, out paramValues);
                if (paramValues == null)
                {
                    paramValues = new List<string>();
                    Parameters.Add(current.ParsedString!, paramValues);
                }

                paramValues.Add(next!.ParsedString!);

                current = ArgumentToken.Next(argv);
            }

            if (HasArg("help"))
            {
                Console.WriteLine(OptionHelpString());
                Environment.Exit(0);
            }

            StringBuilder errors = new StringBuilder();
            if (ReqdArgMissing(errors))
            {
                throw new Exception("Errors:" + errors);
            }
        }

        /// <summary>
        /// Gets the list of values of the variable.
        /// </summary>
        /// <param name="varName">required command line variable (without the '-')</param>
        /// <returns>List of string values of the variable</returns>
        public static List<string>? Values(string varName)
        {
            List<string>? varValues;
            Parameters.TryGetValue(varName, out varValues);
            if (varValues != null)
                return varValues;

            //default values
            Option option = Options[varName];

            if (option == null || option.DefaultValue == null || option.DefaultValue.Length == 0)
                return null;
            varValues = new List<string>(option.DefaultValue.Length);
            foreach (string defaultValue in option.DefaultValue)
            {
                if (defaultValue != null)
                    varValues.Add(defaultValue);
            }
            return varValues.Count == 0 ? null : varValues;
        }

        /// <summary>
        /// Gets the string value of the variable
        /// </summary>
        /// <param name="varName">required command line variable (without the '-')</param>
        /// <returns>string value of the variable</returns>
        public static string? Value(string varName)
        {
            List<string>? values = Values(varName);
            if (values == null)
                return null;
            return values[0];
        }

        /// <summary>
        /// Gets the boolean value of the variable
        /// </summary>
        /// <param name="varName">required command line variable (without the '-')</param>
        /// <returns>boolean value of the variable</returns>
        public static bool BoolValue(string varName)
        {
            List<string>? values = Values(varName);
            bool retValue = HasArg(varName);

            if (values == null)
                return retValue;

            try
            {
                retValue = Boolean.Parse(values[0]);
            }
            catch (Exception) { }

            return retValue;
        }

        /// <summary>
        /// Gets the int value of the variable
        /// </summary>
        /// <param name="varName">required command line variable (without the '-')</param>
        /// <returns>int value of the variable</returns>
        public static int IntValue(string varName)
        {
            List<string>? values = Values(varName);
            if (values == null)
                return 0;
            return int.Parse(values[0]);
        }

        /// <summary>
        /// Sets program name
        /// </summary>
        /// <param name="progname">Program name for the help string</param>
        public static void ProgName(string progname)
        {
            ProgramName = progname;
        }

        private static bool ReqdArgMissing(StringBuilder missingReqdError)
        {
            bool isReqdMissing = false;
            foreach (var optionKey in Options.Keys)
            {
                if (Options[optionKey].IsRequired && !Parameters.ContainsKey(optionKey))
                {
                    isReqdMissing = true;
                    missingReqdError.Append("\nCommand line option: '" + Options[optionKey].Arg + "' is required");
                }
            }
            return isReqdMissing;
        }

        /// <summary>
        /// Represents command line option
        /// </summary>
        private class Option
        {
            StringBuilder buf = new StringBuilder(100);

            public string Arg;
            public string[]? DefaultValue;
            public string Description;
            public bool IsRequired;

            public Option(string a, string[]? dv, string d, bool isRequired)
            {
                Arg = a;
                DefaultValue = dv;
                Description = d;
                IsRequired = isRequired;
            }

            public override string ToString()
            {
                buf.Clear();
                buf.Append(" ");
                if (!IsRequired)
                    buf.Append("[");
                buf.Append("-");
                buf.Append(Arg);
                buf.Append(" ");

                buf.Append(" <");
                buf.Append(Description);
                if (DefaultValue != null)
                {
                    buf.Append(". Default is:");
                    bool isFirst = true;
                    foreach (string dStr in DefaultValue)
                    {
                        if (isFirst)
                        {
                            isFirst = false;
                        }
                        else
                        {
                            buf.Append(",");
                        }
                        buf.Append(dStr);
                    }
                }

                buf.Append(">");
                if (!IsRequired)
                    buf.Append(" ]");
                buf.Append("\n");
                return buf.ToString();
            }

        }

        /// <summary>
        /// Parses the arguments array
        /// </summary>
        private class ArgumentToken
        {
            public bool HasArgPrefix = false;
            public bool IsKnownOption = false;
            public string? ParsedString;

            public ArgumentToken(string arg)
            {
                if (arg == null)
                    return;

                if (arg.StartsWith(ArgumentPrefix))
                {
                    HasArgPrefix = true;
                    string optionString = arg.Substring(ArgumentPrefix.Length, arg.Length - 1);
                    if (Options.ContainsKey(optionString))
                    {
                        ParsedString = optionString;
                        IsKnownOption = true;
                    }
                    else
                    {
                        // Not a known option: leave ArgumentPrefix (e.g. "-") as part of the value string
                        ParsedString = arg;
                    }
                }
                else
                    ParsedString = arg;
            }

            private static int argCount = 0;
            public static void Clear()
            {
                argCount = 0;
            }

            public static ArgumentToken? Next(string[] argv)
            {
                if (argCount < argv.Length)
                {
                    ArgumentToken next = new ArgumentToken(argv[argCount]);
                    ++argCount;
                    return next;
                }
                else
                    return null;
            }
        }
    }
}
