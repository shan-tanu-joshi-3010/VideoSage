import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia 6.5

Item {
    id: root
    anchors.fill: parent
    property string videoId

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // ======================
        // 🔙 BACK BUTTON
        // ======================
        Button {
            text: "← Back"
            Layout.preferredWidth: 100
            onClicked: stack.pop()
        }

        // ======================
        // 🎬 MEDIA PLAYER
        // ======================
        MediaPlayer {
            id: player
            source: NetworkManager.streamUrl(videoId)
            autoPlay: true
            videoOutput: videoOut
            audioOutput: audioOut
        }

        AudioOutput {
            id: audioOut
            volume: 0.7
        }

        // ======================
        // 📺 VIDEO DISPLAY
        // ======================
        VideoOutput {
            id: videoOut
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: true
        }

        // ======================
        // ⏳ SEEK SLIDER + TIME
        // ======================
        RowLayout {
            Layout.fillWidth: true

            Slider {
                id: seekSlider
                Layout.fillWidth: true
                from: 0
                to: player.duration
                value: player.position

                // Dragging slider updates the player
                onMoved: player.position = value
            }

            Text {
                text: {
                    function fmt(ms) {
                        if (ms <= 0 || isNaN(ms)) return "00:00"
                        return Qt.formatTime(Math.floor(ms / 1000), "mm:ss")
                    }
                    fmt(player.position) + " / " + fmt(player.duration)
                }
                font.pixelSize: 14
            }

        }

        // ======================
        // ▶️ PLAYBACK CONTROLS
        // ======================
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // PLAY / PAUSE
            Button {
                text: player.playbackState === MediaPlayer.PlayingState ? "⏸ Pause" : "▶ Play"
                onClicked: {
                    if (player.playbackState === MediaPlayer.PlayingState)
                        player.pause()
                    else
                        player.play()
                }
            }

            // STOP
            Button {
                text: "⏹ Stop"
                onClicked: player.stop()
            }

            // SUMMARIZE
            Button {
                text: "🧠 Summarize"
                onClicked: {
                    summaryPopup.open()
                    summaryText.text = "⏳ Summarizing..."
                    NetworkManager.summarizeVideo(videoId)
                }
            }

            // ======================
            // 🎛 PLAYBACK SPEED
            // ======================
            ComboBox {
                id: speedBox
                model: ["0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "2.0x"]
                currentIndex: 2

                onCurrentIndexChanged: {
                    const speeds = [0.5, 0.75, 1.0, 1.25, 1.5, 2.0]
                    player.playbackRate = speeds[currentIndex]
                }
            }

            // ======================
            // 🔊 VOLUME
            // ======================
            RowLayout {
                spacing: 5

                Text {
                    text: "🔊"
                    font.pixelSize: 20
                }

                Slider {
                    id: volumeSlider
                    from: 0
                    to: 1
                    value: 0.7
                    Layout.preferredWidth: 150
                    onValueChanged: audioOut.volume = value
                }
            }
        }
    }

    // ======================
    // 🧾 SUMMARY POPUP
    // ======================
    Dialog {
        id: summaryPopup
        title: "Video Summary"
        modal: true
        standardButtons: Dialog.Ok
        width: parent.width * 0.8
        height: parent.height * 0.6

        ScrollView {
            anchors.fill: parent
            Text {
                id: summaryText
                wrapMode: Text.WordWrap
                padding: 10
            }
        }
    }

    // Get summary text from C++
    Connections {
        target: NetworkManager
        onSummaryReady: summaryText.text = text
    }
}
