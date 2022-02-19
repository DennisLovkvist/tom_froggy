#pragma once
#include <string>
#include <box2d/box2d.h>
struct UserData
{
	public:
	std::string name;
    int collsisions;
	b2Body* connected_platform;
   	bool connect;
};