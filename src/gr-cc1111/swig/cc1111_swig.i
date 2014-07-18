/* -*- c++ -*- */

#define CC1111_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "cc1111_swig_doc.i"

%{
#include "cc1111/cc1111_packet_decoder.h"
%}
%include "cc1111/cc1111_packet_decoder.h"
GR_SWIG_BLOCK_MAGIC2(cc1111, cc1111_packet_decoder);
