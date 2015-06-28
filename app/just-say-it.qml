import QtQuick 2.0
import Ubuntu.Components 1.1
import Ubuntu.Content 1.1
import Ubuntu.Components.ListItems 0.1 as ListItem
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
    applicationName: "just-say-it.ted"

    /*
     This property enables the application to change orientation
     when the device is rotated. The default is false.
    */
    automaticOrientation: true

    // Removes the old toolbar and enables new features of the new header.
    useDeprecatedToolbar: false

    width: units.gu(100)
    height: units.gu(76)

    PageStack {
        id: pageStack
        Component.onCompleted: pageStack.push(root)

        Page {
            id: root
            title: i18n.tr("Just say it!")

            AudioRecorder {
                id: recorder
            }

            states: [
                State {
                    name: "recording"
                    when: recorder.state == "recording"
                    PropertyChanges {
                        target: button
                        text: i18n.tr("Stop Recording")
                    }
                },
                State {
                    name: "stopped"
                    when: recorder.state == "stopped"
                    PropertyChanges {
                        target: button
                        text: i18n.tr("Start Recording")
                    }
                },
                State {
                    name: "upload"
                    when: recorder.state == "uploading"
                    PropertyChanges {
                        target: button
                        text: i18n.tr("Stop Upload")
                    }
                },
                State {
                    name: "processing"
                    when: recorder.state == "processing"
                    PropertyChanges {
                        target: button
                        text: i18n.tr("Cancel Processing")
                    }
                }
            ]

            Column {
                spacing: units.gu(1)
                anchors {
                    margins: units.gu(2)
                    fill: parent
                }

                Button {
                    id: button
                    objectName: "button"
                    width: parent.width

                    onClicked: {
                        if (recorder.state == "stopped")
                            recorder.record()
                        else
                            recorder.stop()
                    }
                }

                TextArea {
                    id: label
                    objectName: "label"
                    width: parent.width

                    readOnly: true
                    placeholderText: i18n.tr("No Detected Text")
                    text: recorder.text
                    autoSize: true
                }

                ListItem.ItemSelector {
                    id: langSelector
                    showDivider: false
                    expanded: false
                    model: ['en-US', 'en-GB', 'de-DE', 'es-ES', 'fr-FR', 'it-IT', 'zh-CN']
                    delegate: OptionSelectorDelegate {
                        objectName: "selectLang" + modelData
                        text: {
                            return {
                                'en-US': i18n.tr("English (United States)"),
                                'en-GB': i18n.tr("English (British)"),
                                'de-DE': i18n.tr("German"),
                                'es-ES': i18n.tr("European Spanish"),
                                'fr-FR': i18n.tr("French"),
                                'it-IT': i18n.tr("Italian"),
                                'zh-CN': i18n.tr("Chinese (Mandarin)")
                            }[modelData]
                        }
                        showDivider: false
                    }
                    selectedIndex: model.indexOf('en-US')
                    onDelegateClicked: {
                        print("Language: " + model[index]);
                    }
                }

                Button {
                    id: useButton
                    objectName: "useButton"
                    width: parent.width
                    text: i18n.tr("Share Text")
                    onClicked: pageStack.push(picker, {"text": recorder.text})
                    enabled: recorder.text != ""
                }

                Button {
                    id: closeButton
                    objectName: "useButton"
                    width: parent.width
                    text: i18n.tr("Close")
                    onClicked: Qt.quit()
                }

            }
        }

        Page {
            id: picker
            visible: false
            property var text

            Component {
                id: resultComponent
                ContentItem {}
            }

            ContentPeerPicker {
                id: peerPicker

                visible: parent.visible
                contentType: ContentType.Text
                handler: ContentHandler.Share

                onCancelPressed: {
                    print ("onCancelPressed");
                    pageStack.pop();
                }

                onPeerSelected: {
                    print ("onPeerSelected: " + peer.name);
                    print ("sending text: " + picker.text);
                    var request = peer.request();
                    request.items = [ resultComponent.createObject(root, {"text": picker.text}) ];
                    request.state = ContentTransfer.Charged;
                    pageStack.pop();
                }
            }
        }
    }
}

