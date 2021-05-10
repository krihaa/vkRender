#pragma once

#ifndef LOG_H
#define LOG_H

#include <iostream>

void inline warn(const std::string&& msg)
{
    std::cerr << "[WARNING]: " << msg << std::endl;
}

void inline warn(const char* msg)
{
    std::cerr << "[WARNING]: " << msg << std::endl;
}

#endif
