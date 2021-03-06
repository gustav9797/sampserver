#pragma once
#include <iostream>
#include <vector>

class PlayerHandler;
class InteriorHandler;
class HouseHandler;
class WeaponHandler;
class VehicleHandler;
class ObjectHandler;
class RegisterLoginHandler;

class Handler;
class Player;
class WorldPositionObject;
class Interior;

class GameUtility
{
private:
	std::vector<Handler*> *handlers;
public:
	GameUtility(void);
	~GameUtility(void);

	bool OnCommand(Player *player, std::string cmd, std::vector<std::string> args, GameUtility *gameUtility);
	void CheckForHacks();
	void Load(GameUtility* gameUtility);

	void AddHandler(Handler *handler);
	static bool IsPlayerClose(Player *player, WorldPositionObject object, float range);
	static bool IsPlayerClose(Player *player, float x, float y, float z, Interior *interior, float range);

	PlayerHandler *playerHandler;
	InteriorHandler *interiorHandler;
	HouseHandler *houseHandler;
	WeaponHandler *weaponHandler;
	VehicleHandler *vehicleHandler;
	ObjectHandler *objectHandler;
	RegisterLoginHandler *registerLoginHandler;
};

