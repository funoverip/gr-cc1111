#!/usr/bin/env python
 
import sys
import time
from time import sleep
from rflib import *
from binascii import hexlify, unhexlify

#frequency = 869.0e6  


def ConfigureD(d, frequency):
	d.setModeIDLE()
	d.setMdmModulation(MOD_2FSK)
	print "Freq: %d" % (frequency)
	d.setFreq(frequency)
        d.setMdmDeviatn(19000)
        d.setMdmDRate(40000)
	d.setMdmSyncWord(0xd391)
	d.setMdmSyncMode(SYNC_MODE_30_32)
	d.setEnablePktCRC()
	d.setEnablePktDataWhitening()
	d.makePktVLEN()
	#d.setMaxPower()


if __name__ == '__main__':

	try:

		if len(sys.argv) != 2:
			print "Usage   : %s <freq in Hz>" % sys.argv[0]
			print "Example : %s 433058000" % sys.argv[0]
			sys.exit(0)

		freq = long(sys.argv[1])
		i = 0
		d = RfCat()
		ConfigureD(d, freq)
	
		print "[rfcat] Entering TX Mode"
		while True:
			msg = "Hello from RFCat ... (%d)" % i
			#d.RFxmit(chr(len(msg)) + msg)
			print "[rfcat send] : '%s'" % msg
			d.RFxmit(msg)
			i += 1
			sleep(0.5)
		print "Done"

        except KeyboardInterrupt:
                print("W: interrupt received, proceeding")
	        d.setModeIDLE()
	        sys.exit(0)




