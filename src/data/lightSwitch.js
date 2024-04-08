var Socket = new WebSocket("ws://" + window.location.hostname + ":81/");
Socket.onmessage = function(event) {
  processReceivedCommand(event);
}; //http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-9-websockets/

// When the connection is open, send some data to the server
Socket.onopen = function() {
 
  Socket.send("S");
};

function turnLight(lightNumber) {
  //reacting on WebSite button change
  console.log("Licht :" + lightNumber); //+","+myStatus);
  var send =
    "L" +
    lightNumber +
    "=" +
    (document.getElementById("Switch" + lightNumber).checked ? "1" : "0");
  //alert(send);
  Socket.send(send);
}

function processReceivedCommand(evt) {
  //receive from Controller instruction
  document.getElementById("rd").innerHTML = evt.data;
  console.log(evt.data);
  var myMessage = evt.data;
  if (myMessage.indexOf("S=") == 0) {
    document.getElementById("Switch1").checked =
      myMessage.charAt(2) == "0" ? false : true; //https://www.w3schools.com/js/js_string_methods.asp
    document.getElementById("Switch2").checked =
      myMessage.charAt(3) == "0" ? false : true;
    document.getElementById("Switch3").checked =
      myMessage.charAt(4) == "0" ? false : true;
    document.getElementById("Switch4").checked =
      myMessage.charAt(5) == "0" ? false : true;
  }
  /*
    if (evt.data ==='L1=1') 
    {  
        document.getElementById('BTN_LED').innerHTML = 'Turn on the LED';  
        document.getElementById('LED_status').innerHTML = 'LED is off';  
    //document.getElementById('Switch1').checked=true;
    }
    if (evt.data ==='L1=0') 
    {  
        document.getElementById('BTN_LED').innerHTML = 'Turn off the LED'; 
        document.getElementById('LED_status').innerHTML = 'LED is on';   
    //document.getElementById('Switch1').checked=false;
    }
    */
}

function requestSetting() {
  //request setting of lights
  //Socket.send("S");
}
