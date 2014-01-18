#pragma once
#include <iostream>
#include <map>
#include "MyPlayer.h"
#include "Handler.h"
#include <sampgdk\a_samp.h>
class WorldPositionObject;
class PlayerHandler : public Handler
{
public:
	PlayerHandler();
	~PlayerHandler();
	virtual bool OnCommand(MyPlayer *player, std::string cmd, std::vector<std::string> args, GameUtility *gameUtility);
	virtual void CheckForHacks();
	virtual void Load();
	void SAMPGDK_CALL KickDelayed(int, void * playerId);
	void TeleportPlayer(int playerid, WorldPositionObject pos);
	std::map<int, MyPlayer*> *players;
};
