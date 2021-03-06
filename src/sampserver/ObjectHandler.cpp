#include "ObjectHandler.h"
#include <sstream>
#include "Object.h"
#include "Interior.h"
#include "Player.h"
#include "InteriorHandler.h"
#include "GameUtility.h"
#include "Player.h"
#include "PlayerHandler.h"
#include "MySQLFunctions.h"
#include "PlayerObject.h"

ObjectHandler::ObjectHandler(void)
{
	objects = new std::map<int, Object*>();
}


ObjectHandler::~ObjectHandler(void)
{
}

bool ObjectHandler::OnCommand(Player *player, std::string cmd, std::vector<std::string> args, GameUtility *gameUtility)
{
	if(cmd == "createobject")
	{
		if(args.size() >= 1)
		{
			float *x = new float(); float *y = new float(); float *z = new float();
			player->GetPos(x, y, z);
			int model = atoi(args[0].c_str());
			Object *object = this->CreateObject(model, player, gameUtility->interiorHandler->getInterior(GetPVarInt(player->getId(), "currentinterior")), *x, *y, *z, 0, 0, 0, 200);
			std::stringstream s;
			s << "Created object with ID " << object->getId();
			SendClientMessage(player->getId(), 0xFFFFFFFF, s.str().c_str());
			delete x, y, z;
		}
		else
			SendClientMessage(player->getId(), 0xFFFFFFFF, "Usage: /createobject <model>");
		return true;
	}
	else if(cmd == "selectobject")
	{
		if(args.size() == 0)
			SelectObject(player->getId());
		else
		{
			int id = atoi(args[0].c_str());
			SetPVarInt(player->getId(), "selectedobject", id);
			std::stringstream s;
			s << "Selected object " << id;
			SendClientMessage(player->getId(), 0xFFFFFFFF, s.str().c_str());
		}
		return true;
	}
	else if(cmd == "getcloseobjects")
	{
		for(auto it = objects->begin(); it != objects->end(); it++)
		{
			if(GameUtility::IsPlayerClose(player, it->second->getX(), it->second->getY(), it->second->getZ(), it->second->getInterior(), 5))
			{
				std::stringstream s;
				s << "Object " << it->first << ": Model:" << it->second->getModel() << " DrawDistance:" << it->second->getDrawDistance();
				SendClientMessage(player->getId(), 0xFFFFFFFF, s.str().c_str());
			}
		}
	}
	else if(cmd == "editobject")
	{
		int selectedObject = GetPVarInt(player->getId(), "selectedobject");
		if(selectedObject != -1)
			EditPlayerObject(player->getId(), selectedObject);
		else
			SendClientMessage(player->getId(), 0xFFFFFFFF, "You do not have any object selected.");
		return true;
	}
	else if(cmd == "removeobject")
	{
		int selectedObject = GetPVarInt(player->getId(), "selectedobject");
		if(selectedObject != -1)
		{
			Object *object = nullptr;
			for(auto it = objects->begin(); it != objects->end(); it++)
			{
				if(it->second->HasObject(selectedObject))
					object = it->second;
			}
			if(object != nullptr)
			{
				RemoveObject(object->getId());
				SendClientMessage(player->getId(), 0xFFFFFFFF, "Object removed");
			}
			else
				SendClientMessage(player->getId(), 0xFFFFFFFF, "Could not remove object. Selected object not found?");
		}
		else
			SendClientMessage(player->getId(), 0xFFFFFFFF, "You do not have any object selected.");
		return true;
	}
	return false;
}

void ObjectHandler::CheckForHacks()
{
}

void ObjectHandler::Load(GameUtility* gameUtility)
{
	sql::ResultSet *res = MySQLFunctions::ExecuteQuery("SELECT * FROM objects");
	while (res->next())
	{
		int objectId = res->getInt("id");
		Object *object = new Object(
			res->getInt("id"),
			res->getInt("model"), 
			gameUtility->interiorHandler->getInterior(res->getInt("interior")), 
			res->getDouble("x"), 
			res->getDouble("y"), 
			res->getDouble("z"), 
			res->getDouble("rx"), 
			res->getDouble("ry"), 
			res->getDouble("rz"),
			res->getDouble("drawdistance"));
		objects->emplace(object->getId(), object);
	}
	delete res;
}

Object *ObjectHandler::CreateObject(int model, Player* player, Interior *interior, float x, float y, float z, float rx, float ry, float rz, float drawDistance)
{
	int objectId = getFreeObjectId();
	Object *object = new Object(objectId, model, interior, x, y, z, rx, ry, rz, drawDistance);
	objects->emplace(object->getId(), object);
	sql::PreparedStatement *statement = MySQLFunctions::con->prepareStatement("INSERT INTO objects(id, interior, model, x, y, z, rx, ry, rz, drawdistance) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
	statement->setInt(1, objectId);
	statement->setInt(2, interior->interiorId_);
	statement->setInt(3, model);
	statement->setDouble(4, x);
	statement->setDouble(5, y);
	statement->setDouble(6, z);
	statement->setDouble(7, rx);
	statement->setDouble(8, ry);
	statement->setDouble(9, rz);
	statement->setDouble(10, drawDistance);
	MySQLFunctions::ExecutePreparedQuery(statement);
	return object;
}

void ObjectHandler::RemoveObject(int id)
{
	Object *object = getObject(id);
	object->Destroy();
	delete object;
	objects->erase(id);
	sql::PreparedStatement *s = MySQLFunctions::con->prepareStatement("DELETE FROM `samp`.`objects` WHERE `id`=?");
	s->setInt(1, id);
	MySQLFunctions::ExecutePreparedQuery(s);
}

std::vector<Object*> *ObjectHandler::getCloseObjects(float x, float y, float z, float maxDistance)
{
	return nullptr;
}

int ObjectHandler::getFreeObjectId()
{
	int openSlot = -1;
	for(int i = 0; true; i++)
	{
		if(objects->find(i) == objects->end())
		{
			openSlot = i;
			break;
		}
	}
	return openSlot;
}

Object *ObjectHandler::getObject(int objectId)
{
	if(objects->find(objectId) != objects->end())
	{
		return objects->at(objectId);
	}
	return nullptr;
}

void ObjectHandler::Update(GameUtility *gameUtility)
{
	std::map<int, Player*> *players = gameUtility->playerHandler->players;
	for(auto it = players->begin(); it != players->end(); it++)
	{
		Player *player = it->second;
		for(auto i = objects->begin(); i != objects->end(); i++)
		{
			Object *object = i->second;
			if(!object->HasPlayerObject(player->getId()))
			{
				if(GameUtility::IsPlayerClose(player, object->getX(), object->getY(), object->getZ(), object->getInterior(), object->getDrawDistance()))
				{
					PlayerObject temp = PlayerObject::Create(player->getId(), object->getModel(), object->getX(), object->getY(), object->getZ(), object->getRotX(), object->getRotY(), object->getRotZ(), object->getDrawDistance());
					object->AddPlayerObject(player->getId(), temp.GetObjectId());

					std::stringstream s;
					s << "Object " << object->getId() << " was streamed in";
					SendClientMessage(player->getId(), 0xFFFFFFFF, s.str().c_str());
				}
			}
			else if(!IsPlayerObjectMoving(player->getId(), GetPVarInt(player->getId(), "selectedobject")))
			{
				if(!GameUtility::IsPlayerClose(player, object->getX(), object->getY(), object->getZ(), object->getInterior(), object->getDrawDistance()))
				{
					object->RemovePlayerObject(player->getId());

					std::stringstream s;
					s << "Object " << object->getId() << " was streamed out";
					SendClientMessage(player->getId(), 0xFFFFFFFF, s.str().c_str());
				}
			}
		}
	}
}

bool ObjectHandler::OnPlayerSelectObject(Player *player, int type, int objectSampId, int model, float x, float y, float z)
{
	if(type == SELECT_OBJECT_PLAYER_OBJECT)
	{
		SetPVarInt(player->getId(), "selectedobject", objectSampId);
		std::stringstream s;
		s << "Selected object " << objectSampId;
		SendClientMessage(player->getId(), 0xFFFFFFFF, s.str().c_str());
	}
	return true;
}

bool ObjectHandler::OnPlayerEditObject(Player *player, int playerobject, int objectSampId, int response, float xo, float yo, float zo, float xr, float yr, float zr)
{
	if(playerobject)
	{
		for(auto it = objects->begin(); it != objects->end(); it++)
		{
			if(it->second->HasObject(objectSampId))
			{
				if(response = EDIT_RESPONSE_FINAL)
				{
					it->second->UpdatePosition(false, xo, yo, zo, xr, yr, zr);
					//std::cout << "Updated after object moved" << std::endl;
				}
				else if(response == EDIT_RESPONSE_CANCEL ||response == EDIT_RESPONSE_UPDATE)
				{
					it->second->UpdatePosition(false, it->second->getX(), it->second->getY(), it->second->getZ(), it->second->getRotX(), it->second->getRotY(), it->second->getRotZ());
					//std::cout << "Updated after cancel/exit" << std::endl;
				}
				break;
			}
		}
	}
	return true;
}

bool ObjectHandler::OnPlayerObjectMoved(Player *player, int objectSampId)
{
	return true;
}
