#pragma once

#include "ProtocolManagerImpl.h"

class SessionClient;
class OldGameTalkProtocol;

typedef ProtocolManagerImpl<SessionClient, boost::shared_ptr<OldGameTalkProtocol> > ProtocolManager;

