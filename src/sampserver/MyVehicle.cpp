#include "MyVehicle.h"


MyVehicle::MyVehicle(int id, int model, float x, float y, float z, float rotation, int color1, int color2, int respawnDelay, std::string licensePlate) : Vehicle(id)
{
	id_ = id;
	model_ = model;
	x_ = x;
	y_ = y;
	z_ = z;
	rotation_ = rotation;
	color1_ = color1;
	color2_ = color2;
	respawnDelay_ = respawnDelay;
	licensePlate_ = licensePlate;
}


MyVehicle::MyVehicle(Vehicle vehicle, int color1, int color2, int respawnDelay, std::string licensePlate) : Vehicle(vehicle)
{
	id_ = vehicle.GetId();
	model_ = vehicle.GetModel();
	float *tempX = new float();
	float *tempY = new float();
	float *tempZ = new float();
	vehicle.GetPos(tempX, tempY, tempZ);
	x_ = *tempX;
	y_ = *tempY;
	z_ = *tempZ;
	delete tempX, tempY, tempZ;
	rotation_ = vehicle.GetZAngle();
	licensePlate_ = licensePlate;
	color1_ = color1;
	color2_ = color2;
	respawnDelay_ = respawnDelay;
}


MyVehicle::~MyVehicle()
{
}