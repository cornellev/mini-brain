#!/bin/bash

set -e

mv ros2_ws og_ros2_ws_backup_$(date +%Y%m%d_%H%M%S)

git clone -b the-mini-jetson https://github.com/cornellev/mini-brain.git

echo "Setup complete. The new ROS2 workspace is ready."