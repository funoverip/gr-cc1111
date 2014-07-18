#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: grc_cc1111_hackrf_sender
# Author: Jerome Nokin
# Generated: Fri Jul 18 13:51:40 2014
##################################################

from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import cc1111
import osmosdr

class grc_cc1111_hackrf_sender(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "grc_cc1111_hackrf_sender")

        ##################################################
        # Variables
        ##################################################
        self.symbole_rate = symbole_rate = 40000
        self.samp_rate = samp_rate = 2e6
        self.samp_per_sym = samp_per_sym = int(samp_rate / symbole_rate)
        self.preamble = preamble = '0101010101010101'
        self.myqueue_in = myqueue_in = gr.msg_queue(2)
        self.frequency = frequency = 433.6e6
        self.bit_per_sym = bit_per_sym = 1
        self.access_code = access_code = '11010011100100011101001110010001'

        ##################################################
        # Blocks
        ##################################################
        self.osmosdr_sink_0 = osmosdr.sink( args="numchan=" + str(1) + " " + "rfcat" )
        self.osmosdr_sink_0.set_sample_rate(samp_rate)
        self.osmosdr_sink_0.set_center_freq(frequency, 0)
        self.osmosdr_sink_0.set_freq_corr(0, 0)
        self.osmosdr_sink_0.set_gain(10, 0)
        self.osmosdr_sink_0.set_if_gain(20, 0)
        self.osmosdr_sink_0.set_bb_gain(20, 0)
        self.osmosdr_sink_0.set_antenna("", 0)
        self.osmosdr_sink_0.set_bandwidth(0, 0)
          
        self.digital_gmsk_mod_0 = digital.gmsk_mod(
        	samples_per_symbol=int(samp_per_sym),
        	bt=0.5,
        	verbose=False,
        	log=False,
        )
        self.cc1111_cc1111_packet_encoder_0 = cc1111.cc1111_packet_mod_base(cc1111.cc1111_packet_encoder(
                        samples_per_symbol=samp_per_sym,
                        bits_per_symbol=bit_per_sym,
                        preamble=preamble,
                        access_code=access_code,
                        pad_for_usrp=True,
        		do_whitening=True,
        		add_crc=True
                ),
        	source_queue=myqueue_in
        	)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.digital_gmsk_mod_0, 0), (self.osmosdr_sink_0, 0))
        self.connect((self.cc1111_cc1111_packet_encoder_0, 0), (self.digital_gmsk_mod_0, 0))



    def get_symbole_rate(self):
        return self.symbole_rate

    def set_symbole_rate(self, symbole_rate):
        self.symbole_rate = symbole_rate
        self.set_samp_per_sym(int(self.samp_rate / self.symbole_rate))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_samp_per_sym(int(self.samp_rate / self.symbole_rate))
        self.osmosdr_sink_0.set_sample_rate(self.samp_rate)

    def get_samp_per_sym(self):
        return self.samp_per_sym

    def set_samp_per_sym(self, samp_per_sym):
        self.samp_per_sym = samp_per_sym

    def get_preamble(self):
        return self.preamble

    def set_preamble(self, preamble):
        self.preamble = preamble

    def get_myqueue_in(self):
        return self.myqueue_in

    def set_myqueue_in(self, myqueue_in):
        self.myqueue_in = myqueue_in

    def get_frequency(self):
        return self.frequency

    def set_frequency(self, frequency):
        self.frequency = frequency
        self.osmosdr_sink_0.set_center_freq(self.frequency, 0)

    def get_bit_per_sym(self):
        return self.bit_per_sym

    def set_bit_per_sym(self, bit_per_sym):
        self.bit_per_sym = bit_per_sym

    def get_access_code(self):
        return self.access_code

    def set_access_code(self, access_code):
        self.access_code = access_code

if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    tb = grc_cc1111_hackrf_sender()
    tb.start()
    raw_input('Press Enter to quit: ')
    tb.stop()
    tb.wait()
