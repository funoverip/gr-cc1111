
Introduction
============

GNU Radio blocks to handle CC1111 based packet format (header, whitening and CRC16). 
Tested between RFCat and HackRF One.

- Packet Encoder (Source) read payload from gr.msg_queue(), given as parameter.
- Packet Decoder (Sink) send payload to gr.msg_queue(), given as parameter.

Status
======
- Beta version

Documentation
=============
- TODO
- See grc/*.png as flow graph examples
- See testing-scripts/* as complete examples (FSK modulation/demodulation between RFCat and HackRF One)



