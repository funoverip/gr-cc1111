#!/usr/bin/python

import signal
import sys
from time import sleep
from gnuradio import gr
import grc_cc1111_hackrf_sender
from binascii import hexlify, unhexlify

# rf object
rf = ''

def signal_handler(signal, frame):
	global rf
        print('You pressed Ctrl+C!')
	rf.stop()
        sys.exit(0)


if __name__ == '__main__':

	signal.signal(signal.SIGINT, signal_handler)

	i = 0

	try:

                if len(sys.argv) != 2:
                        print "Usage   : %s <freq in Hz>" % sys.argv[0]
                        print "Example : %s 433610000" % sys.argv[0]
                        sys.exit(0)


		rf = grc_cc1111_hackrf_sender.grc_cc1111_hackrf_sender()
		rf.set_frequency(long(sys.argv[1]))
		rf.start()

		while True:
			# Send a string
			tosend = "Hello from HackRF One (%d)" % i
			i+=1
			msg = gr.message_from_string(tosend)
			print "[hackrf send] : '%s'" % (tosend)
	        	rf.myqueue_in.insert_tail(msg)

			sleep(1)

	except KeyboardInterrupt:
		print("W: interrupt received, proceeding")
