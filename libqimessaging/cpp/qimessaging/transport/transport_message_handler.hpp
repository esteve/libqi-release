#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_MESSAGE_HANDLER_HPP_
#define _QI_TRANSPORT_TRANSPORT_MESSAGE_HANDLER_HPP_

# include <qimessaging/transport/buffer.hpp>

namespace qi {
  namespace transport {

    /// <summary> Server Interface. You need to create a class that inherit TransportMessageHandler
    /// and implement messageHandler. Each time a server receive a request, it will call messageHandler
    /// with a request, you should then complete the reply.</summary>
    /// \ingroup Transport
    class TransportMessageHandler {
    public:
      /// <summary> Generic message handler. </summary>
      /// <param name="request"> The request </param>
      /// <param name="reply"> The reply </param>
      virtual void messageHandler(qi::transport::Buffer& request,
                                  qi::transport::Buffer& reply) = 0;
    };
  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_MESSAGE_HANDLER_HPP_
