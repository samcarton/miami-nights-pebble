// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

/*

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://samcarton.github.io/miami-nights-pebble/config.v1.3.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ' + JSON.stringify(config_data));

  // Prepare AppMessage payload
  var dict = {};
  if(config_data['bt_toggle'] == true){
    dict['KEY_BT_TOGGLE'] = config_data['bt_toggle'];
  }
  
  dict['KEY_DATE_FORMAT'] = config_data['date_format'];
  
  if(config_data.day_toggle === true){
    dict.KEY_DAY_TOGGLE = config_data.day_toggle;
  }
  
  // leading zero toggle
  if(config_data['lz_toggle'] == true){
    dict['KEY_LZ_TOGGLE'] = config_data['lz_toggle'];
  }
  
  // left align toggle
  if(config_data['la_toggle'] == true){
    dict['KEY_LA_TOGGLE'] = config_data['la_toggle'];
  }

  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
});
*/ 