var Socket = new WebSocket("ws://" + window.location.hostname + ":81/");
Socket.onmessage = function(event) {
  processReceivedCommand(event);
}; //http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-9-websockets/

// When the connection is open, send some data to the server
Socket.onopen = function() {
 
  Socket.send("I");
};

function processReceivedCommand(evt) {
  //receive from Controller instruction
  document.getElementById("infoData").innerHTML = evt.data;
  console.log(evt.data);

  }