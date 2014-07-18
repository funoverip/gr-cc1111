
Introduction
============

GnuRadio blocks to handle CC1111 based packet format, whitening and CRC16.
Typically, "<len (1 byte)><payload><CRC16 (2 bytes)>

Packet Encoder (Source) read data from gr.msg_queue(), given as parameter.
Packet Decoder (Sink) send data to gr.msg_queue(), given as parameter.

Status
======
Beta version

Documentation
=============
- TODO
- See grc/*.png as flow graph examples
- See testing-scripts/* as complete examples (FSK modulation/demodulation between RFCat and HackRF One)



