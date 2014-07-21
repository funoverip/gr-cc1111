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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "cc1111_packet_decoder_impl.h"

namespace gr {
  namespace cc1111 {

    cc1111_packet_decoder::sptr
    cc1111_packet_decoder::make(msg_queue::sptr target_queue, bool do_unwhitening, bool do_crc16_check, bool verbose, bool drop_header)
    {
      return gnuradio::get_initial_sptr
        (new cc1111_packet_decoder_impl(target_queue, do_unwhitening, do_crc16_check, verbose, drop_header));
    }

    /*
     * The private constructor
     */
    cc1111_packet_decoder_impl::cc1111_packet_decoder_impl(msg_queue::sptr arg_target_queue, bool arg_do_unwhitening, bool arg_do_crc16_check, bool arg_verbose, bool arg_drop_header)
      : gr::block("cc1111_packet_decoder",
              gr::io_signature::make(1, 1, sizeof(unsigned char)),
              gr::io_signature::make(1, 1, sizeof(unsigned char))),

              target_queue(arg_target_queue),
              do_unwhitening(arg_do_unwhitening),
              do_crc16_check(arg_do_crc16_check),
              verbose(arg_verbose),
	      drop_header(arg_drop_header),

              is_msg(false),
              buffer_expected_len(0),
              bit_index(0),
              buffer_i(0)
    {
        // reset buffer
        buffer_reset();
        // init pn9 table
        pn9_init_table();
    }

    /*
     * Our virtual destructor.
     */
    cc1111_packet_decoder_impl::~cc1111_packet_decoder_impl()
    {
    }

    void
    cc1111_packet_decoder_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
	ninput_items_required[0] = noutput_items;
    }

    int
    cc1111_packet_decoder_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {

        const unsigned char *in = (const unsigned char *) input_items[0];
        unsigned char *out = (unsigned char *) output_items[0];
        unsigned int produced=0;

        // Do <+signal processing+>
        for(int i=0;i<noutput_items;i++){

                // we are currently processing a message
                if(is_msg){
                        // add new bit
                        buffer_append(in[i]);
                        // end of message ? 
                        if ((buffer_i != 0 && bit_index == 8 && buffer_i == buffer_expected_len - 1) ||
                                buffer_i == (BUF_MAX_SIZE)-1){
                                is_msg = false; // stop
                                buffer_flush(out);
                                produced = buffer_i+1;
                        }
                }else{
                        // did we find the beginning of a new message ?
                        if(in[i] & 0x02){
                                // reset buffer 
                                buffer_reset();
                                // add the first bit
                                buffer_append(in[i]);
                                // swith processing mode
                                is_msg = true;
                        }
                }
        }

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return produced;
    }



    /****************************************************************
    BUFFER FLUSH
    ****************************************************************/
    // print/send out current buffer
    int cc1111_packet_decoder_impl::buffer_flush(unsigned char* out){

        //char timestamp_buf[64];
	uint16_t checksum = CRC_INIT;
	uint16_t checksum_found;

        // un-whitenig buffer ?
	if (do_unwhitening){
	        if(pn9_xor(buffer, buffer_i+1) == -1){
			//fprintf(stderr, "xor_pn9() returned -1\n");
		}
	}

        if(verbose){
		fprintf(stdout, "[verbose] Pkt: ");
                for(int j=0;j<buffer_i+1;j++){
                        fprintf(stdout, "%02x ", (unsigned char)buffer[j]);
                }
        }

	// CHECKSUM ?
	if(do_crc16_check){
		
                // get CRC from frame (2 last bytes)
                checksum_found = buffer[buffer_i-1];
                checksum_found <<= 8;
                checksum_found += buffer[buffer_i];
		// compute real CRC
                for(int i = 0; i < (buffer_i+1) - 2 ; i++)
                        checksum = culCalcCRC((unsigned char)buffer[i], checksum);
		// drop frame if wrong CRC
		if(checksum != checksum_found){
			if(verbose){
				fprintf(stdout, "[crc error]\n");
			}

			buffer_i = -1; // on return, we do 'produced = buffer_i+1;'
			return 0;
		}else{
			// if CRC is correct, remove it from frame
			buffer_i -= 2;
		}
	}

	if(verbose){
		fprintf(stdout, "\n");
	}


	// DROP HEADER ?
	if(drop_header){
	        // copy buffer to out port
	        memcpy(out, buffer+1, buffer_i);
	        // .. and send a message to the queue (remove header (1byte))
	        message::sptr msg = message::make(0, 0, 0, buffer_i);
	        memcpy(msg->msg(), buffer+1, buffer_i);   // copy
	        target_queue->insert_tail(msg);       // send it
	        msg.reset();                          // free it up
  	}else{
	        // copy buffer to out port
	        memcpy(out, buffer, buffer_i+1);
	        // .. and send a message to the queue
	        message::sptr msg = message::make(0, 0, 0, buffer_i+1);
	        memcpy(msg->msg(), buffer, buffer_i+1);   // copy
	        target_queue->insert_tail(msg);       // send it
	        msg.reset();                          // free it up
	}
	


	return 1;
    }


    /****************************************************************
    BUFFER APPEND
    ****************************************************************/
    // we receive a byte and add the first bit to the end of the buffer
    int cc1111_packet_decoder_impl::buffer_append(unsigned char byte){

        // need a new byte in buffer[] ?
        if(bit_index == 8){
                bit_index=0;
                buffer_i++;
        }

	// is the first byte complete ? If yes, it contains the length of the message
	// header len and CRC must be added
	if(buffer_i == 1 && bit_index == 0){
		// payload len
		if(do_unwhitening){
			buffer_expected_len =     (int)(buffer[0] ^ 0xff); // un-whitening
		}else{
			buffer_expected_len =     (int)(buffer[0]);
		}
		// add crc len ?
		if(do_crc16_check){
			buffer_expected_len +=  2; // CRC16 is of course 2 bytes length
		}
		// add header len
		buffer_expected_len += 1;  // header is 1 byte (len)
	}

        // add new bit to the end of the buffer
        if(bit_index==0){
                buffer[buffer_i] = (byte & 0x1);
        }else{
                buffer[buffer_i] = (buffer[buffer_i] << 1) | (byte & 0x1);
        }
        // inc bit index
        bit_index++;

        return 1;
    }

    /****************************************************************
    BUFFER RESET
    ****************************************************************/
    int cc1111_packet_decoder_impl::buffer_reset(){
	memset(buffer,0,BUF_MAX_SIZE);
        bit_index=0;
        buffer_i=0;
        return 1;
    }


   /****************************************************************
   CRC16
   ****************************************************************/
    uint16_t cc1111_packet_decoder_impl::culCalcCRC(unsigned char crcData, uint16_t crcReg) {
        unsigned int i;
        for (i = 0; i < 8; i++) {
                if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
                        crcReg = (crcReg << 1) ^ CRC16_POLY;
                else
                        crcReg = (crcReg << 1);
                crcData <<= 1;
        }
        return crcReg;
    }


    /****************************************************************
    WHITENING
    ****************************************************************/
    int cc1111_packet_decoder_impl::pn9_xor(unsigned char *buf, int len) {
        int max_len = (len > sizeof(pn9_table)) ? sizeof(pn9_table) -1 : len;
	for (int i = 0; i < max_len; ++i)
    		buf[i] ^= pn9_table[i];
        if (len > sizeof(pn9_table))
                return -1;
	else
	  	return 0;
    }

    int cc1111_packet_decoder_impl::pn9_init_table(){

	// 511 Bytes XOR’d with Data during a Whitening Operation 
	unsigned char tmp[] = {
		0xff, 0xe1, 0x1d, 0x9a, 0xed, 0x85, 0x33, 0x24, 0xea, 0x7a, 0xd2, 0x39, 0x70, 0x97, 0x57, 0x0a,
		0x54, 0x7d, 0x2d, 0xd8, 0x6d, 0x0d, 0xba, 0x8f, 0x67, 0x59, 0xc7, 0xa2, 0xbf, 0x34, 0xca, 0x18,
		0x30, 0x53, 0x93, 0xdf, 0x92, 0xec, 0xa7, 0x15, 0x8a, 0xdc, 0xf4, 0x86, 0x55, 0x4e, 0x18, 0x21,
		0x40, 0xc4, 0xc4, 0xd5, 0xc6, 0x91, 0x8a, 0xcd, 0xe7, 0xd1, 0x4e, 0x09, 0x32, 0x17, 0xdf, 0x83,
		0xff, 0xf0, 0x0e, 0xcd, 0xf6, 0xc2, 0x19, 0x12, 0x75, 0x3d, 0xe9, 0x1c, 0xb8, 0xcb, 0x2b, 0x05,
		0xaa, 0xbe, 0x16, 0xec, 0xb6, 0x06, 0xdd, 0xc7, 0xb3, 0xac, 0x63, 0xd1, 0x5f, 0x1a, 0x65, 0x0c,
		0x98, 0xa9, 0xc9, 0x6f, 0x49, 0xf6, 0xd3, 0x0a, 0x45, 0x6e, 0x7a, 0xc3, 0x2a, 0x27, 0x8c, 0x10,
		0x20, 0x62, 0xe2, 0x6a, 0xe3, 0x48, 0xc5, 0xe6, 0xf3, 0x68, 0xa7, 0x04, 0x99, 0x8b, 0xef, 0xc1,
		0x7f, 0x78, 0x87, 0x66, 0x7b, 0xe1, 0x0c, 0x89, 0xba, 0x9e, 0x74, 0x0e, 0xdc, 0xe5, 0x95, 0x02,
		0x55, 0x5f, 0x0b, 0x76, 0x5b, 0x83, 0xee, 0xe3, 0x59, 0xd6, 0xb1, 0xe8, 0x2f, 0x8d, 0x32, 0x06,
		0xcc, 0xd4, 0xe4, 0xb7, 0x24, 0xfb, 0x69, 0x85, 0x22, 0x37, 0xbd, 0x61, 0x95, 0x13, 0x46, 0x08,
		0x10, 0x31, 0x71, 0xb5, 0x71, 0xa4, 0x62, 0xf3, 0x79, 0xb4, 0x53, 0x82, 0xcc, 0xc5, 0xf7, 0xe0,
		0x3f, 0xbc, 0x43, 0xb3, 0xbd, 0x70, 0x86, 0x44, 0x5d, 0x4f, 0x3a, 0x07, 0xee, 0xf2, 0x4a, 0x81,
		0xaa, 0xaf, 0x05, 0xbb, 0xad, 0x41, 0xf7, 0xf1, 0x2c, 0xeb, 0x58, 0xf4, 0x97, 0x46, 0x19, 0x03,
		0x66, 0x6a, 0xf2, 0x5b, 0x92, 0xfd, 0xb4, 0x42, 0x91, 0x9b, 0xde, 0xb0, 0xca, 0x09, 0x23, 0x04,
		0x88, 0x98, 0xb8, 0xda, 0x38, 0x52, 0xb1, 0xf9, 0x3c, 0xda, 0x29, 0x41, 0xe6, 0xe2, 0x7b, 0xf0,
		0x1f, 0xde, 0xa1, 0xd9, 0x5e, 0x38, 0x43, 0xa2, 0xae, 0x27, 0x9d, 0x03, 0x77, 0x79, 0xa5, 0x40,
		0xd5, 0xd7, 0x82, 0xdd, 0xd6, 0xa0, 0xfb, 0x78, 0x96, 0x75, 0x2c, 0xfa, 0x4b, 0xa3, 0x8c, 0x01,
		0x33, 0x35, 0xf9, 0x2d, 0xc9, 0x7e, 0x5a, 0xa1, 0xc8, 0x4d, 0x6f, 0x58, 0xe5, 0x84, 0x11, 0x02,
		0x44, 0x4c, 0x5c, 0x6d, 0x1c, 0xa9, 0xd8, 0x7c, 0x1e, 0xed, 0x94, 0x20, 0x73, 0xf1, 0x3d, 0xf8,
		0x0f, 0xef, 0xd0, 0x6c, 0x2f, 0x9c, 0x21, 0x51, 0xd7, 0x93, 0xce, 0x81, 0xbb, 0xbc, 0x52, 0xa0,
		0xea, 0x6b, 0xc1, 0x6e, 0x6b, 0xd0, 0x7d, 0x3c, 0xcb, 0x3a, 0x16, 0xfd, 0xa5, 0x51, 0xc6, 0x80,
		0x99, 0x9a, 0xfc, 0x96, 0x64, 0x3f, 0xad, 0x50, 0xe4, 0xa6, 0x37, 0xac, 0x72, 0xc2, 0x08, 0x01,
		0x22, 0x26, 0xae, 0x36, 0x8e, 0x54, 0x6c, 0x3e, 0x8f, 0x76, 0x4a, 0x90, 0xb9, 0xf8, 0x1e, 0xfc,
		0x87, 0x77, 0x68, 0xb6, 0x17, 0xce, 0x90, 0xa8, 0xeb, 0x49, 0xe7, 0xc0, 0x5d, 0x5e, 0x29, 0x50,
		0xf5, 0xb5, 0x60, 0xb7, 0x35, 0xe8, 0x3e, 0x9e, 0x65, 0x1d, 0x8b, 0xfe, 0xd2, 0x28, 0x63, 0xc0,
		0x4c, 0x4d, 0x7e, 0x4b, 0xb2, 0x9f, 0x56, 0x28, 0x72, 0xd3, 0x1b, 0x56, 0x39, 0x61, 0x84, 0x00,
		0x11, 0x13, 0x57, 0x1b, 0x47, 0x2a, 0x36, 0x9f, 0x47, 0x3b, 0x25, 0xc8, 0x5c, 0x7c, 0x0f, 0xfe,
		0xc3, 0x3b, 0x34, 0xdb, 0x0b, 0x67, 0x48, 0xd4, 0xf5, 0xa4, 0x73, 0xe0, 0x2e, 0xaf, 0x14, 0xa8,
		0xfa, 0x5a, 0xb0, 0xdb, 0x1a, 0x74, 0x1f, 0xcf, 0xb2, 0x8e, 0x45, 0x7f, 0x69, 0x94, 0x31, 0x60,
		0xa6, 0x26, 0xbf, 0x25, 0xd9, 0x4f, 0x2b, 0x14, 0xb9, 0xe9, 0x0d, 0xab, 0x9c, 0x30, 0x42, 0x80,
		0x88, 0x89, 0xab, 0x8d, 0x23, 0x15, 0x9b, 0xcf, 0xa3, 0x9d, 0x12, 0x64, 0x2e, 0xbe, 0x07};

	memcpy((unsigned char*)pn9_table, (unsigned char*)tmp, PN9_TABLE_SIZE );

    }




  } /* namespace cc1111 */
} /* namespace gr */

