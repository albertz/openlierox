#ifndef PLAYER_INPUT_H
#define PLAYER_INPUT_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include <string>
#include <list>

void registerPlayerInput();

std::string leftStart(int index, const std::list<std::string> &args);
std::string leftStop(int index, const std::list<std::string> &args);
std::string rightStart(int index, const std::list<std::string> &args);
std::string rightStop(int index, const std::list<std::string> &args);
std::string upStart(int index, const std::list<std::string> &args);
std::string upStop(int index, const std::list<std::string> &args);
std::string downStart(int index, const std::list<std::string> &args);
std::string downStop(int index, const std::list<std::string> &args);
std::string fireStart(int index, const std::list<std::string> &args);
std::string fireStop(int index, const std::list<std::string> &args);
std::string jumpStart(int index, const std::list<std::string> &args);
std::string jumpStop(int index, const std::list<std::string> &args);
std::string changeStart(int index, const std::list<std::string> &args);
std::string changeStop(int index, const std::list<std::string> &args);
std::string say( const std::list<std::string> &args );

#endif // _PLAYER_INPUT_H_
