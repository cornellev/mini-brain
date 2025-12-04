#!/bin/bash

# start the main application
echo "Starting the application..."
docker run -it --rm --privileged -v /dev:/dev --user rosuser mini-brain /home/rosuser/ros2_ws/entry.sh /bin/bash