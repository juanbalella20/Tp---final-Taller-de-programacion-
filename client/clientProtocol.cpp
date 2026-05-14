#include "clientProtocol.h"

ClientProtocol::ClientProtocol(Socket&& socket) : socket(std::move(socket)) {}
