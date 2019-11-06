#pragma once
#include <random>

using namespace std;

struct DNA
{
    void set(uniform_real_distribution<float> &dist, mt19937 & gen)
    {
        force = dist(gen) * 0.4f + 0.8f;
        speed = dist(gen) + 1.5f;
        moodiness = dist(gen) * 0.1f + 0.001f;
        mentalEndurance = dist(gen) * 2 + 1;
        mentalHealth = dist(gen) * 20 - 10;
        moodLimit = 10;
    }

    float force;
    float speed;
    float moodiness;
    float mentalEndurance;
    float mentalHealth;
    float moodLimit;
};
