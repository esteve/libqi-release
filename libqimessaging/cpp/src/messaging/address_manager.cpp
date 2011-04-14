/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/messaging/address_manager.hpp"
#include "src/messaging/network/constants.hpp"
#include <cstdio>

namespace qi {
  namespace detail {
    AddressManager::AddressManager() :
      _masterPort(kDefaultMasterPort),
      _nextFreePort(kDefaultMasterPort)
    {}

    AddressManager::~AddressManager() {}

    int AddressManager::getMasterPort() const {
      return _masterPort;
    }

    void AddressManager::setMasterPort(int masterPort) {
      // This should only be called by the master
      // at startup before any ports are given
      _masterPort = masterPort;
      _nextFreePort = masterPort;
    }

    int AddressManager::getNewPort(const std::string& machineID) {
      // FIXME: machineID currently ignored
      // should really have per machine lists
      return ++_nextFreePort;
    }
  }
}


