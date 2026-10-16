#include <iostream>
#include <X11/Xlib.h>
#include <string>
#include <chrono>
#include <thread>
#include <array>
#include <vector>
#include <mutex>
#include <functional>
#include <string_view>
#include <cstdlib>
#include <cstdio>
#include "inirw/inirw.h"

struct Block{
    long int interval;
    std::string command;
    std::string output;
};

constexpr std::string_view spaces = " \t\n\r";
constexpr std::string_view quotes = "\"'";

std::vector<std::string> split(std::string_view input, std::string_view delimiter){
    std::vector<std::string> chunks;
    size_t start = 0;
    size_t end = 0;
    while((end = input.find_first_of(delimiter, start))!=std::string::npos){
        if(start!=end) chunks.push_back(std::string(input.substr(start, end-start)));
        start = end + 1;
    }
    if(start < input.length()) chunks.push_back(std::string(input.substr(start)));
    return chunks;
}

std::string trim(std::string_view input, std::string_view tT){
    size_t first = input.find_first_not_of(tT);
    if(first == std::string::npos) return "";
    size_t last = input.find_last_not_of(tT);
    return std::string(input.substr(first, (last-first+1)));
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

Display *display;
Window root;

std::mutex mtx;

void upd(){
    std::string line = "";
    for(size_t i = 0; i<statusBlocks.size(); ++i){
        line+=(i==0 ? "" : delimiter) + statusBlocks[i].output;
    }
    XStoreName(display, root, line.c_str());
    XFlush(display);
}

void sT(Block& block){
    std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();
    while(true){
        next+=std::chrono::milliseconds(block.interval);
        std::string out = runCmd(block.command);
        {
            std::lock_guard<std::mutex> lock(mtx);
            block.output=out;
            upd();
        }
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if(next<now) next = now;
        std::this_thread::sleep_until(next);
    }
}

int main(){
    display = XOpenDisplay(NULL);
    if(!display){
        std::cerr << "Failed to open X display\n";
        return 1;
    }
    root = DefaultRootWindow(display);

    std::string configPath;
    configPath += std::getenv("HOME");
    configPath += "/.config/simstatbar/config.ini";

    INIRW::Ini ini;
    INIRW::read(configPath, ini);

    delimiter = trim(ini["settings"]["delimiter"], quotes);
    std::vector<std::string> blocks = split(ini["settings"]["blocks"], ",");

    for(const std::string& sectName : blocks){
        INIRW::SectData& values = ini[trim(sectName, spaces)];
        Block cb;
        cb.interval = std::stol(trim(values["interval"], quotes));
        cb.command = trim(values["command"], quotes);
        statusBlocks.push_back(cb);
    }

    std::vector<std::thread> threads;

    for(Block& block : statusBlocks) threads.emplace_back(sT, std::ref(block));
    for(std::thread& t : threads) t.join();

    return 0;
}
