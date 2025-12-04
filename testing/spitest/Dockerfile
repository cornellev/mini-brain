FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential wget ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /root

# Get and build the kernel's spidev_test utility
RUN wget https://raw.githubusercontent.com/torvalds/linux/master/tools/spi/spidev_test.c \
 && printf '\n#ifndef SPI_RX_CPHA_FLIP\n#define SPI_RX_CPHA_FLIP 0\n#endif\n#ifndef SPI_MOSI_IDLE_LOW\n#define SPI_MOSI_IDLE_LOW 0\n#endif\n' >> spidev_test.c \
 && gcc -o spidev_test spidev_test.c


CMD ["bash"]
