#!/usr/bin/python

import signal
import sys
from gnuradio import gr
import grc_cc1111_hackrf_receiver

# rf object
rf = ''

def signal_handler(signal, frame):
	global rf
        print('You pressed Ctrl+C!')
	rf.stop()
        sys.exit(0)


if __name__ == '__main__':

	signal.signal(signal.SIGINT, signal_handler)

	try:

                if len(sys.argv) != 2:
                        print "Usage   : %s <freq in Hz>" % sys.argv[0]
                        print "Example : %s 433070000" % sys.argv[0]
                        sys.exit(0)

		freq = long(sys.argv[1])
		rf = grc_cc1111_hackrf_receiver.grc_cc1111_hackrf_receiver()
		rf.set_frequency_center(freq)
		rf.start()

		while True:
			# Read a string
			msg = rf.myqueue_out.delete_head().to_string()
			print "[hackrf rcv] : '%s'" % msg

	except KeyboardInterrupt:
		print("W: interrupt received, proceeding")
