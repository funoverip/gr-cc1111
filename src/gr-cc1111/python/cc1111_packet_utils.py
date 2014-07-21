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



# This code is mainly a copy/paste of digital/packet_utils.py
# header, whitening and CRC were adapted to CC11xx packet format

import struct
from gnuradio import gru
from gnuradio.digital import packet_utils

CRC16_POLY =  0x8005
CRC16_INIT =  0xFFFF
PN9_MAX_ENTRIES = 1024

default_access_code = packet_utils.conv_packed_binary_string_to_1_0_string('\xAC\xDD\xA4\xE2\xF2\x8C\x20\xFC')
default_preamble    = packet_utils.conv_packed_binary_string_to_1_0_string('\xA4\xF2')
pn9_table	    = []



def whiten(data):
	global pn9_table
	data_new = ''
	length = len(data)
	if length > PN9_MAX_ENTRIES:
		print "WARNING: WHITENING table not long enough!. Data will be corrupted"
	for i in range(length):
		data_new += chr(ord(data[i]) ^ pn9_table[i]) 
	return data_new


def dewhiten(data):
	return whiten(data)        # self inverse

def crc16(data):
	checksum = CRC16_INIT
	for c in data:
		checksum = crc16_2(ord(c), checksum); 
	return struct.pack('!H',checksum)

def crc16_2(crcData,crcReg):
	for bit in range(0, 8):
		if(((crcReg & 0x8000) >> 8) ^ (crcData & 0x80)):
			crcReg = ((crcReg << 1) ^ CRC16_POLY ) & 0x0ffff ;
		else:
			crcReg = (crcReg << 1) & 0x0ffff;
		crcData = (crcData << 1) & 0x0ff;
	return crcReg


# Gen Whitening table
def pn9_gen(number):
        state = 0x1ff
        pn9_table = [0xff]
        for count in range(number):
                for i in range(8):
                        state = (state >> 1) + (((state & 1) ^ (state >> 5) & 1) << 8)
                pn9_table.append(state & 0xff)
        return pn9_table
pn9_table = pn9_gen(0x1ff)


def make_header(payload_len):
	return struct.pack('b', payload_len & 0x0ff)

def make_packet(payload, samples_per_symbol, bits_per_symbol,
		preamble, access_code,
                pad_for_usrp=True, do_whitening=False, add_crc=False):
	"""
	Build a packet

	Args:
		payload: packet payload, len [0, 4096]
		samples_per_symbol: samples per symbol (needed for padding calculation) (int)
		bits_per_symbol: (needed for padding calculation) (int)
		preamble: (eg: 10101010...)
		access_code: the sync word
		pad_for_usrp:
		do_whitening: ccxxxx whitening version
		add_crc: add CRC16 (2 bytes) checksum
		
	Packet will have access code at the beginning, followed by length (1 byte), payload
	and finally CRC-16.
	"""

	if not packet_utils.is_1_0_string(preamble):
		raise ValueError, "preamble must be a string containing only 0's and 1's (%r)" % (preamble,)

	if not packet_utils.is_1_0_string(access_code):
		raise ValueError, "access_code must be a string containing only 0's and 1's (%r)" % (access_code,)

	(packed_access_code, padded) = packet_utils.conv_1_0_string_to_packed_binary_string(access_code)
	(packed_preamble, ignore) = packet_utils.conv_1_0_string_to_packed_binary_string(preamble)

	# len
	payload_length = len(payload)
	# header (1 byte)
	pkt_hd = make_header(payload_length)
	# data
	pkt_dt = payload
	# final message
	final_message = ''.join((pkt_hd, pkt_dt))

	# CRC ?
	if add_crc:
		crc = crc16(final_message)
		#print "crc: %02x %02x" % (ord(crc[0:1]), ord(crc[1:2]))
		final_message = ''.join((final_message, crc))

	# Whitening ?
	pkt = ''
	if do_whitening:
		pkt = ''.join((packed_preamble, packed_access_code,  whiten(final_message), '\x55'))
	else:
        	pkt = ''.join((packed_preamble, packed_access_code, final_message, '\x55'))

	# Padding ?
	if pad_for_usrp:
        	usrp_packing = packet_utils._npadding_bytes(len(pkt), samples_per_symbol, bits_per_symbol) * '\x55'
        	pkt = pkt + usrp_packing
	
	return pkt

