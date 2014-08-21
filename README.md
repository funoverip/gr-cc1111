
Introduction
============

GNU Radio blocks to handle CC1111 based packet format (header, whitening and CRC16). 
Tested between RFCat and HackRF One.

- Packet Encoder (Source) read payload from gr.msg_queue(), given as parameter.
- Packet Decoder (Sink) send payload to gr.msg_queue(), given as parameter.

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

