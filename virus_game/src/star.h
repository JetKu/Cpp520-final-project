#ifndef __STAR_AGENT__H
#define __STAR_AGENT__H

#include "enviro.h"
#include <string>
#include <vector>
#include <ctime>

using namespace enviro;

class StarController : public Process, public AgentInterface
{

public:
    StarController() : Process(), AgentInterface(),
                       rotation_speed(0.5), last_move_time(0) {}

    void init()
    {
        // where the star can appear
        positions = {
            {-150, -150},
            {150, -150},
            {-150, 150},
            {150, 150},
            {0, -180},
            {-180, 0},
            {180, 0},
            {0, 180}};

        move_to_random_position();

        notice_collisions_with("Player", [&](Event &e)
                               {

            emit(Event("add_point"));
            
            move_to_random_position(); });

        watch("game_over", [&](Event &e)
              { move_to_random_position(); });

        last_move_time = time(NULL);
    }

    void start() {}

    void update()
    {
        // slowly rotate
        omni_apply_force(0, 0);
        apply_force(0, rotation_speed);

        // move randomly every 10 seconds
        time_t now = time(NULL);
        if (now - last_move_time >= 10)
        {
            move_to_random_position();
            last_move_time = now;
        }
    }

    void move_to_random_position()
    {
        // randomly select a position
        int index = rand() % positions.size();
        teleport(positions[index].first, positions[index].second, 0);
    }

    void stop() {}

private:
    double rotation_speed;
    time_t last_move_time;
    std::vector<std::pair<double, double>> positions;
};

class Star : public Agent
{
public:
    Star(json spec, World &world) : Agent(spec, world)
    {
        add_process(c);
    }

private:
    StarController c;
};

DECLARE_INTERFACE(Star)

#endif