#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: grc_cc1111_test_local_loop
# Author: Jerome Nokin
# Generated: Fri Jul 18 14:00:18 2014
##################################################

from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import cc1111
import math

class grc_cc1111_test_local_loop(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "grc_cc1111_test_local_loop")

        ##################################################
        # Variables
        ##################################################
        self.symbole_rate = symbole_rate = 80000
        self.samp_rate = samp_rate = 2e6
        self.samp_per_sym = samp_per_sym = int(samp_rate / symbole_rate)
        self.preamble = preamble = '0101010101010101'
        self.myqueue_out = myqueue_out = gr.msg_queue(2)
        self.myqueue_in = myqueue_in = gr.msg_queue(2)
        self.bit_per_sym = bit_per_sym = 1
        self.access_code = access_code = '11010011100100011101001110010001'

        ##################################################
        # Blocks
        ##################################################
        self.digital_gmsk_mod_0 = digital.gmsk_mod(
        	samples_per_symbol=int(samp_per_sym),
        	bt=0.5,
        	verbose=False,
        	log=False,
        )
        self.digital_correlate_access_code_bb_0_0 = digital.correlate_access_code_bb(access_code, 1)
        self.digital_clock_recovery_mm_xx_0_0 = digital.clock_recovery_mm_ff(samp_per_sym*(1+0.0), 0.25*0.175*0.175, 0.5, 0.175, 0.005)
        self.digital_binary_slicer_fb_0_0_0 = digital.binary_slicer_fb()
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
        self.cc1111_cc1111_packet_decoder_0 = cc1111.cc1111_packet_decoder(myqueue_out,True, True, False, True)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_char*1, samp_rate,True)
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_char*1)
        self.analog_quadrature_demod_cf_0_0 = analog.quadrature_demod_cf(1)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.digital_clock_recovery_mm_xx_0_0, 0), (self.digital_binary_slicer_fb_0_0_0, 0))
        self.connect((self.analog_quadrature_demod_cf_0_0, 0), (self.digital_clock_recovery_mm_xx_0_0, 0))
        self.connect((self.digital_binary_slicer_fb_0_0_0, 0), (self.digital_correlate_access_code_bb_0_0, 0))
        self.connect((self.digital_gmsk_mod_0, 0), (self.analog_quadrature_demod_cf_0_0, 0))
        self.connect((self.digital_correlate_access_code_bb_0_0, 0), (self.cc1111_cc1111_packet_decoder_0, 0))
        self.connect((self.cc1111_cc1111_packet_decoder_0, 0), (self.blocks_null_sink_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.digital_gmsk_mod_0, 0))
        self.connect((self.cc1111_cc1111_packet_encoder_0, 0), (self.blocks_throttle_0, 0))



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
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)

    def get_samp_per_sym(self):
        return self.samp_per_sym

    def set_samp_per_sym(self, samp_per_sym):
        self.samp_per_sym = samp_per_sym
        self.digital_clock_recovery_mm_xx_0_0.set_omega(self.samp_per_sym*(1+0.0))

    def get_preamble(self):
        return self.preamble

    def set_preamble(self, preamble):
        self.preamble = preamble

    def get_myqueue_out(self):
        return self.myqueue_out

    def set_myqueue_out(self, myqueue_out):
        self.myqueue_out = myqueue_out

    def get_myqueue_in(self):
        return self.myqueue_in

    def set_myqueue_in(self, myqueue_in):
        self.myqueue_in = myqueue_in

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
    tb = grc_cc1111_test_local_loop()
    tb.start()
    raw_input('Press Enter to quit: ')
    tb.stop()
    tb.wait()
