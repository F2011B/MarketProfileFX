import QtQuick 2.6
import QtQuick.Controls 2.0

Item {

    Column {
        spacing: 40
        width: parent.width

        Label {
            width: parent.width
            wrapMode: Label.Wrap
            horizontalAlignment: Qt.AlignHCenter
            text: "StockSymbols"
        }

        Tumbler {
            model: 10
            visibleItemCount: 5
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

}
