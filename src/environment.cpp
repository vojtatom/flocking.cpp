#include <unordered_map>
#include <iostream>
#include <glm/glm.hpp>
#include "environment.hpp"

using namespace std;

Environment::Environment(int argc, char *argv[])
{
    int apptype = 0;
    int boidsCountPower = 10;

    if (argc > 1)
        apptype = atoi(argv[1]);

    if (argc > 2)
        boidsCountPower = max(1, atoi(argv[2]));

    //general settings
    addRecord("appType", apptype);
    addRecord("boidCount", 1 << boidsCountPower);
    addRecord("spaceLow", -1000, -1000, -1000);
    addRecord("spaceHigh", 1000, 1000, 1000);
    addRecord("grid", 50, 50, 50);

    //flocking
    addRecord("flockingZone", 40.0f);
    addRecord("speedFactor", 2.0f);
    addRecord("forceLimit", 0.1f);

    //rendering
    addRecord("treeMemoryLimit", 4096);
    addRecord("boidSize", 2.f);

	size_t i = 0, j = 0;
	while (true)
	{
		if (argv[0][i] == '\0')
			break;

		if (argv[0][i] == '\\' || argv[0][i] == '/')
			j = i + 1;
		i++;
	}

	path = string(argv[0], j);
	cout << path << endl;

    //todo READ file
}


void Environment::addRecord(unsigned char type, const string & name, const string & initValue)
{
    switch (type)
    {
    case EType::TINT:
        mapInt[name] = atoi(initValue.c_str());
        break;
    case EType::TFLOAT:
        mapFloat[name] = atof(initValue.c_str());
        break;
    default:
        break;
    }
}

void Environment::addRecord(const string & name, const int32_t & initValue)
{
    mapInt[name] = initValue;
}


void Environment::addRecord(const string & name, const float & initValue)
{
    mapFloat[name] = initValue;
}

void Environment::addRecord(const string & name, const float & x, const float & y, const float & z)
{
    mapVec[name] = glm::vec3(x, y, z);
}

float Environment::getFloat(const char * value)
{
    auto it = mapFloat.find(string(value));
    if (it != mapFloat.end())
        return it->second;
    return 0.0f;
}

int32_t Environment::getInt(const char * value)
{
    auto it = mapInt.find(string(value));
    if (it != mapInt.end())
        return it->second;
    return 0;
}

glm::vec3 Environment::getVec(const char * value)
{
    auto it = mapVec.find(string(value));
    if (it != mapVec.end())
        return it->second;
    return glm::vec3(0.0f);
}

const string& Environment::getPath() const
{
	return path;
}