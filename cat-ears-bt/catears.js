//from https://beaufortfrancois.github.io/sandbox/web-bluetooth/generator/
class CatEars {
    constructor() {
        this.device = null;
        this.onDisconnected = this.onDisconnected.bind(this);
    }

    request() {
        let options = {
            "filters": [{
                "services": ["4fafc201-1fb5-459e-8fcc-c5c9c331914b"]
            }]
        };
        return navigator.bluetooth.requestDevice(options)
            .then(device => {
                this.device = device;
                this.device.addEventListener('gattserverdisconnected', this.onDisconnected);
            });
    }

    connect() {
        if (!this.device) {
            return Promise.reject('Device is not connected.');
        }
        return this.device.gatt.connect();
    }

    readInfo() {
        return this.device.gatt.getPrimaryService("4fafc201-1fb5-459e-8fcc-c5c9c331914b")
            .then(service => service.getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8"))
            .then(characteristic => characteristic.readValue());
    }

    writeInfo(data) {
        return this.device.gatt.getPrimaryService("4fafc201-1fb5-459e-8fcc-c5c9c331914b")
            .then(service => service.getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8"))
            .then(characteristic => characteristic.writeValue(data));
    }

    startInfoNotifications(listener) {
        return this.device.gatt.getPrimaryService("4fafc201-1fb5-459e-8fcc-c5c9c331914b")
            .then(service => service.getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8"))
            .then(characteristic => characteristic.startNotifications())
            .then(characteristic => characteristic.addEventListener('characteristicvaluechanged', listener));
    }

    stopInfoNotifications(listener) {
        return this.device.gatt.getPrimaryService("4fafc201-1fb5-459e-8fcc-c5c9c331914b")
            .then(service => service.getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8"))
            .then(characteristic => characteristic.stopNotifications())
            .then(characteristic => characteristic.removeEventListener('characteristicvaluechanged', listener));
    }

    disconnect() {
        if (!this.device) {
            return Promise.reject('Device is not connected.');
        }
        return this.device.gatt.disconnect();
    }

    onDisconnected() {
        console.log('Device is disconnected.');
    }
}

var catEars = new CatEars();

function appendHtml(el, str, to_create) {
    var elem = document.createElement(to_create);
    elem.innerHTML = str;
    return el.appendChild(elem);
}

var movement_list = {
    "triste" : "Triste",
    "penaud" : "Penaud",
    "gauche" : "Oreille Gauche",
    "droit" : "Oreille Droite",
    "aguet" : "Aux aguets",
    "content" : "Content",
    "ecoute" : "écoute",
    "surprise" : "Surprise",
    "baisse" : "Baissées",
    "tourne" : "Tournées",
    "reset" : "Reset Origine"
}
function send_str(s)
{
    let encoder = new TextEncoder();
    let buffer = encoder.encode(s);
    catEars.device && catEars.writeInfo(buffer);
}

function send_info(e)
{
    let mvt = e.target.attributes["href"].nodeValue.substring(1);
    send_str(mvt);
}


var sensor;
var virtual_pos;
var real_pos;
var prev_pos;

import {
        AbsoluteOrientationSensor,
        RelativeOrientationSensor
} from './motion-sensors.js';

if (navigator.permissions) {
        // https://w3c.github.io/orientation-sensor/#model
        Promise.all([navigator.permissions.query({ name: "accelerometer" }),
                        navigator.permissions.query({ name: "magnetometer" }),
                        navigator.permissions.query({ name: "gyroscope" })])
                .then(results => {
                        if (results.every(result => result.state === "granted")) {
                                initSensor();
                        } else {
                                console.log("Permission to use sensor was denied.");
                        }
                }).catch(err => {
                        console.log("Integration with Permissions API is not enabled, still try to start app.");
                        initSensor();
                });
} else {
        console.log("No Permissions API, still try to start app.");
        initSensor();
}
function initSensor() {
        const options = { frequency: 60 };
        sensor = new RelativeOrientationSensor(options);
        //sensor = new AbsoluteOrientationSensor(options);
        sensor.onreading = () => { 
                var debug_div = document.querySelector("#debug");
                let vect = sensor.quaternion;
                let demi_theta = Math.acos(vect[3]);
                let x = vect[0] / Math.sin(demi_theta);
                let y = vect[1] / Math.sin(demi_theta);
                let z = vect[2] / Math.sin(demi_theta);
                let mvt = "manual:" + y*100 + ":"
                            + x*100 + ":"
                            + y*100 + ":"
                            + y*100 ;
                send_str(mvt);

                debug_div.innerHTML = "x=" + x + "<br>y=" + y + "<br>z=" + z + "<br>theta" + 2*demi_theta + "<br>"+  sensor.quaternion;
                };
        sensor.onerror = (event) => {
                if (event.error.name == 'NotReadableError') {
                        console.log("Sensor is not available.");
                }
        }
}

function init_position() {
virtual_pos = {"left" : { "azi" : 0, "alt" : 0},
                  "right" : { "azi" : 0, "alt" : 0} };
real_pos =  {"left" : { "azi" : 0, "alt" : 0},
                  "right" : { "azi" : 0, "alt" : 0} };
prev_pos =  {"left" : { "azi" : 0, "alt" : 0},
                  "right" : { "azi" : 0, "alt" : 0} };
}

init_position();

function definepoint(pos, azi, alt, set)
{
    let canvas = document.getElementById(pos+'-ear');
    let ctx = canvas.getContext('2d');

    let x = (azi + 100)
    let y = (alt + 100)

    ctx.fillStyle = 'red';
    /* delete previous point */
    if (set)
        ctx.fillRect(x-2, y-2, 5, 5);
    else
        ctx.clearRect(x-2, y-2, 5, 5);

}
function is_position_absolute() {
    return document.querySelector('#absolute').checked ;
}
function is_position_relative() {
    return document.querySelector('#relative').checked ;
}
function is_position_accelerometer() {
    return document.querySelector('#accelerometer').checked ;
}
var buttonConnect = document.getElementById('buttonConnect');
buttonConnect.addEventListener('click', event => {
    catEars.request()
    .then(_ => catEars.connect())
    .then(_ => { buttonConnect.style.visibility = "hidden" ;} )
    .then(_ => {
        var movelist_div = document.querySelector("#movelist");
        for(var key in movement_list) {
            elem = appendHtml(movelist_div, '<dt><a href="#'+ key +'" class="move">'+ movement_list[key] +'<\/a><\/dt>', "dl", {"class" : "dl-horizontal"});
            elem.addEventListener('click', event => {init_position(); send_info(event);}, false);
        }
    })
    .catch(error => { console.log(error) });
});
function manualmouseout(event) {
    event.preventDefault();
    if (event.target.id.includes("right")) {
        virtual_pos.right.azi = 0;
        virtual_pos.right.alt = 0;
    } else {
        virtual_pos.left.azi = 0;
        virtual_pos.left.alt = 0;
    }
    return false;
}

function manualmousemove(event) {
    event.preventDefault();
    let offLeft = event.target.offsetLeft;
    let offHeight = event.target.offsetHeight;
    let offTop = event.target.offsetTop;
    let offWidth = event.target.offsetWidth;

    if (event.target.id.includes("right")) {
        virtual_pos.right.azi = -100 + Math.round(200*(event.pageX - offLeft)/(offWidth));
        virtual_pos.right.alt = -100 + Math.round(200*(event.pageY - offTop)/(offHeight));
    } else {
        virtual_pos.left.azi = -100 + Math.round(200*(event.pageX - offLeft)/(offWidth));
        virtual_pos.left.alt = -100 + Math.round(200*(event.pageY - offTop)/(offHeight));
    }
    return false;
}
function manualtouchend(event) {
    event.preventDefault();
    if (event.targetTouches[0].target.id.includes("right")) {
        virtual_pos.right.azi = 0;
        virtual_pos.right.alt = 0;
    } else {
        virtual_pos.left.azi = 0;
        virtual_pos.left.alt = 0;
    }
    return false;
}
function manualtouchmove(event) {
    event.preventDefault();
    offLeft = event.target.offsetLeft;
    offHeight = event.target.offsetHeight;
    offTop = event.target.offsetTop;
    offWidth = event.target.offsetWidth;

    if (event.targetTouches[0].target.id.includes("right")) {
        virtual_pos.right.azi = -100 + Math.round(200*(event.targetTouches[0].pageX - offLeft)/(offWidth));
        virtual_pos.right.alt = -100 + Math.round(200*(event.targetTouches[0].pageY - offTop)/(offHeight));
    } else {
        virtual_pos.left.azi = -100 + Math.round(200*(event.targetTouches[0].pageX - offLeft)/(offWidth));
        virtual_pos.left.alt = -100 + Math.round(200*(event.targetTouches[0].pageY - offTop)/(offHeight));
    }
    return false;
}

function inputModeChange(event) {
    console.log(event.target.value);
}


function initInteractive() {
    let touchZones = document.getElementsByClassName('touch-zone');
    for (var i=0; i<touchZones.length; i++) {
        touchZones[i].addEventListener('mousemove', manualmousemove, false);
        touchZones[i].addEventListener('mouseout', manualmouseout, false);
        touchZones[i].addEventListener('touchmove', manualtouchmove, false);
        touchZones[i].addEventListener('touchend', manualtouchend, false);
    }
    /* better
    touchZones.forEach( (elem) => {
        elem.addEventListener('mousemove', manualmousemove, false);
        elem.addEventListener('mouseout', manualmouseout, false);
        elem.addEventListener('touchmove', manualtouchmove, false);
        elem.addEventListener('touchend', manualtouchend, false);

     };*/


    let modeSelectors = document.querySelectorAll("input.mode");
    modeSelectors.forEach( function(elem){
        elem.addEventListener('change', inputModeChange);
    });

    let pos_inter_id = setInterval( _ => {
        let accel = 5;

        if (is_position_absolute()) {
            real_pos.right.azi = virtual_pos.right.azi;
            real_pos.right.alt = virtual_pos.right.alt;
            real_pos.left.azi = virtual_pos.left.azi;
            real_pos.left.alt = virtual_pos.left.alt;
        } else if (is_position_relative()){
            real_pos.right.azi += Math.round(virtual_pos.right.azi/accel);
            if (real_pos.right.azi > 100)
                real_pos.right.azi = 100;
            if (real_pos.right.azi < -100)
                real_pos.right.azi = -100;
            real_pos.right.alt += Math.round(virtual_pos.right.alt/accel);
            if (real_pos.right.alt > 100)
                real_pos.right.alt = 100;
            if (real_pos.right.alt < -100)
                real_pos.right.alt = -100;
            real_pos.left.azi += Math.round(virtual_pos.left.azi/accel);
            if (real_pos.left.azi > 100)
                real_pos.left.azi = 100;
            if (real_pos.left.azi < -100)
                real_pos.left.azi = -100;
            real_pos.left.alt += Math.round(virtual_pos.left.alt/accel);
            if (real_pos.left.alt > 100)
                real_pos.left.alt = 100;
            if (real_pos.left.alt < -100)
                real_pos.left.alt = -100;
        } else {
            /* Nothing */
        }


        if ((prev_pos.right.azi != real_pos.right.azi) ||
            (prev_pos.right.alt != real_pos.right.alt) ||
            (prev_pos.left.azi != real_pos.left.azi) ||
            (prev_pos.left.alt != real_pos.left.alt)) {

            definepoint("right", prev_pos.right.azi, prev_pos.right.alt, false);
            definepoint("left", prev_pos.left.azi, prev_pos.left.alt, false);

            prev_pos.right.azi = real_pos.right.azi;
            prev_pos.right.alt = real_pos.right.alt;
            prev_pos.left.azi = real_pos.left.azi;
            prev_pos.left.alt = real_pos.left.alt;

            let mvt = "manual:" + real_pos.left.azi + ":"
                                + real_pos.left.alt + ":"
                                + real_pos.right.azi + ":"
                                + real_pos.right.alt ;
            send_str(mvt);
            //console.log(mvt);

            definepoint("right", real_pos.right.azi, real_pos.right.alt, true);
            definepoint("left", real_pos.left.azi, real_pos.left.alt, true);

        }
    }, 1*1000);

}

initInteractive() {
    let touchZones = document.getElementsByClassName('touch-zone');
    for (var i=0; i<touchZones.length; i++) {
        touchZones[i].addEventListener('mousemove', manualmousemove, false);
        touchZones[i].addEventListener('mouseout', manualmouseout, false);
        touchZones[i].addEventListener('touchmove', manualtouchmove, false);
        touchZones[i].addEventListener('touchend', manualtouchend, false);
    }
    /* better
    touchZones.forEach( (elem) => {
        elem.addEventListener('mousemove', manualmousemove, false);
        elem.addEventListener('mouseout', manualmouseout, false);
        elem.addEventListener('touchmove', manualtouchmove, false);
        elem.addEventListener('touchend', manualtouchend, false);

     };*/


    let modeSelectors = document.querySelectorAll("input.mode");
    modeSelectors.forEach( function(elem){
        elem.addEventListener('change', inputModeChange);
    });

    var pos_inter_id = setInterval( _ => {
        let accel = 5;

        if (is_position_absolute()) {
            real_pos.right.azi = virtual_pos.right.azi;
            real_pos.right.alt = virtual_pos.right.alt;
            real_pos.left.azi = virtual_pos.left.azi;
            real_pos.left.alt = virtual_pos.left.alt;
        } else if (is_position_relative()){
            real_pos.right.azi += Math.round(virtual_pos.right.azi/accel);
            if (real_pos.right.azi > 100)
                real_pos.right.azi = 100;
            if (real_pos.right.azi < -100)
                real_pos.right.azi = -100;
            real_pos.right.alt += Math.round(virtual_pos.right.alt/accel);
            if (real_pos.right.alt > 100)
                real_pos.right.alt = 100;
            if (real_pos.right.alt < -100)
                real_pos.right.alt = -100;
            real_pos.left.azi += Math.round(virtual_pos.left.azi/accel);
            if (real_pos.left.azi > 100)
                real_pos.left.azi = 100;
            if (real_pos.left.azi < -100)
                real_pos.left.azi = -100;
            real_pos.left.alt += Math.round(virtual_pos.left.alt/accel);
            if (real_pos.left.alt > 100)
                real_pos.left.alt = 100;
            if (real_pos.left.alt < -100)
                real_pos.left.alt = -100;
        } else {
            /* Nothing */
        }


        if ((prev_pos.right.azi != real_pos.right.azi) ||
            (prev_pos.right.alt != real_pos.right.alt) ||
            (prev_pos.left.azi != real_pos.left.azi) ||
            (prev_pos.left.alt != real_pos.left.alt)) {

            definepoint("right", prev_pos.right.azi, prev_pos.right.alt, false);
            definepoint("left", prev_pos.left.azi, prev_pos.left.alt, false);

            prev_pos.right.azi = real_pos.right.azi;
            prev_pos.right.alt = real_pos.right.alt;
            prev_pos.left.azi = real_pos.left.azi;
            prev_pos.left.alt = real_pos.left.alt;

            let mvt = "manual:" + real_pos.left.azi + ":"
                                + real_pos.left.alt + ":"
                                + real_pos.right.azi + ":"
                                + real_pos.right.alt ;
            send_str(mvt);
            //console.log(mvt);

            definepoint("right", real_pos.right.azi, real_pos.right.alt, true);
            definepoint("left", real_pos.left.azi, real_pos.left.alt, true);

        }
    }, 1*1000);

}
initInteractive();
