#include <iostream>
#include <X11/Xlib.h>
#include <string>
#include <chrono>
#include <thread>
#include <array>
#include "mini/ini.h"

struct Block{
    long int interval;
    long int nextTime;
    std::string command;
};

std::string runCmd(const std::string& cmd){
    FILE* pipe = popen(cmd.c_str(), "r");
    if(!pipe){
        return "OPENFAIL";
    }
    std::array<char, 1024> buffer;
    std::string result;
    while(fgets(buffer.data(), buffer.size(), pipe) != nullptr){
        result+=buffer.data();
    }
    pclose(pipe);

    if(!result.empty() && result.back() == '\n'){
        result.pop_back();
    }

    return result;
}

std::vector<Block> statusBlocks;

long int currentTime = 0;

int main(){
    std::string configPath;
    configPath += std::getenv("HOME");
    configPath += "/.config/simstatbar/config.ini";

    mINI::INIFile file(configPath);
    mINI::INIStructure ini;
    if(!file.read(ini)){
        std::cerr << "Failed to read INI file!\n";
        return 1;
    }

    for(std::pair<const std::string, mINI::INIMap<std::string>> section : ini){
        mINI::INIMap<std::string>& values = section.second;
        Block cb;
        cb.interval = std::stol(values["interval"]);
        cb.command = values["command"];
        cb.nextTime = 0;
        statusBlocks.push_back(cb);
    }

    Display *display = XOpenDisplay(NULL);
    if(!display){
        std::cerr << "Failed to open X display\n";
        return 1;
    }
    Window root = DefaultRootWindow(display);

    while(true){
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        for(Block& block : statusBlocks){
            if(block.nextTime < currentTime){
                block.nextTime = currentTime + block.interval;
                std::string output = runCmd(block.command);
                XStoreName(display, root, output.c_str());
                XFlush(display);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    XCloseDisplay(display);
    return 0;
}
