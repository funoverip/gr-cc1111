
Introduction
============

GNU Radio blocks to handle CC1111 based packet format (header, whitening and CRC16).
Tested between RFCat and HackRF One.

Provide the following GNU Radio blocks:
- "Packet Encoder (CC1111) Source" : Read payloads from gr.msg_queue() and format them to CC11xx based packets.
- "Packet Decoder (CC1111)" : Decode CC11xx formatted packets from a GR flow graph, and send payload to gr.msg_queue().

Author
======
- Jerome Nokin 
- http://funoverip.net
- @funoverip.net

Status
======
- Beta version
- Tested on GNURadio 3.7.4

Documentation
=============
- TODO
- Some more explanations can be found here: http://funoverip.net/2014/07/gnu-radio-cc1111-packets-encoderdecoder-blocks/
- See grc/*.png as flow graph examples
- See testing-scripts/* as complete examples (FSK modulation/demodulation between RFCat and HackRF One)

Installation
============

```
cd src/gr-cc1111
mkdir build
cd build
cmake ../
make 
sudo make install
```

