/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * Used by examples to assist in processing command line arguments.
 */
public final class CommandLine
{
    private static Map<String, List<String>> _parameters = new HashMap<String, List<String>>();

    // for random option lookup when parsing command line
    private static Map<String, Option> _options = new HashMap<String, Option>();

    // maintains order so help text can be constructed Can contain either
    // _options or Strings
    private static List<Object> _optionsAndHelpText = new LinkedList<Object>();

    private static String _argumentPrefix = "-";
    private static String _programName = " ";

    static
    {
        addOption("help", false, "Display help information and exit");
    }

    private CommandLine()
    {
        throw new AssertionError();
    }
    
    /**
     * Specify a string command line option (e.g. -option value) without a
     * default value.
     *
     * @param arg the arg
     * @param description explanation of the option
     */
    public static void addOption(String arg, String description)
    {
    	if (!_options.containsKey(arg))
    	{
	        Option option = new Option(arg, null, description, false);
	        _options.put(arg, option);
	        _optionsAndHelpText.add(option);
    	}
    }

    /**
     * Specify a string command line option (e.g. -option value).
     * 
     * @param arg option name
     * @param defaultValue String default
     * @param description explanation of the option
     */
    public static void addOption(String arg, String defaultValue, String description)
    {
    	if (!_options.containsKey(arg))
    	{
	        Option option = new Option(arg, new String[] { defaultValue }, description, false);
	        _options.put(arg, option);
	        _optionsAndHelpText.add(option);
    	}
    }

    /**
     * Adds the option.
     *
     * @param arg the arg
     * @param defaultValues the default values
     * @param description the description
     */
    public static void addOption(String arg, String[] defaultValues, String description)
    {
    	if (!_options.containsKey(arg))
    	{
	        Option option = new Option(arg, defaultValues, description, false);
	        _options.put(arg, option);
	        _optionsAndHelpText.add(option);
    	}
    }

    /**
     * Adds the required option.
     *
     * @param arg the arg
     * @param description the description
     */
    public static void addRequiredOption(String arg, String description)
    {
    	if (!_options.containsKey(arg))
    	{
	        Option option = new Option(arg, null, description, true);
	        _options.put(arg, option);
	        _optionsAndHelpText.add(option);
    	}
    }

    /**
     * Specify an integer command line option (e.g. -option 10).
     * 
     * @param arg option name
     * @param defaultValue int default
     * @param description explanation of the option
     */
    public static void addOption(String arg, int defaultValue, String description)
    {
        addOption(arg, String.valueOf(defaultValue), description);
    }

    /**
     * Specify a boolean command line option (e.g. -option true).
     * 
     * @param arg option name
     * @param defaultValue boolean default
     * @param description explanation of the option
     */
    public static void addOption(String arg, boolean defaultValue, String description)
    {
        addOption(arg, String.valueOf(defaultValue), description);
    }

    /**
     * Option help string.
     *
     * @return option help text generated from all added options
     */
    public synchronized static String optionHelpString()
    {
        StringBuilder helpString = new StringBuilder(_optionsAndHelpText.size() * 80);
        helpString.append("Usage: ");
        helpString.append(_programName);
        helpString.append(" <Options>\n");
        helpString.append("Valid Options:\n");
        for (Iterator<Object> iter = _optionsAndHelpText.iterator(); iter.hasNext();)
        {
            Object next = iter.next();
            helpString.append(next.toString());
        }
        return helpString.toString();
    }

    /**
     * Checks for arg.
     *
     * @param varName the var name
     * @return true, if successful
     */
    public static boolean hasArg(String varName)
    {
        return _parameters.containsKey(varName);
    }
    
    private static class ArgumentToken 
    {
        public boolean hasArgPrefix = false;
        public boolean isKnownOption = false;
        public String  parsedString;

        public ArgumentToken(String arg)
        {
            if (arg == null)
                return;
            
            if (arg.startsWith(_argumentPrefix))
            {
                hasArgPrefix = true;
                String optionString = arg.substring(_argumentPrefix.length(), arg.length());
                if (_options.containsKey(optionString))
                {
                    parsedString = optionString;
                    isKnownOption = true;
                }
                else
                {
                    // Not a known option: leave _argumentPrefix (e.g. "-") as part of the value string
                    parsedString = arg;
                }
            }
            else
                parsedString = arg;
        }
        
        private static int argCount = 0;
        public static void clear()
        {
            argCount = 0;
        }
        
        public static ArgumentToken next(String[] argv)
        {
            if (argCount < argv.length)
            {
                ArgumentToken next = new ArgumentToken(argv[argCount]);
                ++argCount;
                return next;
            }
            else
                return null;
        }
    }
    
    /**
     * Parses the args.
     *
     * @param argv the argv
     */
    public static void parseArgs(String[] argv)
    {
        ArgumentToken.clear();
        ArgumentToken current = ArgumentToken.next(argv);
        while (current != null)
        {
            if (!current.hasArgPrefix) // all options start with prefix e.g. "-"
            {
                throw new IllegalArgumentException("Unknown option: " + current.parsedString);
            }
            
            if(!current.isKnownOption) // option must match the known options
            {
                throw new IllegalArgumentException("Unknown option: " + current.parsedString);
            }

            ArgumentToken next = ArgumentToken.next(argv);
            if ( next == null || 
                 (next != null && next.hasArgPrefix && next.isKnownOption) )
            {
                // Next token is a new -option (or end-of-line), so treat current 
                // -option as a boolean with true value
                List<String> paramValues = new ArrayList<String>(); 
                paramValues.add("true");
                _parameters.put(current.parsedString, paramValues);
                
                current = next;
                
                continue;
            }

            List<String> paramValues = _parameters.get(current.parsedString);
            if (paramValues == null)
                paramValues = new ArrayList<String>();
            paramValues.add(next.parsedString);
            _parameters.put(current.parsedString, paramValues);
            
            current = ArgumentToken.next(argv);
        }

        if(hasArg("help"))
        {
            System.out.println(optionHelpString());
            System.exit(0);
        }
        
        StringBuilder errors = new StringBuilder();
        if (reqdArgMissing(errors))
        {
                throw new IllegalArgumentException("Errors:" + errors);
        }
    }

    /**
     * Values.
     *
     * @param varName required command line variable (without the '-')
     * @return String value of the variable
     */
    public static List<String> values(String varName)
    {
        List<String> varValues = _parameters.get(varName);
        if (varValues != null)
            return varValues;

        //default values
        Option option = _options.get(varName);
        
        if (option == null || option.defaultValue == null || option.defaultValue.length == 0)
            return null;
        varValues = new ArrayList<String>(option.defaultValue.length);
        for (String defaultValue : option.defaultValue)
        {
            if (defaultValue != null)
                varValues.add(defaultValue);
        }
        return varValues.isEmpty() ? null : varValues;
    }

    /**
     * Value.
     *
     * @param varName required command line variable (without the '-')
     * @return String value of the variable
     * @throws NullPointerException if required variable was not in the command
     *             line arguments passed in to the constructor
     */
    public static String value(String varName)
    {
        List<String> values = values(varName);
        if (values == null)
            return null;
        return values.get(0);
    }

    /**
     * Boolean value.
     *
     * @param varName required command line variable (without the '-')
     * @return String value of the variable
     * @throws NullPointerException if required variable was not in the command
     *             line arguments passed in to the constructor
     */
    public static boolean booleanValue(String varName)
    {
        List<String> values = values(varName);
        if (values == null) 
            return hasArg(varName);
        return Boolean.parseBoolean(values.get(0));
    }
   
    /**
     * Int value.
     *
     * @param varName required command line variable (without the '-')
     * @return String value of the variable
     * @throws NumberFormatException if required variable string
     *             cannot be parsed as an integer
     */
    public static int intValue(String varName)
    {
        List<String> values = values(varName);
        if (values == null)
            return 0;
        return Integer.valueOf(values.get(0));
    }

    /**
     * Program name.
     *
     * @param progname Program name for the help string.
     */
    public static void programName(String progname)
    {
        _programName = progname;
    }

    private static boolean reqdArgMissing(StringBuilder missingReqdError)
    {
        boolean isReqdMissing = false;
        for (Map.Entry<String, Option> optionEntries : _options.entrySet())
        {
            if (optionEntries.getValue().isRequired && !_parameters.containsKey(optionEntries.getKey()))
            {
                isReqdMissing = true;
                missingReqdError.append("\nCommand line option: '" + optionEntries.getValue().arg + "' is required");
            }
        }
        return isReqdMissing;
    }

    private static class Option
    {
        StringBuilder buf = new StringBuilder(100);

        public Option(String a, String[] dv, String d, boolean isRequired)
        {
            arg = a;
            defaultValue = dv;
            description = d;
            this.isRequired = isRequired;
        }

        @Override
        public String toString()
        {
            buf.setLength(0);
            buf.append(" ");
            if(!isRequired)
                buf.append("[");
            buf.append("-");
            buf.append(arg);
            buf.append(" ");
          
            buf.append(" <");
            buf.append(description);
            if (defaultValue != null)
            {
                buf.append(". Default is:");
                boolean isFirst = true;
                for (String dStr : defaultValue)
                {
                    if (isFirst)
                    {
                        isFirst = false;
                    }
                    else
                    {
                        buf.append(",");
                    }
                    buf.append(dStr);
                }
            }

            buf.append(">");
            if(!isRequired)
                buf.append(" ]");
            buf.append("\n");
            return buf.toString();
        }

        private final String arg;
        private String[] defaultValue;
        private final String description;
        private boolean isRequired;
    }
}
