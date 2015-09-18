Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://samcarton.github.io/miami-nights-pebble/index.html');
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
  
  

  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
});