class ESCT_Logger
{
	private static const string logPrefix = "[Escapists]";
	
	static void DebugFormat(string message, string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty, string param4 = string.Empty, string param5 = string.Empty, string param6 = string.Empty, string param7 = string.Empty, string param8 = string.Empty, string param9 = string.Empty)
    {
		string originString = string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9);
		Print(string.Format("%1 %2", logPrefix, originString), LogLevel.DEBUG);
    }
	
	static void InfoFormat(string message, string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty, string param4 = string.Empty, string param5 = string.Empty, string param6 = string.Empty, string param7 = string.Empty, string param8 = string.Empty, string param9 = string.Empty)
    {
		string originString = string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9);
		Print(string.Format("%1 %2", logPrefix, originString), LogLevel.NORMAL);
    }
	
	static void ErrorFormat(string message, string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty, string param4 = string.Empty, string param5 = string.Empty, string param6 = string.Empty, string param7 = string.Empty, string param8 = string.Empty, string param9 = string.Empty)
    {
		string originString = string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9);
		Print(string.Format("%1 %2", logPrefix, originString), LogLevel.ERROR);
    }
	
	static void WarningFormat(string message, string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty, string param4 = string.Empty, string param5 = string.Empty, string param6 = string.Empty, string param7 = string.Empty, string param8 = string.Empty, string param9 = string.Empty)
    {
		string originString = string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9);
		Print(string.Format("%1 %2", logPrefix, originString), LogLevel.WARNING);
    }
	
	static void Error(string message)
	{
		Print(string.Format("%1 %2", logPrefix, message), LogLevel.ERROR);
	}
	
	static void Warning(string message)
	{
		Print(string.Format("%1 %2", logPrefix, message), LogLevel.WARNING);
	}
	
    static void Info(string message)
    {
        Print(string.Format("%1 %2", logPrefix, message), LogLevel.NORMAL);
    }
	
	static void Debug(string message)
	{
		Print(string.Format("%1 %2", logPrefix, message), LogLevel.DEBUG);
	}
}
