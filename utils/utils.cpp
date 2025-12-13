#include <string>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

std::string inputCheck(int argc, char **argv)
{
    std::string filePath;

        if (argc == 1)
            filePath = "configReader/conf/default.conf";
        else if (argc == 2)
            filePath = argv[1];
        else
            throw std::runtime_error("Usage: " + std::string(argv[0]) + " [config_file]");
        if (access(filePath.c_str(), F_OK) == -1)
            throw std::runtime_error("Config file: " + std::string(std::strerror(errno)));

    return filePath;
}