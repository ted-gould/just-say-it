import QtQuick 2.0
import Ubuntu.Components 1.1
import JustSayIt 1.0

/*!
    \brief MainView with Tabs element.
           First Tab has a single Label and
           second Tab has a single ToolbarAction.
*/

MainView {
    // objectName for functional testing purposes (autopilot-qt5)
    objectName: "mainView"

    // Note! applicationName needs to match the "name" field of the click manifest
    applicationName: "just-say-it.just-say-it"

    /*
     This property enables the application to change orientation
     when the device is rotated. The default is false.
    */
    //automaticOrientation: true

    // Removes the old toolbar and enables new features of the new header.
    useDeprecatedToolbar: false

    width: units.gu(100)
    height: units.gu(76)

    Page {
        title: i18n.tr("Just say it!")

        AudioRecorder {
            id: recorder
        }

        Column {
            spacing: units.gu(1)
            anchors {
                margins: units.gu(2)
                fill: parent
            }

            Button {
                objectName: "button"
                width: parent.width

                text: recorder.state == "stopped" ? i18n.tr("Record") : i18n.tr("Stop")

                onClicked: {
                    if (recorder.state == "stopped")
                        recorder.record()
                    else
                        recorder.stop()
                }
            }

            Label {
                id: recorderState
                objectName: "recorderState"

                text: recorder.state
            }

            Label {
                id: label
                objectName: "label"

                text: recorder.text
            }

        }
    }
}

