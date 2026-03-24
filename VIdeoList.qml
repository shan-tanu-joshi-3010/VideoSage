import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs   // ✔ Correct

Item {
    anchors.fill: parent

    Component.onCompleted: NetworkManager.fetchVideos()

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Button {
            text: "Upload Video"
            onClicked: fileDialog.open()
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: NetworkManager.videos

            delegate: Rectangle {
                height: 100
                width: parent.width
                radius: 10
                color: "#f0f0f0"

                RowLayout {
                    anchors.fill: parent
                    spacing: 10

                    Image {
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 90
                        fillMode: Image.PreserveAspectFit
                        source: NetworkManager.thumbUrl(modelData.thumb)
                    }

                    ColumnLayout {
                        Text { text: modelData.title; font.bold: true }
                        Text { text: "Size: " + modelData.size_bytes }
                        Text { text: "At: " + modelData.created_at }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stack.push("VideoPlayerPage.qml", {
                        videoId: modelData.id
                    })
                }
            }
        }
    }

    Dialogs.FileDialog {   // ✔ Correct FileDialog
        id: fileDialog
        title: "Select a video"
        nameFilters: ["Video files (*.mp4 *.avi *.mov)"]
        fileMode: Dialogs.FileDialog.OpenFile

        onAccepted: {
            let localPath = fileDialog.selectedFile.toString().replace("file:///", "")
            console.log("Local path:", localPath)
            NetworkManager.upload(localPath, "Uploaded Video")
        }

    }


}

