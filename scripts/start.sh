#!/bin/bash

# first check if js0 exists
if [ ! -d "/dev/input/js0" ]; then
    echo "Joystick not found! Please connect a joystick and try again."
fi

# start the main application
echo "Starting the application..."
docker run -it --rm --device=/dev/input/js0:/dev/input/js0 --user rosuser mini-brain /bin/bash