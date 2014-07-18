#!/usr/bin/python

import signal
import sys
from time import sleep
from gnuradio import gr
import grc_cc1111_test_local_loop
#import cc1111
from binascii import hexlify, unhexlify

# rf object
rf = ''


if __name__ == '__main__':

	try:
		rf = grc_cc1111_test_local_loop.grc_cc1111_test_local_loop()
		rf.start()

		i = 0

		while True:
			# Send a string
			tosend = "Hello from local_loop (%d)" % i
			msg = gr.message_from_string(tosend)
			print "Pushing message"
			print "   Send: %s" % (tosend)
		        rf.myqueue_in.insert_tail(msg)

			# read it back 
			print "Polling message"
			received = rf.myqueue_out.delete_head().to_string()
			print "   Recv: %s" % (received)

			i +=1
			sleep(2)

	except KeyboardInterrupt:
		print("W: interrupt received, proceeding")
		rf.stop()
