#ifndef FFMPEG_WRAPPER_H
#define FFMPEG_WRAPPER_H

#include <string>

bool transcode_to_mp4(const std::string& in, const std::string& out);
bool extract_thumb(const std::string& in, const std::string& out);

#endif
