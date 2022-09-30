import QtQuick 2.0
import QtLocation 5.6
import QtPositioning 5.6
import QtQuick.Shapes 1.1

Rectangle {
    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin { name: "osm"; }
        center:  QtPositioning.coordinate(59.91, 10.75) // Oslo
        zoomLevel: 5

        MapPolyline {
            id: track
            line.width: 3
            line.color: 'red'
        }

        CoordinateAnimation {
            id: centerAnimation;
            target: map;
            properties: "center";
            duration: 300
            easing.type: Easing.InOutQuad
        }

        Connections {
            target: controller
            function onTrackChanged() {
                var lines = []
                for(var i = 0; i < controller.path.size(); i++){
                    lines[i] = controller.path.coordinateAt(i)
                }
                track.path = lines;
            }
            function onCenterChanged() {
                centerAnimation.from = map.center
                centerAnimation.to = controller.center
                centerAnimation.start()
            }
            function onZoomChanged() {
                map.zoomLevel = controller.zoom
            }
        }

        MapItemView {
            model: controller
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(_latitude_, _longitude_)
                anchorPoint: Qt.point(thumbnail.width * 0.5, thumbnail.height * 1.125)
                sourceItem: Shape {
                    id: thumbnail
                    width: 32
                    height: 32
                    visible: true

                    Image {
                        id: pic
                        source: _pixmap_
                    }

                    Text {
                        text: _name_
                        color: selection.current === _index_ ? "blue" : "black"
                        anchors.horizontalCenter: pic.horizontalCenter
                        anchors.bottom: pic.top
                        anchors.topMargin: 5
                    }

                    ShapePath {
                        id: shape
                        strokeColor: selection.current === _index_ ? "blue" : "black"

                        property real half: thumbnail.width * 0.5
                        property real quarter: thumbnail.width * 0.25
                        property point center: Qt.point(thumbnail.x + thumbnail.width * 0.5 , thumbnail.y + thumbnail.height * 0.5)

                        property point topLeft: Qt.point(center.x - half, center.y - half)
                        property point topRight: Qt.point(center.x + half, center.y - half)
                        property point bottomLeft: Qt.point(center.x - half, center.y + half)
                        property point bottomRight: Qt.point(center.x + half, center.y + half)
                        property point bottomCenter: Qt.point(center.x, center.y + half + quarter)

                        startX: shape.bottomLeft.x; startY: shape.bottomLeft.y

                        PathLine { x: shape.topLeft.x; y: shape.topLeft.y }
                        PathLine { x: shape.topRight.x; y: shape.topRight.y }
                        PathLine { x: shape.bottomRight.x; y: shape.bottomRight.y }
                        PathLine { x: shape.bottomCenter.x; y: shape.bottomCenter.y }
                        PathLine { x: shape.bottomLeft.x; y: shape.bottomLeft.y }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: { selection.current = _index_ }
                    }
                }
            }
        }

//        onZoomLevelChanged: console.log(map.zoomLevel)
//        onCenterChanged: console.log(map.center)
    }
}
