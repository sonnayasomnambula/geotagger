import QtQuick 2.15
import QtLocation 5.12
import QtPositioning 5.12

//import org.sonnayasomnambula.pathcontroller 0.1

Rectangle {
    anchors.fill: parent

    Plugin {
        id: mapPlugin
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: pathController.center
        zoomLevel: pathController.zoom

        MapPolyline {
            id: track
            line.width: 3
            line.color: 'red'
        }

//        onZoomLevelChanged: console.log(zoomLevel)
    }

    function load() {
        var lines = []
        for(var i = 0; i < pathController.geopath.size(); i++){
            lines[i] = pathController.geopath.coordinateAt(i)
        }
        return lines;
    }

    Connections {
        target: pathController
        onGeoPathChanged: track.path = load();
        onCenterChanged: map.center = pathController.center;
        onZoomChanged: map.zoomLevel = pathController.zoom;
    }
}
