/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <string>
#include <qi/log.hpp>
#include <qi/transport/transport_server.hpp>
#include <qi/transport/transport_message_handler.hpp>
#include <qi/transport/src/server_backend.hpp>
#include <qi/transport/src/zmq/zmq_simple_server.hpp>

namespace qi {
  namespace transport {

    TransportServer::TransportServer()
      : _isInitialized(false)
    {
    }

    void TransportServer::serve(const std::string &address) {
      std::vector<std::string> v;
      v.push_back(address);
      serve(v);
    }

    void TransportServer::serve(const std::vector<std::string> &addresses)
    {
      //for(unsigned int i = 0 ; i< addresses.size(); ++i) {
      //  qisInfo << "* GenericTransportServer:serve " << addresses[i] << std::endl;
      //}
      try {
        _transportServer = new detail::ZMQSimpleServerImpl(addresses);
        _isInitialized = true;
      } catch(const std::exception& e) {
        qisError << "Failed to create transport server for addresses:";
        for(unsigned int i = 0 ; i< addresses.size(); ++i) {
          qisError << " " << addresses[i] << std::endl;
        }
        qisError << " Reason:" << e.what() << std::endl;
        throw(e);
      }
    }

    void TransportServer::run()
    {
      if (_isInitialized) {
        _transportServer->run();
      }
    }

    void TransportServer::setMessageHandler(TransportMessageHandler* dataHandler) {
      _transportServer->setDataHandler(dataHandler);
    }

    TransportMessageHandler* TransportServer::getMessageHandler() {
      return _transportServer->getDataHandler();
    }

    bool TransportServer::isInitialized() {
      return _isInitialized;
    }

  }

}
