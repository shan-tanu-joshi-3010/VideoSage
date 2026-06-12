#ifndef PROCESSING_H
#define PROCESSING_H

#include <filesystem>
#include <string>

#include "crow/json.h"

namespace fs = std::filesystem;

/**
 * @brief Extract audio from a video using FFmpeg.
 *
 * Converts the input video to mono 16kHz WAV format.
 *
 * @param video Path to the input video file.
 * @return Path to the generated WAV file.
 *
 * @throws std::runtime_error if extraction fails.
 */
fs::path extract_audio(const fs::path& video);

/**
 * @brief Generate transcript from audio using Whisper.cpp.
 *
 * Uses whisper-cli to transcribe the audio file.
 *
 * @param audio Path to the WAV audio file.
 * @return Transcript as a string.
 *
 * @throws std::runtime_error if Whisper fails
 *         or returns an empty transcript.
 */
std::string whisper_transcribe(
    const fs::path& audio);

/**
 * @brief Send transcript to the Colab summarization API.
 *
 * @param payload JSON payload containing:
 *        {
 *            "video_id": "...",
 *            "transcript": "..."
 *        }
 *
 * @return Raw JSON response from the Colab API.
 *
 * @throws std::runtime_error if API call fails.
 */
std::string call_colab(
    const crow::json::wvalue& payload);

/**
 * @brief Extract summary text from Colab response.
 *
 * Expected response format:
 * {
 *     "summary": "..."
 * }
 *
 * @param response Raw JSON response string.
 * @return Summary text.
 *
 * @throws std::runtime_error if response format is invalid.
 */
std::string extract_summary_from_colab(
    const std::string& response);

#endif // PROCESSING_H