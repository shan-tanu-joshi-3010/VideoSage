#include "ffmpeg.h"
#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

// Normalize paths to forward slashes
static std::string norm(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

// Run FFmpeg and print result
static int run_ff(const std::string& cmd)
{
    std::cout << "\n=====================\n[FFMPEG CMD]\n" << cmd << "\n";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to open pipe!\n";
        return -1;
    }

    char buffer[512];
    std::string output;

    while (fgets(buffer, sizeof(buffer), pipe))
        output += buffer;

    int code = pclose(pipe);

    std::cout << "[FFMPEG OUTPUT]\n" << output << "\n";
    std::cout << "[EXIT CODE] " << code << "\n=====================\n\n";

    return code;
}

bool transcode_to_mp4(const std::string& in, const std::string& out)
{
    std::string nin = norm(in);
    std::string nout = norm(out);

    const std::string FFMPEG_PATH =
        "D:/ffmpeg/ffmpeg-8.0.1-essentials_build/bin/ffmpeg.exe";

    std::ostringstream cmd;
    cmd << FFMPEG_PATH
        << " -y -i \"" << nin << "\" "
        << "-c:v libx264 -preset veryfast -crf 23 "
        << "-c:a aac -b:a 128k -movflags +faststart \"" << nout << "\" 2>&1";

    return run_ff(cmd.str()) == 0;
}

bool extract_thumb(const std::string& in, const std::string& out)
{
    std::string nin = norm(in);
    std::string nout = norm(out);

    const std::string FFMPEG_PATH =
        "D:/ffmpeg/ffmpeg-8.0.1-essentials_build/bin/ffmpeg.exe";

    std::ostringstream cmd;
    cmd << FFMPEG_PATH
        << " -y -ss 00:00:02 -i \"" << nin
        << "\" -frames:v 1 \"" << nout << "\" 2>&1";

    return run_ff(cmd.str()) == 0;
}
