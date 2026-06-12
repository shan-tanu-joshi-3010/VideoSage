#include "processing.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <array>
#include <algorithm>

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

static const std::string COLAB_URL =
    "https://1712-34-125-96-14.ngrok-free.app/summarize";

/* ======================= AUDIO EXTRACTION ======================= */

fs::path extract_audio(const fs::path& video)
{
    fs::path audio =
        video.parent_path() /
        (video.stem().string() + ".wav");

    std::string cmd =
        "ffmpeg -y -i \"" +
        video.string() +
        "\" -ar 16000 -ac 1 \"" +
        audio.string() +
        "\"";

    std::cout
        << "[FFMPEG CMD] "
        << cmd
        << std::endl;

    if (std::system(cmd.c_str()) != 0)
    {
        throw std::runtime_error(
            "audio extraction failed");
    }

    return audio;
}

/* ======================= WHISPER ======================= */

std::string whisper_transcribe(
    const fs::path& audio)
{
#ifdef _WIN32

    std::string whisperExe =
        "D:/qt_projects/videoplayer_server/"
        "whisper/whisper-cli.exe";

    std::string modelPath =
        "D:/qt_projects/videoplayer_server/"
        "whisper/ggml-small.bin";

#else

    std::string whisperExe =
        "/workspaces/VideoSage/"
        "whisper.cpp/build/bin/whisper-cli";

    std::string modelPath =
        "/workspaces/VideoSage/"
        "whisper.cpp/models/"
        "for-tests-ggml-small.bin";

#endif

    std::string audioPath =
        audio.string();

    std::string innerCmd =
        "\"" + whisperExe + "\""
        " -m \"" + modelPath + "\""
        " -f \"" + audioPath + "\""
        " --no-timestamps";

#ifdef _WIN32
    std::string cmd =
        "cmd /C \"" + innerCmd + "\"";
#else
    std::string cmd =
        innerCmd;
#endif

    std::cout
        << "[WHISPER CMD] "
        << cmd
        << std::endl;

    std::array<char, 4096> buffer;
    std::string result;

    FILE* pipe =
        POPEN(cmd.c_str(), "r");

    if (!pipe)
    {
        throw std::runtime_error(
            "whisper failed to start");
    }

    while (
        fgets(buffer.data(),
              buffer.size(),
              pipe))
    {
        result += buffer.data();
    }

    PCLOSE(pipe);

    if (result.empty())
    {
        throw std::runtime_error(
            "empty transcript");
    }

    return result;
}

/* ======================= COLAB CALL ======================= */

std::string call_colab(
    const crow::json::wvalue& payload)
{
    std::string tmp =
        "payload.json";

    {
        std::ofstream ofs(tmp);

        ofs << payload.dump();
    }

    std::string cmd =
        "curl -s --max-time 60 "
        "-X POST " +
        COLAB_URL +
        " -H \"Content-Type: "
        "application/json\""
        " --data @" + tmp;

    std::cout
        << "[COLAB CMD] "
        << cmd
        << std::endl;

    std::array<char, 4096> buffer;
    std::string response;

    FILE* pipe =
        POPEN(cmd.c_str(), "r");

    if (!pipe)
    {
        throw std::runtime_error(
            "colab call failed");
    }

    while (
        fgets(buffer.data(),
              buffer.size(),
              pipe))
    {
        response += buffer.data();
    }

    PCLOSE(pipe);

    fs::remove(tmp);

    return response;
}

/* ======================= SUMMARY EXTRACTION ======================= */

std::string extract_summary_from_colab(
    const std::string& response)
{
    auto json =
        crow::json::load(response);

    if (!json ||
        !json.has("summary"))
    {
        throw std::runtime_error(
            "invalid colab response");
    }

    return json["summary"].s();
}