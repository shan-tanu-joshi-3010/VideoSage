// ======================= main.cpp =======================
#define CROW_MAIN
#include "crow.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <array>

#include "io.h"
#include "ffmpeg.h"
#include "kafka/KafkaProducer.h"   

namespace fs = std::filesystem;

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

/* ======================= GLOBALS ======================= */

static fs::path ROOT;
static fs::path P_ORIG;
static fs::path P_ENC;
static fs::path P_THUM;

static const std::string COLAB_URL =
    "https://1712-34-125-96-14.ngrok-free.app/summarize";

/* ======================= HELPERS ======================= */

fs::path get_video_path(const std::string& id) {
    fs::path p = P_ENC / (id + ".mp4");
    if (!fs::exists(p))
        throw std::runtime_error("video not found");
    return p;
}

fs::path extract_audio(const fs::path& video) {
    fs::path audio = video.parent_path() /
                     (video.stem().string() + ".wav");

    std::string cmd =
        "ffmpeg -y -i \"" + video.string() +
        "\" -ar 16000 -ac 1 \"" + audio.string() + "\"";

    if (std::system(cmd.c_str()) != 0)
        throw std::runtime_error("audio extraction failed");

    return audio;
}


std::string win_cmd_path(const fs::path& p)
{
    std::string s = p.u8string();   // UTF-8 safe
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}
std::string whisper_transcribe(const fs::path& audio)
{
    // Absolute paths (Windows-style)
   #ifdef _WIN32

std::string whisperExe =
    "D:/qt_projects/videoplayer_server/whisper/whisper-cli.exe";

std::string modelPath =
    "D:/qt_projects/videoplayer_server/whisper/ggml-small.bin";

#else

std::string whisperExe =
    "/workspaces/VideoSage/whisper.cpp/build/bin/whisper-cli";

std::string modelPath =
    "/workspaces/VideoSage/whisper.cpp/models/for-tests-ggml-small.bin";

#endif

    // IMPORTANT: use native Windows path
    std::string audioPath = audio.string();

    // Inner command (the one that works in CMD)
    std::string innerCmd =
        "\"" + whisperExe + "\""
                            " -m \"" + modelPath + "\""
                      " -f \"" + audioPath + "\""
                      " --no-timestamps";

    // 🔥 CRITICAL FIX: wrap with cmd /C ""
    #ifdef _WIN32
    std::string cmd = "cmd /C \"" + innerCmd + "\"";
#else
    std::string cmd = innerCmd;
#endif

    std::cout << "[WHISPER CMD] " << cmd << std::endl;

    std::array<char, 4096> buffer;
    std::string result;

    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("whisper failed to start");

    while (fgets(buffer.data(), buffer.size(), pipe))
        result += buffer.data();

    PCLOSE(pipe);

    if (result.empty())
        throw std::runtime_error("empty transcript");

    return result;
}




/* ======================= COLAB CALL ======================= */

std::string call_colab(const crow::json::wvalue& payload) {

    std::string tmp = "payload.json";
    {
        std::ofstream ofs(tmp);
        ofs << payload.dump();   // ✅ FIXED
    }

    std::string cmd =
        "curl -s --max-time 60 -X POST " + COLAB_URL +
        " -H \"Content-Type: application/json\""
        " --data @" + tmp;

    std::array<char, 4096> buffer;
    std::string response;

    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("colab call failed");

    while (fgets(buffer.data(), buffer.size(), pipe))
        response += buffer.data();

    PCLOSE(pipe);
    fs::remove(tmp);

    return response;
}


std::string extract_summary_from_colab(const std::string& response)
{
    auto json = crow::json::load(response);
    if (!json || !json.has("summary"))
        throw std::runtime_error("invalid colab response");

    return json["summary"].s();
}

std::unique_ptr<KafkaProducer> kafkaProducer;

/* ======================= MAIN ======================= */

int main(int argc, char* argv[]) {

    kafkaProducer =
    std::make_unique<KafkaProducer>(
        "localhost:9092",
        "video-jobs");

    fs::path exeDir = fs::absolute(fs::path(argv[0])).parent_path();

    ROOT = fs::path("D:/qt_projects/videoplayer_server/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/media");   // 👈 YOUR ABSOLUTE PATH

    P_ORIG = fs::path("D:/qt_projects/videoplayer_server/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/media/original");
    P_ENC  =  fs::path("D:/qt_projects/videoplayer_server/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/media/encoded");
    P_THUM =  fs::path("D:/qt_projects/videoplayer_server/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/media/thumbs");

    fs::create_directories(P_ORIG);
    fs::create_directories(P_ENC);
    fs::create_directories(P_THUM);

    crow::SimpleApp app;

    CROW_ROUTE(app, "/health")([] { return "ok"; });

    /* ======================= LIST VIDEOS ======================= */

    CROW_ROUTE(app, "/videos").methods("GET"_method)
        ([] {
            crow::json::wvalue arr = crow::json::wvalue::list();
            int i = 0;

            for (const auto& f : fs::directory_iterator(P_ENC)) {
                if (f.path().extension() == ".mp4") {

                    std::string id = f.path().stem().string();
                    fs::path thumb = P_THUM / (id + ".jpg");

                    crow::json::wvalue v;
                    v["id"] = id;
                    v["size_bytes"] = (long long)fs::file_size(f.path());

                    // ✅ add thumb ONLY if it exists
                    if (fs::exists(thumb))
                        v["thumb"] = thumb.filename().string();
                    else
                        v["thumb"] = "";

                    arr[i++] = std::move(v);
                }
            }

            return crow::response(arr);
        });

    /* ======================= STREAM ======================= */

    CROW_ROUTE(app, "/videos/<string>/stream")
    ([](const crow::request& req, std::string id) {

        fs::path fpath = get_video_path(id);

        std::ifstream f(fpath, std::ios::binary);
        f.seekg(0, std::ios::end);
        long long size = f.tellg();

        auto r = parse_range(req.get_header_value("Range"), size);

        long long start = r ? r->start : 0;
        long long end   = r ? r->end : size - 1;

        long long len = end - start + 1;
        std::string body(len, '\0');

        f.seekg(start);
        f.read(body.data(), len);

        crow::response resp(r ? 206 : 200);
        resp.body = std::move(body);
        resp.set_header("Content-Type", "video/mp4");
        resp.set_header("Content-Length", std::to_string(len));

        if (r) {
            resp.set_header("Content-Range",
                            "bytes " + std::to_string(start) + "-" +
                                std::to_string(end) + "/" + std::to_string(size));
        }

        return resp;
    });

    /* ======================= 🔥 SUMMARIZE ======================= */

    CROW_ROUTE(app, "/videos/<string>/summarize")
    .methods("POST"_method)
([](std::string id)
{
    try
    {
        std::cout << "[QUEUE SUMMARY] Video ID: "
                  << id << std::endl;

        // Verify video exists
        fs::path video = get_video_path(id);

        // Publish Kafka Job
        bool success =
            kafkaProducer->sendJob(
                id,
                video.string());

        if (!success)
        {
            return crow::response(
                500,
                "Failed to queue summarization job");
        }

        crow::json::wvalue out;

        out["status"] = "QUEUED";
        out["video_id"] = id;
        out["message"] =
            "Summarization job submitted successfully";

        return crow::response(202, out);
    }
    catch (const std::exception& e)
    {
        crow::json::wvalue error;

        error["status"] = "FAILED";
        error["error"] = e.what();

        return crow::response(500, error);
    }
});

    CROW_ROUTE(app, "/thumb/<string>")
    ([](std::string filename) {

        fs::path thumbPath = P_THUM / filename;

        if (!fs::exists(thumbPath))
            return crow::response(404, "Thumbnail not found");

        std::ifstream ifs(thumbPath, std::ios::binary);
        std::string data((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());

        crow::response res(200, data);
        res.set_header("Content-Type", "image/jpeg");
        return res;
    });

    /* ======================= UPLOAD ======================= */

    CROW_ROUTE(app, "/upload/<string>")
    .methods("PUT"_method)
    ([](const crow::request& req, std::string id) {

        try {

            std::string ext = req.url_params.get("ext")
                                ? req.url_params.get("ext")
                                : ".mp4";

            std::string title = req.url_params.get("title")
                                ? req.url_params.get("title")
                                : id;

            if (req.body.empty()) {
                return crow::response(400, "Empty upload");
            }

            // Save original video
            fs::path originalPath = P_ORIG / (id + ext);

            {
                std::ofstream ofs(originalPath, std::ios::binary);

                if (!ofs) {
                    return crow::response(500,
                                        "Failed to create file");
                }

                ofs.write(req.body.data(), req.body.size());
            }

            std::cout << "[UPLOAD] Saved video: "
                    << originalPath << std::endl;

            // For now just copy to encoded folder
            fs::path encodedPath = P_ENC / (id + ".mp4");

            fs::copy_file(originalPath,
                        encodedPath,
                        fs::copy_options::overwrite_existing);

            std::cout << "[UPLOAD] Encoded video: "
                    << encodedPath << std::endl;

            crow::json::wvalue out;

            out["status"] = "success";
            out["video_id"] = id;
            out["title"] = title;

            return crow::response(200, out);
        }
        catch (const std::exception& e) {

            return crow::response(500, e.what());
        }
    });


    app.port(18080).multithreaded().run();
}
