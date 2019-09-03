//
// Created by jxq on 19-9-3.
//

#include "Connector.h"

Connector(EventLoop* loop, const InetAddress& serverAddr);
~Connector();

void start();
void restart();
void stop();