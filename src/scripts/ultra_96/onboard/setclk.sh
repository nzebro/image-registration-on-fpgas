#!/bin/sh

# Usage: setclk.sh <freq-in-MHz>

# Set the clock frequency for fclk0
# Not all frequencies are supported due to how the PLLs work, the actual
# set frequency will be displayed.


CLK_FREQ=$1
CLK_NAME=fclk2_mhz

echo "[INFO] Prev frequency was $PREV_FREQ"
PREV_FREQ=sudo python3 -c "from pynq.ps import Clocks; import sys; sys.stdout.write(str(Clocks.$CLK_NAME))"
echo $PREV_FREQ

echo "[INFO] Setting frequency to $CLK_FREQ"
sudo python3 -c "from pynq.ps import Clocks; Clocks.$CLK_NAME = $CLK_FREQ; print(Clocks.$CLK_NAME)"