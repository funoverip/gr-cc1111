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


#ifndef INCLUDED_CC1111_CC1111_PACKET_DECODER_H
#define INCLUDED_CC1111_CC1111_PACKET_DECODER_H

#include <cc1111/api.h>
#include <gnuradio/block.h>
#include <gnuradio/msg_queue.h>

namespace gr {
  namespace cc1111 {

    /*!
     * \brief <+description of block+>
     * \ingroup cc1111
     *
     */
    class CC1111_API cc1111_packet_decoder : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<cc1111_packet_decoder> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of cc1111::cc1111_packet_decoder.
       *
       * To avoid accidental use of raw pointers, cc1111::cc1111_packet_decoder's
       * constructor is in a private implementation
       * class. cc1111::cc1111_packet_decoder::make is the public interface for
       * creating new instances.
       */
      static sptr make(msg_queue::sptr target_queue, bool, bool, bool, bool);
    };

  } // namespace cc1111
} // namespace gr

#endif /* INCLUDED_CC1111_CC1111_PACKET_DECODER_H */

