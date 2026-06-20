#include <iostream>
#include <filesystem>

#include <rdkafkacpp.h>
#include <crow.h>

#include "processing/processing.h"

namespace fs = std::filesystem;

void processVideo(const std::string& videoId,
                  const std::string& videoPath)
{
    try
    {
        std::cout << "\n========== PROCESSING ==========\n";
        std::cout << "Video ID   : " << videoId << std::endl;
        std::cout << "Video Path : " << videoPath << std::endl;

        /* Extract audio */

        fs::path audio =
            extract_audio(videoPath);

        std::cout << "[FFMPEG] Audio extracted: "
                  << audio << std::endl;

        /* Transcribe */

        std::string transcript =
            whisper_transcribe(audio);

        std::cout << "[WHISPER] Transcription completed\n";

        std::cout << "\n===== TRANSCRIPT =====\n";
        std::cout << transcript.substr(0, 1000);
        std::cout << "======================\n";

        /* Summarize */

        crow::json::wvalue payload;

        payload["video_id"] = videoId;
        payload["transcript"] = transcript;

        std::string modelResponse =
            call_colab(payload);

        std::string summary =
            extract_summary_from_colab(
                modelResponse);

        std::cout << "\n====== SUMMARY ======\n";
        std::cout << summary << std::endl;
        std::cout << "=====================\n";

        /* Cleanup */

        if (fs::exists(audio))
        {
            fs::remove(audio);

            std::cout << "[CLEANUP] Removed "
                      << audio << std::endl;
        }

        std::cout << "[SUCCESS] Processing complete for "
                  << videoId << std::endl;

        std::cout << "==============================\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n[WORKER ERROR] "
                  << e.what()
                  << std::endl;

        std::cerr << "Failed Video ID: "
                  << videoId
                  << std::endl;
    }
}

int main()
{
    std::string brokers = "localhost:9092";
    std::string topic = "video-jobs";

    std::string errstr;

    auto conf =
        RdKafka::Conf::create(
            RdKafka::Conf::CONF_GLOBAL);

    conf->set(
        "bootstrap.servers",
        brokers,
        errstr);

    conf->set(
        "group.id",
        "video-worker-group",
        errstr);

    conf->set(
        "auto.offset.reset",
        "earliest",
        errstr);

    auto consumer =
        RdKafka::KafkaConsumer::create(
            conf,
            errstr);

    if (!consumer)
    {
        std::cerr
            << "Consumer creation failed: "
            << errstr
            << std::endl;

        return 1;
    }

    auto error =
        consumer->subscribe({topic});

    if (error != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Subscribe failed: "
                  << RdKafka::err2str(error)
                  << std::endl;

        return 1;
    }

    std::cout << "Worker started..."
              << std::endl;

    while (true)
    {
        auto msg =
            consumer->consume(1000);

        switch (msg->err())
        {
        case RdKafka::ERR_NO_ERROR:
        {
            try
            {
                std::string payload(
                    static_cast<const char*>(
                        msg->payload()),
                    msg->len());

                std::cout
                    << "\n===== JOB RECEIVED =====\n";

                std::cout
                    << payload
                    << std::endl;

                auto json =
                    crow::json::load(payload);

                if (!json)
                {
                    std::cerr
                        << "Invalid JSON received"
                        << std::endl;

                    break;
                }

                std::string videoId =
                    json["video_id"].s();

                std::string videoPath =
                    json["video_path"].s();

                std::cout << "Original Path : "
                        << videoPath
                        << std::endl;

                /* Convert Windows path to Codespaces path */
                if (videoPath.find("D:/") == 0)
                {
                    videoPath =
                        "/workspaces/VideoSage/" +
                        videoPath;
                }

                std::cout << "Resolved Path: "
                        << videoPath
                        << std::endl;

                /* Verify video exists */
                if (!fs::exists(videoPath))
                {
                    std::cerr << "[ERROR] Video not found: "
                            << videoPath
                            << std::endl;
                    break;
                }

                processVideo(
                    videoId,
                    videoPath);
            }
            catch (const std::exception& e)
            {
                std::cerr
                    << "[PROCESSING ERROR] "
                    << e.what()
                    << std::endl;
            }

            break;
        }

        case RdKafka::ERR__TIMED_OUT:
            break;

        case RdKafka::ERR__PARTITION_EOF:
            break;

        default:

            std::cerr
                << msg->errstr()
                << std::endl;
        }

        delete msg;
    }

    consumer->close();

    delete consumer;
    delete conf;

    return 0;
}