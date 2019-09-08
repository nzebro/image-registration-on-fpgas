#!/bin/sh
# Usage: load_bitfile.sh [optional different bitstream name]

if [ "$1" ]; then
        BIT="$1"
else
        BIT="iron_wrapper.bit"
fi

echo "Downloading bitstream named $BIT"
sudo python3 -c "from pynq import Overlay; o = Overlay('$BIT'); o.download()"