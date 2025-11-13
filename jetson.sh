#!/bin/bash

set -e

mv ../ros2_ws ../og_ros2_ws
git clone -b the-mini-jetson https://github.com/cornellev/mini-brain.git
mv mini-brain ../ros2_ws

echo "Setup complete. The new ROS2 workspace is ready."