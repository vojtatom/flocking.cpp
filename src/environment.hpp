#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

using namespace std;

/**
 * @brief Types of values in the enviroment file
 */
enum EType {
    /**
     * @brief Integer type
     */
    TINT,
    /**
     * @brief Float type
     */
    TFLOAT,
    /**
     * @brief Vector type
     */
    TVEC
};

/**
 * @brief User environment storing user settings
 */
class Environment
{
public:
    /**
     * @brief Construct a new Environment object
     * 
     * Optional commandline args, overrides the environment file:
     * argv[0]: environment file
     * argv[1]: app type (0-3)
     * argv[2]: exponent of number of boids (powers of two)
     * 
     * @param argc Number of commandline arguments
     * @param argv Command line arguments
     */
    Environment(int argc, char *argv[]);

    /**
     * @brief Get float value
     * 
     * @param value Name of float value to be looked up
     * @return float 
     */
    float getFloat(const char * value);
    /**
     * @brief Get integer value
     * 
     * @param value Name of int32_t value to be looked up
     * @return int32_t 
     */
    int32_t getInt(const char * value);
    /**
     * @brief Get vector value
     * 
     * @param value Name of glm::vec3 value to be looked up
     * @return glm::vec3 
     */
    glm::vec3 getVec(const char * value);

    /**
     * @brief Get the run path
     * @return const string& 
     */
	const string& getPath() const;

private:

    //void addRecord(unsigned char type, const string & name, const string & initValue);

    /**
     * @brief Add integer value to the environment
     * 
     * @param name Name of the value
     * @param initValue Integer value
     */
    void addRecord(const string & name, const int32_t & initValue);

    /**
     * @brief Add float value to the environment
     * 
     * @param name Name of the value
     * @param initValue Float value
     */
    void addRecord(const string & name, const float & initValue);

    /**
     * @brief Add vector value to the environment
     * 
     * @param name Name of the value
     * @param x x coordinate of the vector
     * @param y y coordinate of the vector
     * @param z z coordinate of the vector
     */
    void addRecord(const string & name, const float & x, const float & y, const float & z);

    /**
     * @brief Map of the integer values
     */
    unordered_map<string, int32_t> mapInt;

    /**
     * @brief Map of the float values
     */
    unordered_map<string, float> mapFloat;

    /**
     * @brief Map of the vector values
     */
    unordered_map<string, glm::vec3> mapVec;

    /**
     * @brief Run path
     */
	string path;
};