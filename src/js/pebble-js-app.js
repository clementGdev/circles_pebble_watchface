Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
	if (Pebble.getActiveWatchInfo()) {
		console.log("watch platform " + Pebble.getActiveWatchInfo().platform);
	}
	else console.log("watch platform failed");
	
  }
);


Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
	if (Pebble.getActiveWatchInfo().platform == "basalt") {
		Pebble.openURL("https://dl.dropbox.com/s/5rnu7h3u7slijgx/Circles_config.html?dl=0");
	}
    else Pebble.openURL("https://dl.dropbox.com/s/t8fowhk1n1zfx31/Circles_config_b_w.html?dl=0");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {  
    //Get JSON dictionary
    //console.log("Configuration window returned");
	console.log("response" + e.response);
    var configuration = JSON.parse(decodeURIComponent(e.response));
    //console.log("Configuration window returned: " + JSON.stringify(configuration));

    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      {"KEY_BACKGROUND_COLOR": configuration.background_color, "KEY_CIRCLES_COLOR": configuration.circle_color},
      function(e) {
        console.log("Sending settings data...");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);