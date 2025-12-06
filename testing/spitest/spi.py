import spidev, time

spi = spidev.SpiDev()
spi.open(0, 0)          # /dev/spidev0.0
spi.mode = 0
spi.max_speed_hz = 500000
spi.bits_per_word = 8

while True:
    tx = [0xAA] + list(range(1, 18))  # 18 bytes total
    rx = spi.xfer2(tx)
    print("TX:", [hex(b) for b in tx])
    print("RX:", [hex(b) for b in rx])  # ignore for now
    time.sleep(0.5)

spi.close()