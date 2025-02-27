#ifndef __PLAYER_AGENT__H
#define __PLAYER_AGENT__H

#include "enviro.h"
#include <cmath>
#include <string>

using namespace enviro;

class PlayerController : public Process, public AgentInterface
{

public:
    PlayerController() : Process(), AgentInterface(),
                         moving(false), target_x(0), target_y(0),
                         score(0), normal_speed(100) {}

    void init()
    {
        watch("screen_click", [this](Event e)
              {
                  target_x = e.value()["x"];
                  target_y = e.value()["y"];
                  moving = true;

                  // Immediately update the angle toward the target
                  double dx = target_x - position().x;
                  double dy = target_y - position().y;
                  double angle = atan2(dy, dx);
                  teleport(position().x, position().y, angle); // Immediately turn to the target
              });

        notice_collisions_with("Virus", [&](Event &e)
                               {
            // When player collides with virus, emit game over event
            emit(Event("game_over"));
            // Reset score
            score = 0;
            update_score_display(); });

        // Listen for game reset event, reset position to bottom right
        watch("game_over", [&](Event &e)
              { teleport(200, 200, 0); });

        // Listen for add point event
        watch("add_point", [&](Event &e)
              {
            // Increase score
            score++;
            update_score_display(); });

        // Initialize score display
        update_score_display();
    }

    void start()
    {
        // At game start, player is in the bottom right
        teleport(200, 200, 0);
    }

    void update()
    {
        if (moving)
        {
            // Move directly forward, no need to rotate
            track_velocity(normal_speed, 0);

            // Check if near target
            double dx = position().x - target_x;
            double dy = position().y - target_y;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance < 5)
            {
                moving = false;
                track_velocity(0, 0); // Stop moving
            }
        }
    }

    // Update score display
    void update_score_display()
    {
        std::string score_text = "Score: " + std::to_string(score);
        label(score_text, 0, -25); // Display score above player
    }

    void stop() {}

private:
    bool moving;
    double target_x, target_y;
    int score;
    double normal_speed;
};

class Player : public Agent
{
public:
    Player(json spec, World &world) : Agent(spec, world)
    {
        add_process(c);
    }

private:
    PlayerController c;
};

DECLARE_INTERFACE(Player)

#endif