#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

using namespace std;

enum EType {
    TINT,
    TFLOAT,
    TVEC
};

class Environment
{
public:
    Environment(int argc, char *argv[]);
    float getFloat(const char * value);
    int32_t getInt(const char * value);
    glm::vec3 getVec(const char * value);
	const string& getPath() const;

private:
    void addRecord(unsigned char type, const string & name, const string & initValue);
    void addRecord(const string & name, const int32_t & initValue);
    void addRecord(const string & name, const float & initValue);
    void addRecord(const string & name, const float & x, const float & y, const float & z);

    unordered_map<string, int32_t> mapInt;
    unordered_map<string, float> mapFloat;
    unordered_map<string, glm::vec3> mapVec;

	string path;
};