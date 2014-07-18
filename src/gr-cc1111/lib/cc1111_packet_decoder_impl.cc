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

	printf("cc1111_packet_decoder_impl::pn9_init_table(): TODOOOOOOOOOOO\n");

	pn9_table[0] = 0xff;
	pn9_table[1] = 0xe1;
	pn9_table[2] = 0x1d;
	pn9_table[3] = 0x9a;
	pn9_table[4] = 0xed;
	pn9_table[5] = 0x85;
	pn9_table[6] = 0x33;
	pn9_table[7] = 0x24;
	pn9_table[8] = 0xea;
	pn9_table[9] = 0x7a;
	pn9_table[10] = 0xd2;
	pn9_table[11] = 0x39;
	pn9_table[12] = 0x70;
	pn9_table[13] = 0x97;
	pn9_table[14] = 0x57;
	pn9_table[15] = 0x0a;
	pn9_table[16] = 0x54;
	pn9_table[17] = 0x7d;
	pn9_table[18] = 0x2d;
	pn9_table[19] = 0xd8;
	pn9_table[20] = 0x6d;
	pn9_table[21] = 0x0d;
	pn9_table[22] = 0xba;
	pn9_table[23] = 0x8f;
	pn9_table[24] = 0x67;
	pn9_table[25] = 0x59;
	pn9_table[26] = 0xc7;
	pn9_table[27] = 0xa2;
	pn9_table[28] = 0xbf;
	pn9_table[29] = 0x34;
	pn9_table[30] = 0xca;
	pn9_table[31] = 0x18;
	pn9_table[32] = 0x30;
	pn9_table[33] = 0x53;
	pn9_table[34] = 0x93;
	pn9_table[35] = 0xdf;
	pn9_table[36] = 0x92;
	pn9_table[37] = 0xec;
	pn9_table[38] = 0xa7;
	pn9_table[39] = 0x15;
	pn9_table[40] = 0x8a;
	pn9_table[41] = 0xdc;
	pn9_table[42] = 0xf4;
	pn9_table[43] = 0x86;
	pn9_table[44] = 0x55;
	pn9_table[45] = 0x4e;
	pn9_table[46] = 0x18;
	pn9_table[47] = 0x21;
	pn9_table[48] = 0x40;
	pn9_table[49] = 0xc4;
	pn9_table[50] = 0xc4;
	pn9_table[51] = 0xd5;
	pn9_table[52] = 0xc6;
	pn9_table[53] = 0x91;
	pn9_table[54] = 0x8a;
	pn9_table[55] = 0xcd;
	pn9_table[56] = 0xe7;
	pn9_table[57] = 0xd1;
	pn9_table[58] = 0x4e;
	pn9_table[59] = 0x09;
	pn9_table[60] = 0x32;
	pn9_table[61] = 0x17;
	pn9_table[62] = 0xdf;
	pn9_table[63] = 0x83;


    }




  } /* namespace cc1111 */
} /* namespace gr */

