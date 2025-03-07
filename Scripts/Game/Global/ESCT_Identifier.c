class ESCT_IdentifierGenerator
{
	static string GenerateStringId(int size = 10)
	{
		const string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		const int mask = int.MAX;

		string id = "";
		for (int i = 0; i < size; i++) 
		{
			int randomValue = Math.RandomInt(0, mask);
			id += alphabet[randomValue % alphabet.Length()];
		}
		
		return id;
	}

	static int GenerateIntId(int size = 6)
	{
		const string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		const int mask = int.MAX;

		string id = "";
		for (int i = 0; i < size; i++) 
		{
			int randomValue = Math.RandomInt(0, mask);
			id += alphabet[randomValue % alphabet.Length()];
		}
		
	    // Hash the string ID to an integer value
	    int hash = 0;
	    for (int i = 0; i < id.Length(); i++) 
		{
	        hash = (hash * 31 + alphabet.IndexOf(id[i])) % mask;
	    }
		
		return hash;
	}
}
