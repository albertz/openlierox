#ifndef PLAYER_INPUT_H
#define PLAYER_INPUT_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include <string>
#include <list>
#include "CWormHuman.h"

void registerPlayerInput();

std::string eventStart(size_t index, CWormHumanInputHandler::Actions action, std::list<std::string> const& = std::list<std::string>());
std::string eventStop(size_t index, CWormHumanInputHandler::Actions action, std::list<std::string> const& = std::list<std::string>());

std::string say( const std::list<std::string> &args );

#endif // _PLAYER_INPUT_H_
