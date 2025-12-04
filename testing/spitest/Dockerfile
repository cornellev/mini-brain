FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential wget ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /root

# Get and build the kernel's spidev_test utility
RUN wget https://raw.githubusercontent.com/torvalds/linux/v5.15/tools/spi/spidev_test.c \
    && gcc -o spidev_test spidev_test.c


CMD ["bash"]
