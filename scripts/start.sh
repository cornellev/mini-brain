#!/bin/bash

# start the main application
echo "Starting the application..."
docker run -it --rm --privileged -v /dev/input:/dev/input --user rosuser mini-brain /bin/bash