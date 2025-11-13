FROM ros:humble

# ENV DEBIAN_FRONTEND=noninteractive
ENV ROS_DISTRO=humble

# install some common tools
RUN apt-get update && apt-get install -y \
    vim \
    git \
    python3-pip \
    wget \
    curl \
    build-essential \
    cmake \
    tmux \
    && rm -rf /var/lib/apt/lists/*

# install colcon and other python tools
RUN pip3 install -U \
    colcon-common-extensions \
    colcon-metadata \
    colcon-ros \
    colcon-bundle

# create a user to avoid running as root
ARG username=rosuser
ARG userid=1000
ARG USER_GID=1000

RUN groupadd -g ${USER_GID} ${username} \
    && useradd -m -u ${userid} -g ${USER_GID} -s /bin/bash ${username} \
    && apt-get update \
    && apt-get install -y sudo \
    && echo "${username} ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers \
    && rm -rf /var/lib/apt/lists/*

# add ackermann msgs and other packages
RUN apt-get update && apt-get install -y \
    ros-${ROS_DISTRO}-ackermann-msgs \
    ros-${ROS_DISTRO}-joy \
    && rm -rf /var/lib/apt/lists/*

USER ${username}
WORKDIR /home/${username}/ros2_ws
RUN mkdir -p src
COPY --chown=rosuser:rosuser ./src ./src

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /home/${username}/.bashrc

# verify colcon
#RUN colcon version-check
#RUN /bin/bash -c "source /opt/ros/${ROS_DISTRO}/setup.bash && colcon build --symlink-install"
# RUN echo "source /home/${username}/ros2_ws/install/setup.bash" >> /home/${username}/.bashrc

COPY --chown=rosuser:rosuser ./entrypoint.sh /home/rosuser/entrypoint.sh
COPY --chown=rosuser:rosuser ./jetson.sh /home/rosuser/jetson.sh
RUN chmod +x /home/rosuser/entrypoint.sh

ENTRYPOINT ["/bin/bash", "/home/rosuser/entrypoint.sh"]

CMD ["bash"]