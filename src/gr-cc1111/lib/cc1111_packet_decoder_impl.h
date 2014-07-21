/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_CC1111_CC1111_PACKET_DECODER_IMPL_H
#define INCLUDED_CC1111_CC1111_PACKET_DECODER_IMPL_H

#include <cc1111/cc1111_packet_decoder.h>
//#include <time.h>
#include <cstdio>
	
#define BUF_MAX_SIZE		2048    // bytes
#define PN9_TABLE_SIZE		511

#define CRC16_POLY 		0x8005	// CRC16
#define CRC_INIT 		0xFFFF



namespace gr {
  namespace cc1111 {

    class cc1111_packet_decoder_impl : public cc1111_packet_decoder
    {
     private:
	msg_queue::sptr target_queue;	   	// block arg.  where to send the packet when received
	bool do_unwhitening;			// block arg
	bool do_crc16_check;			// block arg
	bool verbose;				// block arg
	bool drop_header;			// block arg

        bool is_msg;
        unsigned char buffer[BUF_MAX_SIZE];	// store message
	int buffer_expected_len; 		// message length according to message header, in bits
        int bit_index;				// bit index, within a byte. byte_index would have been better ?
        int buffer_i;				// index of buffer[]

        struct timeval time_init;
        struct timeval time_sync_found;

	// 99.9% inspired from https://github.com/matthijskooijman/arduino-max/blob/master/Pn9.cpp 
	unsigned char pn9_table[PN9_TABLE_SIZE]; 

     public:
      cc1111_packet_decoder_impl(msg_queue::sptr target_queue, bool do_unwhitening, bool do_crc16_check, bool verbose, bool drop_header);
      ~cc1111_packet_decoder_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);

      // manage buffer
      int buffer_flush(unsigned char* out);
      int buffer_append(unsigned char byte);
      int buffer_reset();

      // crc16
      uint16_t culCalcCRC(unsigned char crcData, uint16_t crcReg);

      // Un-Whitening
      int pn9_xor(unsigned char *buf, int len);
      int pn9_init_table();

    };

  } // namespace cc1111
} // namespace gr

#endif /* INCLUDED_CC1111_CC1111_PACKET_DECODER_IMPL_H */

