import QtQuick 2.14
import QtQuick.Controls 2.14

ApplicationWindow {
    width: 900
    height: 620
    visible: true
    title: "Crow Video Server Client"

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: VideoList { }
    }
}
