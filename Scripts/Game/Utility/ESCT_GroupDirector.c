class ESCT_GroupDirector
{
	static ESCT_EGroupSize GetGroupSize(ESCT_ELocationType locationType = ESCT_ELocationType.Undefined, bool excludeSentry = false)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return null;

		int playerCount = playerManager.GetPlayerCount();
		if (playerCount == 0)
			return ESCT_EGroupSize.Undefined;
		
		float sentryWeight = 0.4 - (playerCount - 1) * 0.03;
		float fireteamWeight = 0.3 + (playerCount - 1) * 0.02;
		float squadWeight = 0.1 + (playerCount - 1) * 0.05;

		if (locationType == ESCT_ELocationType.Helipad)
		{
			fireteamWeight = 0.3 + (playerCount - 1) * 0.04;
			squadWeight = 0.25 + (playerCount - 1) * 0.075;
		}
		else if (locationType == ESCT_ELocationType.StartArea || locationType == ESCT_ELocationType.Checkpoint)
		{
			sentryWeight = 0.4 + (playerCount - 1) * 0.08;
			fireteamWeight = 0.3 + (playerCount - 1) * 0.05;
		}
		else if (locationType == ESCT_ELocationType.City)
		{
			fireteamWeight = 0.3 + (playerCount - 1) * 0.04;
		}

		if (excludeSentry)
			sentryWeight = 0;
		
		array<float> sizeWeights = {sentryWeight, fireteamWeight, squadWeight};
		array<ESCT_EGroupSize> sizes = {ESCT_EGroupSize.Sentry, ESCT_EGroupSize.Fireteam, ESCT_EGroupSize.Squad};
		int index = SCR_ArrayHelper.GetWeightedIndex(sizeWeights, Math.RandomFloat01());

		return sizes[index];
	}
}