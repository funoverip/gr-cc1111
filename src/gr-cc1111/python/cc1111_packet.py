#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2014 funoverip.net.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 


# This code is mainly a copy/paste of blks2/packet.py
# header, whitening and CRC were adapted to CC11xx packet format

from binascii import hexlify
from gnuradio import gr, digital
from gnuradio import blocks
from gnuradio.digital import packet_utils
import cc1111_packet_utils
import gnuradio.gr.gr_threading as _threading


##how many messages in a queue
DEFAULT_MSGQ_LIMIT = 2


##################################################
## CC1111 Packet Encoder
##################################################
class _cc1111_packet_encoder_thread(_threading.Thread):

	def __init__(self, msgq, send):
		self._msgq = msgq
		self._send = send
		_threading.Thread.__init__(self)
		self.setDaemon(1)
		self.keep_running = True
		self.start()

	def run(self):
		while self.keep_running:
			msg = self._msgq.delete_head() #blocking read of message queue
			sample = msg.to_string() #get the body of the msg as a string
			self._send(sample)
		


class cc1111_packet_encoder(gr.hier_block2):
	"""
	Hierarchical block for wrapping packet-based modulators.
	"""
	def __init__(self, samples_per_symbol, bits_per_symbol, preamble='', access_code='', pad_for_usrp=True, do_whitening=False, add_crc=False):
		"""
		packet_mod constructor.
        
		Args:
			samples_per_symbol: number of samples per symbol
			bits_per_symbol: number of bits per symbol
			access_code: AKA sync vector
			pad_for_usrp: If true, packets are padded such that they end up a multiple of 128 samples
			do_whitening: apply CC111x whitening
			add_crc: add CRC16 
		"""

		#setup parameters
		self._samples_per_symbol = samples_per_symbol
 		self._bits_per_symbol = bits_per_symbol
		self._pad_for_usrp = pad_for_usrp
		if not preamble: #get preamble
			preamble = cc1111_packet_utils.default_preamble
		if not access_code: #get access code
			access_code = cc1111_packet_utils.default_access_code
		if not packet_utils.is_1_0_string(preamble):
			raise ValueError, "Invalid preamble %r. Must be string of 1's and 0's" % (preamble,)
		if not packet_utils.is_1_0_string(access_code):
			raise ValueError, "Invalid access_code %r. Must be string of 1's and 0's" % (access_code,)
		self._preamble = preamble
		self._access_code = access_code
		self._pad_for_usrp = pad_for_usrp
		self._do_whitening = do_whitening
		self._add_crc = add_crc

		#create blocks
		msg_source = blocks.message_source(gr.sizeof_char, DEFAULT_MSGQ_LIMIT)
		self._msgq_out = msg_source.msgq()
		#initialize hier2
		gr.hier_block2.__init__(
			self,
			"cc1111_packet_encoder",
			gr.io_signature(0, 0, 0), # Input signature
			gr.io_signature(1, 1, gr.sizeof_char) # Output signature
		)
		#connect
		self.connect(msg_source, self)

	def send_pkt(self, payload):
		"""
		Wrap the payload in a packet and push onto the message queue.

		Args:
			payload: string, data to send
		"""
		packet = cc1111_packet_utils.make_packet(
			payload,
			self._samples_per_symbol,
			self._bits_per_symbol,
			self._preamble,
			self._access_code,
			self._pad_for_usrp,
			self._do_whitening,
			self._add_crc
		)
		msg = gr.message_from_string(packet)
		self._msgq_out.insert_tail(msg)


class cc1111_packet_mod_base(gr.hier_block2):
	"""
	Hierarchical block for wrapping packet source block.
	"""

	def __init__(self, packet_source, source_queue):
		#initialize hier2
		gr.hier_block2.__init__(
			self,
			"cc1111_packet_mod_base",
			gr.io_signature(0, 0, 0), # Input signature
			gr.io_signature(1, 1, packet_source._hb.output_signature().sizeof_stream_item(0)) # Output signature
		)
		self.connect(packet_source, self)
		#start thread
		_cc1111_packet_encoder_thread(source_queue, packet_source.send_pkt)

