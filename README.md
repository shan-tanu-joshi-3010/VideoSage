# 🎥 VidSage – Intelligent Video Summarization System

VidSage is a full-stack video processing and summarization system that enables users to upload, stream, and generate AI-powered summaries of videos.  
The system is designed with a scalable backend architecture where heavy processing is offloaded from the UI to a server and further delegated to an AI model hosted on Google Colab.

---

## 🚀 Features

- 📤 Upload videos to server
- 🎬 Stream videos with partial content support (Range requests)
- 🖼️ Automatic thumbnail generation
- 🎧 Audio extraction using FFmpeg
- 📝 Speech-to-text transcription using Whisper
- 🤖 AI-powered summarization via Mistral 7B (hosted on Colab)
- ⚡ Asynchronous architecture (UI remains responsive)
- 🌐 Local + remote hybrid processing (Crow + Flask + ngrok)

---

## 🧠 Motivation

Traditional platforms like YouTube:
- Do not provide semantic summaries of videos
- Require full video consumption for understanding
- Lack developer-level customization

VidSage solves this by:
- Providing instant summaries of video content
- Enabling backend-level control over processing
- Supporting research and experimentation with AI pipelines

---

## ⚙️ Tech Stack

### Frontend
- Qt (C++)
- QML (UI layer)
- QNetworkAccessManager (API calls)

### Backend (Primary Server)
- C++ (Crow Framework)
- FFmpeg (video/audio processing)
- Whisper.cpp (speech-to-text)
- REST APIs

### AI Layer (External)
- Python (Flask server)
- Mistral 7B (4-bit quantized)
- Hosted on Google Colab
- ngrok (for tunneling)

---

## 🏗️ System Architecture


[ QML UI ]
|
| (HTTP Requests)
↓
[ Crow C++ Server ]
|
|-- Extract Audio (FFmpeg)
|-- Transcription (Whisper)
|-- JSON Creation
|
↓
[ Flask Server (Colab via ngrok) ]
|
|-- Mistral 7B Model
|
↓
[ Summary Response ]
|
↓
[ Crow Server → UI ]


---

## 🔄 Working Mechanism

### 1. Video Upload
- User selects video from UI
- Video is sent to Crow server via HTTP PUT
- Server stores video in `media/encoded`

---

### 2. Video Streaming
- UI requests `/videos/<id>/stream`
- Server supports byte-range requests (partial streaming)
- Efficient playback without full download

---

### 3. Summarization Pipeline

#### Step 1: Trigger
- User clicks "Summarize" in UI

#### Step 2: Server Processing
- Extract audio from video using FFmpeg
- Convert to 16kHz mono WAV

#### Step 3: Transcription
- Whisper CLI processes audio
- Generates transcript

#### Step 4: AI Processing
- Transcript sent to Flask server (Colab)
- Mistral 7B generates summary

#### Step 5: Response
- Summary returned to Crow server
- UI displays result

---

## ⚡ Advantages Over YouTube

| Feature | VidSage | YouTube |
|--------|--------|--------|
| AI Summary | ✅ Yes | ❌ No |
| Developer Control | ✅ Full | ❌ None |
| Custom Pipeline | ✅ Yes | ❌ No |
| Local Processing | ✅ Yes | ❌ No |
| Research Friendly | ✅ Yes | ❌ No |

---

## 📊 Key Design Decisions

### 1. Server-side Processing
Heavy tasks (FFmpeg, Whisper) are moved to backend to:
- Prevent UI freezing
- Enable scalability
- Support remote streaming

---

### 2. External AI Model (Colab)
- Offloads GPU-heavy inference
- Avoids local hardware limitations
- Enables experimentation with large models

---

### 3. Asynchronous Communication
- UI remains responsive
- Network calls handled via signals/slots
- No blocking operations in frontend

---

## ⚠️ Current Limitations

- Whisper is CPU-bound → scalability bottleneck
- No job queue → concurrent requests may overload server
- ngrok dependency → unstable for production
- No authentication system
- Limited error handling for large-scale usage

---

## 🔮 Future Scope

### 🔹 1. Queue-based Processing
- Introduce task queue (Redis / RabbitMQ)
- Worker pool for Whisper processing

---

### 🔹 2. GPU Acceleration
- Move Whisper to GPU
- Reduce transcription time significantly

---

### 🔹 3. Model Hosting
- Deploy Mistral locally or on cloud VM
- Remove ngrok dependency

---

### 🔹 4. Semantic Indexing
- Store transcripts
- Enable search within videos

---

### 🔹 5. Multi-user Support
- Authentication system
- User-specific video storage

---

### 🔹 6. Real-time Summarization
- Stream-based summarization (live processing)

---

## 📁 Project Structure


videoplayer_ui/
├── QML UI
├── NetworkManager.cpp

videoplayer_server/
├── main.cpp (Crow server)
├── media/
│ ├── encoded/
│ ├── thumbs/
│ ├── original/

colab_server/
├── Flask app
├── Mistral model


---

## 🧪 How to Run

### Backend (Crow)

./videoplayer_server
Colab
Run Flask server
Start ngrok tunnel
Update URL in server
UI
Run Qt application
Upload and summarize videos 
### 🧠 Learnings from This Project
Designing multi-component systems
Handling real-world failures (paths, processes, APIs)
Understanding bottlenecks (CPU-bound tasks)
Integrating C++ with AI pipelines
Building asynchronous UI-driven applications


---

## ⚠️ Disadvantages / Limitations

This system is functional but not production-ready. Key limitations include:

### 🔻 1. CPU Bottleneck (Whisper)
- Whisper transcription runs on CPU
- High latency for long videos
- Cannot handle multiple concurrent requests efficiently
- Becomes the first point of failure under load

---

### 🔻 2. No Task Queue / Job Management
- All requests are processed synchronously
- No buffering or scheduling of jobs
- Concurrent summarization requests may:
  - Overwrite temp files
  - Spawn multiple heavy processes
  - Crash the server

---

### 🔻 3. Dependency on External Tunnel (ngrok)
- Colab connection relies on ngrok
- URL changes frequently
- Not stable for production systems
- Adds network latency

---

### 🔻 4. Weak Fault Tolerance
- Failure in any stage breaks entire pipeline:
  - FFmpeg failure
  - Whisper failure
  - Colab timeout
- No retry mechanism or fallback

---

### 🔻 5. No Persistent Metadata Storage
- Videos are stored in filesystem only
- No structured DB for:
  - transcripts
  - summaries
  - user data
- Limits scalability and querying

---

### 🔻 6. Security Limitations
- No authentication or authorization
- Open endpoints (upload, summarize)
- Vulnerable to misuse or spam

---

### 🔻 7. Platform Dependency (Windows-specific)
- Uses `_popen` and Windows command formatting
- Not easily portable to Linux/Mac without modification

---

### 🔻 8. No Resource Isolation
- FFmpeg + Whisper share system resources
- No containerization or sandboxing
- Risk of system slowdown or crashes

---

## 🔮 Future Improvements

### 🚀 1. Introduce Job Queue System (HIGH IMPACT)
- Use Redis / RabbitMQ
- Decouple request from processing
- Implement worker-based execution
- Prevent server overload

---

### 🚀 2. Worker Pool for Whisper
- Dedicated transcription workers
- Parallel processing with controlled concurrency
- Avoid CPU saturation

---

### 🚀 3. GPU Acceleration
- Move Whisper to GPU (CUDA / PyTorch version)
- Reduce transcription time by 5–10x
- Improve scalability

---

### 🚀 4. Replace ngrok with Stable Deployment
- Deploy Flask model on:
  - AWS EC2
  - GCP VM
  - Docker container
- Use fixed API endpoint

---

### 🔻 5. Limited Database Usage (SQLite Constraints)

- SQLite is used for storing video metadata
- Works well for local, low-scale usage

However:

- Not suitable for high concurrency (multiple writes)
- No horizontal scalability
- Limited performance under heavy load
- Lacks advanced features like:
  - distributed queries
  - replication
  - high availability

👉 Current usage is sufficient for development but not production-grade systems
---

### 🚀 6. Caching Layer
- Cache summaries for already processed videos
- Avoid redundant computation
- Improve response time

---

### 🚀 7. Chunk-Based Processing
- Split long videos into segments
- Process in parallel
- Combine summaries
- Improves performance for large videos

---

### 🚀 8. Real-time Streaming Summarization
- Process video while streaming
- Generate incremental summaries
- Move toward live summarization

---

### 🚀 9. Microservice Architecture
Break system into services:
- Video Service
- Transcription Service
- AI Service
- API Gateway

Improves scalability and maintainability

---

### 🚀 10. Containerization (Docker)
- Isolate FFmpeg, Whisper, and server
- Ensure portability across environments
- Simplify deployment

---

### 🚀 11. Add Authentication System
- User accounts
- Video ownership
- Access control

---

### 🚀 12. Frontend Improvements
- Progress bar for summarization
- Status updates (processing, completed, failed)
- Better error feedback

---

### 🚀 13. Monitoring & Logging
- Track:
  - request latency
  - failures
  - CPU usage
- Use tools like Prometheus / Grafana

---

### 🚀 14. Replace CLI Calls with Native Integration
- Avoid `_popen` and system calls
- Integrate Whisper via C++ bindings or Python service
- Improve reliability

---

## 🧠 Engineering Insight

This project highlights a critical real-world principle:

> The biggest challenge is not building features,  
> but managing **compute-heavy pipelines under constraints**.

Key takeaways:
- CPU-bound tasks limit scalability
- External dependencies introduce instability
- Proper task orchestration is essential for production systems
