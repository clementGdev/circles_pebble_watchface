Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
  }
);


Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("https://dl.dropbox.com/s/5rnu7h3u7slijgx/Circles_config.html?dl=0");//"https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/sdktut9-config.html");
  }
);


Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    //console.log("Configuration window returned");
	//console.log("response" + e.response);
    var configuration = JSON.parse(decodeURIComponent(e.response));
    //console.log("Configuration window returned: " + JSON.stringify(configuration));
 
    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      {"KEY_BACKGROUND_COLOR": configuration.background_color, "KEY_CIRCLES_COLOR": configuration.circles_color},
      function(e) {
        console.log("Sending settings data...");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);