#include <iostream>
#include <X11/Xlib.h>
#include <string>
#include <chrono>
#include <thread>
#include <array>
#include <vector>
#include "mini/ini.h"

struct Block{
    long int interval;
    long int nextTime;
    std::string command;
    std::string output;
};

std::vector<std::string> split(const std::string& input, const std::string& delimiter){
    std::vector<std::string> chunks;
    size_t start = 0;
    size_t end = 0;
    while((end = input.find_first_of(delimiter, start))!=std::string::npos){
        if(start!=end) chunks.push_back(input.substr(start, end-start));
        start = end + 1;
    }
    if(start < input.length()) chunks.push_back(input.substr(start));
    return chunks;
}

std::string trim(const std::string& input){
    size_t first = input.find_first_not_of(" \t\n\r");
    if(first == std::string::npos) return "";
    size_t last = input.find_last_not_of(" \t\n\r");
    return input.substr(first, (last-first+1));
}

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
std::string delimiter;

long int currentTime = 0;
std::string outLine;

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

    delimiter = ini["settings"]["delimiter"];
    std::vector<std::string> blocks = split(ini["settings"]["blocks"], ",");

    for(std::string sectName : blocks){
        mINI::INIMap<std::string>& values = ini[trim(sectName)];
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
        outLine = "";
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        for(int i = 0; i < statusBlocks.size(); i++){
            Block& block = statusBlocks[i];
            if(block.nextTime < currentTime){
                block.nextTime = currentTime + block.interval;
                block.output = runCmd(block.command);
            }
            outLine += (i==(statusBlocks.size()-1)) ? block.output : block.output + delimiter;
        }
        XStoreName(display, root, outLine.c_str());
        XFlush(display);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    XCloseDisplay(display);
    return 0;
}
