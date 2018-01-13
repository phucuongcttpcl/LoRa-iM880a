var awsIOT = require("aws-iot-device-sdk");
serialport = require("serialport");

// Set up UART Communication
var serialPortName = '/dev/ttyS0'; //'/dev/ttyS0'
var readData = '';

var port = new serialport(serialPortName, {
    baudRate: 9600,
    dataBits: 8,
    parity: 'none',
    stopBits: 1,
    flowControl: false
});

const parser = new serialport.parsers.Readline('\n');
port.pipe(parser);

// Set up AWS IOT Device
var device = awsIOT.device({
    keyPath: './certs/2fb097f359-private.pem.key',
    certPath: './certs/2fb097f359-certificate.pem.crt',
    caPath: './certs/root-CA.crt',
    clientID: 'Rasp_Demo_Thing',
    region: 'us-east-2',
    host: 'a305v8ngoiy05f.iot.us-east-2.amazonaws.com',
    port: '8883'
});

var messID = 0;

// UART Communication events
port.on('open', onPortOpen);
parser.on('data', onPortData);
port.on('close', onPortClose);
port.on('error', onPortError);
port.write('Hi Raspberry Pi, I am PC');

function onPortOpen() {
    console.log("Port Open");
}

function onPortData(data) {
    console.log("Data receive on UART: " + data);
    var splitData = data.split(" - ");

    device.publish('LoRenMess', JSON.stringify({
        messID: messID++,
        packID: splitData[2],
        latitude: splitData[0],
        longitude: splitData[1],
        timeStamp: Date.now().toString()
    }));
}

function onPortClose() {
    console.log("Port Close");
}

function onPortError(error) {
    console.log(error);
}

// AWS IOT Device events
device.on('connect', onDeviceConnect);

device.on('message', onDeviceReceiveMess);

function onDeviceConnect(){
    console.log('Connected');
    device.subscribe('AWS_REP');
}

function onDeviceReceiveMess(topic, payload){
    console.log("Data received from topic: ", topic, payload.toString());
}