#ifndef __VIRUS_AGENT__H
#define __VIRUS_AGENT__H

#include "enviro.h"
#include <string>
#include <chrono>
#include <ctime>

using namespace enviro;

class VirusController : public Process, public AgentInterface
{

public:
    // Define state/mode constants
    enum Mode
    {
        MOVING,
        TURNING,
        FLICKERING,
        INVISIBLE
    };

    VirusController() : Process(), AgentInterface(),
                        mode(MOVING), v(0), direction(0),
                        last_time(0), turn_start(0), flicker_start(0),
                        invisible_start(0), blink_on(false), last_blink_time(0),
                        init_slow_time(0) {}

    void init()
    {
        // Initial speed and direction
        v = 40 + rand() % 30; // Speed between 40-70
        direction = rand() % 360;

        // Slow start time
        init_slow_time = time(NULL);

        // Listen for game reset signal
        watch("game_over", [&](Event &e)
              {
            teleport(rand() % 300 - 150, rand() % 300 - 150, 0);
            mode = MOVING;
            decorate(""); // Clear any decorations
            // Reset slow time
            init_slow_time = time(NULL); });

        last_time = time(NULL);
        srand(time(NULL) + id()); // Ensure different viruses have different random behaviors
    }

    void start() {}

    void update()
    {
        // Detect sensor trigger obstacles - only react to static obstacles (walls)
        std::string reflection = sensor_reflection_type(0);
        if (sensor_value(0) < 30 && mode == MOVING && reflection != "Player")
        {
            mode = TURNING;
            turn_start = time(NULL);
            // Change direction
            direction += 120 + rand() % 120;
            if (direction >= 360)
            {
                direction -= 360;
            }
        }

        // Execute different behaviors according to the current mode
        switch (mode)
        {
        case MOVING:
            moving_behavior();
            break;
        case TURNING:
            turning_behavior();
            break;
        case FLICKERING:
            flickering_behavior();
            break;
        case INVISIBLE:
            invisible_behavior();
            break;
        }

        // Randomly trigger flashing
        time_t now = time(NULL);
        if (now - last_time >= 3 && mode == MOVING)
        {
            last_time = now;
            if ((rand() % 100) < 30)
            { // 30% chance to flash and become transparent
                mode = FLICKERING;
                flicker_start = now;
                last_blink_time = 0;
                blink_on = false;
                // Ensure it is visible when starting
                decorate("");
            }
        }
    }

    // Moving behavior
    void moving_behavior()
    {
        // Check if it's within the initial slow time (first 2 seconds)
        time_t now = time(NULL);
        double current_v = v;

        if (now - init_slow_time < 2)
        {
            // Speed halved in the first 2 seconds
            current_v = v * 0.5;
        }

        track_velocity(current_v, 0);
        // Ensure no decorations
        decorate("");
    }

    // Turning behavior
    void turning_behavior()
    {
        track_velocity(0, 1); // Rotate

        // Turn for a period of time
        if (time(NULL) - turn_start > 1)
        {
            teleport(position().x, position().y, direction * M_PI / 180);
            mode = MOVING;
        }
    }

    // Flashing behavior - using transparency
    void flickering_behavior()
    {
        track_velocity(v, 0); // Continue moving

        // Get current timestamp (in milliseconds)
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long long current_time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

        // Check if still within flashing time (1 second)
        if (ts.tv_sec - flicker_start < 1)
        {
            // Switch state every 100 milliseconds
            if (current_time - last_blink_time > 100)
            {
                blink_on = !blink_on;
                last_blink_time = current_time;

                if (blink_on)
                {
                    // Flashing on - use semi-transparent
                    decorate("<circle cx='0' cy='0' r='20' fill='rgba(255,255,255,0.7)' stroke='rgba(255,255,255,0.5)'/>");
                }
                else
                {
                    // Flashing off - show original color
                    decorate("");
                }
            }
        }
        else
        {
            // Flashing ends, enter invisible phase
            mode = INVISIBLE;
            invisible_start = ts.tv_sec;

            // Change display status to white overlay (pure white, simulating transparency)
            // Note: The circle here must be slightly larger than the virus to completely cover it
            decorate("<circle cx='0' cy='0' r='18' fill='white' stroke='white'/>");
        }
    }

    // Invisible state - maintain white overlay
    void invisible_behavior()
    {
        track_velocity(v, 0); // Continue moving

        // Maintain invisibility for 3 seconds
        time_t now = time(NULL);
        if (now - invisible_start > 3)
        {
            // Return to normal state
            mode = MOVING;
            decorate("");
        }
    }

    void stop() {}

private:
    Mode mode;
    double v;
    double direction;
    time_t last_time;
    time_t turn_start;
    time_t flicker_start;
    time_t invisible_start;
    bool blink_on;
    long long last_blink_time;
    time_t init_slow_time;
};

class Virus : public Agent
{
public:
    Virus(json spec, World &world) : Agent(spec, world)
    {
        add_process(c);
    }

private:
    VirusController c;
};

DECLARE_INTERFACE(Virus)

#endif