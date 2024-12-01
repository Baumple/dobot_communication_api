#pragma once

#include <stdint.h>
// Struct containing information about the bot's location.
struct DobotPose {
  float x;             // Robotic arm coordinate x
  float y;             // Robotic arm coordinate y
  float z;             // Robotic arm cooridnate z
  float r;             // Robotic arm cooridnate r
  float jointAngle[4]; // Robotica rm 4 axis (basement, rear arm, forearm,
                       // EndEffector) angles
};

// Device version
struct DobotVersion {
  uint8_t major;
  uint8_t minor;
  uint8_t revision;
};
