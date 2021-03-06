//from https://beaufortfrancois.github.io/sandbox/web-bluetooth/generator/
const params = new URLSearchParams(new URL(window.location.href).search.slice(1));
const mydebug = !!Number(params.get("debug"));

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
    let elem = document.createElement(to_create);
    elem.innerHTML = str;
    return el.appendChild(elem);
}

let movement_list = {
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
    console.log(s);
    let encoder = new TextEncoder();
    let buffer = encoder.encode(s);
    catEars.device && catEars.writeInfo(buffer);
}

function send_info(e)
{
    let elem_a = e.target.querySelector("a") || e.target;
    let mvt = elem_a.getAttribute("href").substring(1);
    send_str(mvt);
}


let sensor;
let virtual_pos;
let real_pos;
let prev_pos;

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
//TODELETE.
//function clamp(min, val, max) {
//    return Math.max(min, Math.min(val, max));
//}
function initSensor() {
        const options = { frequency: 60 };
        sensor = new RelativeOrientationSensor(options);
        //sensor = new AbsoluteOrientationSensor(options);
        sensor.onreading = () => { 
            let debug_div = document.querySelector("#debug");
            let vect = sensor.quaternion;
            let q0 = vect[3];
            let q1 = vect[0];
            let q2 = vect[1];
            let q3 = vect[2];

            /* get roll, pitch, yaw */
            let sinr_cosp = 2 * (q0*q1 + q2*q3);
            let cosr_cosp = 1 - 2 * (q1*q1 + q2*q2);
            let roll = Math.atan2(sinr_cosp, cosr_cosp);

            let sinp = 2 * (q0*q2 - q3*q1);
            let pitch;
            if (sinp >=1)
                pitch = Math.pi / 2 ;
            else if (sinp <= -1)
                pitch = - Math.pi / 2;
            else
                pitch = Math.asin(sinp);

            let siny_cosp = 1 - 2 * (q0*q3 + q1*q2);
            let cosy_cosp = 1 - 2 * (q2*q2 + q3*q3);
            let yaw = Math.atan2(siny_cosp, cosy_cosp);

            virtual_pos.left.azi = Math.round(100 * Math.sin(Math.max(pitch - yaw/2, -Math.PI)));
            virtual_pos.right.azi = Math.round(100 * Math.sin(Math.min(pitch + yaw/2,  Math.PI)));
            virtual_pos.right.alt = Math.round(100 * Math.sin(roll));
            virtual_pos.left.alt = Math.round(100 * Math.sin(roll));

            debug_div.innerHTML = "roll=" + roll +
                                  " 100*sin(roll)=" + Math.round(100 * Math.sin(roll)) +
                                  "<br>pitch=" + pitch +
                                  " 100*sin(pitch)=" + Math.round(100*Math.sin(pitch)) +
                                  "<br>yaw=" + yaw +
                                  " 100*cos(yaw)=" + Math.round(100*Math.cos(yaw)) +
                                  "<br> 100*sin(pitch + yaw)=" + Math.round(100*Math.sin(pitch + yaw)) +
                                  "<br>manual:" + virtual_pos.left.azi + ":"
                                + virtual_pos.left.alt + ":"
                                + virtual_pos.right.azi + ":"
                                + virtual_pos.right.alt;
        };
        sensor.onerror = (event) => {
                if (event.error.name == 'NotReadableError') {
                        console.log("Sensor is not available.");
                }
        }
}

function initPosition() {

virtual_pos = {"left" : { "azi" : 0, "alt" : 0},
                  "right" : { "azi" : 0, "alt" : 0} };
real_pos =  {"left" : { "azi" : 0, "alt" : 0},
                  "right" : { "azi" : 0, "alt" : 0} };
prev_pos =  {"left" : { "azi" : 0, "alt" : 0},
                  "right" : { "azi" : 0, "alt" : 0} };
}
function cleanLastPosition() {
	/* delete previous saved position */
	definepoint("right", prev_pos.right.azi, prev_pos.right.alt, false);
	definepoint("left", prev_pos.left.azi, prev_pos.left.alt, false);
}


function definepoint(pos, azi, alt, set)
{
    let canvas = document.getElementById(pos+'-ear');
    if (!canvas) {
        console.log("no canvas");
        return;
    }
    let ctx = canvas.getContext('2d');
    if (!ctx) {
        console.log("no canvas");
        return;
    }

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

function genMoveList() {
    let movelist_div = document.querySelector("#movelist");
    for(let key in movement_list) {
        let elem = appendHtml(movelist_div, '<dt><a href="#'+ key +'" class="move">'+ movement_list[key] +'<\/a><\/dt>', "dl", {"class" : "dl-horizontal"});
        elem.addEventListener('click', event => {send_info(event);}, false);
    }
}
let buttonConnect = document.getElementById('buttonConnect');
if (!mydebug) {
    buttonConnect.addEventListener('click', event => {
        catEars.request()
        .then(_ => catEars.connect())
        .then(_ => { buttonConnect.style.display = "none" ;} )
        .then(_ => {initSelectors(); genMoveList(); })
        .catch(error => { console.log(error); });
    });
} else {
    buttonConnect.style.display = "none" ;
    genMoveList();
    initSelectors();
}
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
    if (event.changedTouches[0].target.id.includes("right")) {
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
    let offLeft = event.target.offsetLeft;
    let offHeight = event.target.offsetHeight;
    let offTop = event.target.offsetTop;
    let offWidth = event.target.offsetWidth;

    if (event.targetTouches[0].target.id.includes("right")) {
        virtual_pos.right.azi = -100 + Math.round(200*(event.targetTouches[0].pageX - offLeft)/(offWidth));
        virtual_pos.right.alt = -100 + Math.round(200*(event.targetTouches[0].pageY - offTop)/(offHeight));
    } else {
        virtual_pos.left.azi = -100 + Math.round(200*(event.targetTouches[0].pageX - offLeft)/(offWidth));
        virtual_pos.left.alt = -100 + Math.round(200*(event.targetTouches[0].pageY - offTop)/(offHeight));
    }
    return false;
}

let pos_inter_id;
function updateEarsPosition() {
    let accel = 5;
    console.log("ici");
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
    }else if (is_position_accelerometer()){
        real_pos.right.azi = virtual_pos.right.azi;
        real_pos.right.alt = virtual_pos.right.alt;
        real_pos.left.azi = virtual_pos.left.azi;
        real_pos.left.alt = virtual_pos.left.alt;
    console.log("la");
    } else {
        console.log("in interval loop but should'nt")
        return
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
}
var selected_mode="preprog";

function inputModeChange(event) {
    //console.log(event.target.value);
    let preprogElem = document.getElementById('movelist');
    let padElem = document.getElementById('manual-move');
    let interval = 1*1000;
    console.log(event.target.value);

    switch(selected_mode) {
    case "accelerometer":
        sensor.stop();
        clearInterval(pos_inter_id);
        padElem.style.display = "none" ;
        break;
    case "absolute":
    case "relative":
        padElem.style.display = "none" ;
        clearInterval(pos_inter_id);
        stopPadInteraction();
        break;
    case "preprog":
        preprogElem.style.display = "none" ;
        break;
    default:
        break;
    }

    selected_mode = event.target.value;

    switch(selected_mode) {
    case "accelerometer":
        cleanLastPosition();
        initPosition();
        padElem.style.display = "inline-block" ;
        pos_inter_id = setInterval(updateEarsPosition, interval);
        sensor.start();
        break;
    case "absolute":
    case "relative":
        padElem.style.display = "inline-block" ;
        startPadInteraction(event.target.value);
        pos_inter_id = setInterval(updateEarsPosition, interval);
        break;
    case "preprog":
        cleanLastPosition();
        initPosition();
        /* maybe should use display = "none" */
        preprogElem.style.display = "block" ;
    default:
        break;
    }
}


function startPadInteraction() {
    let touchZones = document.getElementsByClassName('touch-zone');
    for (let i=0; i<touchZones.length; i++) {
        touchZones[i].addEventListener('mousemove', manualmousemove, false);
        touchZones[i].addEventListener('mouseout', manualmouseout, false);
        touchZones[i].addEventListener('touchmove', manualtouchmove, false);
        touchZones[i].addEventListener('touchend', manualtouchend, false);
    }
}
function stopPadInteraction() {
    let touchZones = document.getElementsByClassName('touch-zone');
    for (let i=0; i<touchZones.length; i++) {
        touchZones[i].removeEventListener('mousemove', manualmousemove, false);
        touchZones[i].removeEventListener('mouseout', manualmouseout, false);
        touchZones[i].removeEventListener('touchmove', manualtouchmove, false);
        touchZones[i].removeEventListener('touchend', manualtouchend, false);
    }
}
function initSelectors() {
    let modeSelectors = document.querySelectorAll("input.mode");
    modeSelectors.forEach( function(elem){
        elem.addEventListener('change', inputModeChange);
    });
}


initPosition();
