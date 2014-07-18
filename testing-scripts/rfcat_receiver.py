#!/usr/bin/env python
 
import sys
import time
from rflib import *
from binascii import hexlify, unhexlify



def ConfigureD(d, freq):
	d.setModeIDLE()
	d.setMdmModulation(MOD_2FSK)
	d.setFreq(freq)
	d.setMdmDeviatn(19000)
	d.setMdmDRate(40000)
	d.setMdmSyncWord(0xd391)
	d.setMdmSyncMode(SYNC_MODE_30_32)
	d.setEnablePktCRC()
	d.setEnablePktDataWhitening()
	d.makePktVLEN()
	#d.makePktFLEN(0x32)


if __name__ == '__main__':


	try:

                if len(sys.argv) != 2:
                        print "Usage   : %s <freq in Hz>" % sys.argv[0]
                        print "Example : %s 433600000" % sys.argv[0]
                        sys.exit(0)


		d = RfCat()
		ConfigureD(d, long(sys.argv[1]))
		d.setModeRX() 
		 
		print "[rfcat] Entering RX Mode"
		while True:
			# readng packet
			pkt=d.RFrecv(timeout=120000)
			frame = pkt[0]
			print "[rfcat recv] : '%s'" % (frame)

        except KeyboardInterrupt:
                print("W: interrupt received, proceeding")
	        d.setModeIDLE()
	        sys.exit(0)




