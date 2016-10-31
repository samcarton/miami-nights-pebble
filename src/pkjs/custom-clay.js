module.exports = function(minified) {
  var clayConfig = this;

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    clayConfig.getItemById('reset-colors-button').on('click', function() {
      clayConfig.getItemByMessageKey('HoursColor').set('00FFFF');
      clayConfig.getItemByMessageKey('MinutesColor').set('FF00FF');
      clayConfig.getItemByMessageKey('DayColor').set('00FF55');
      clayConfig.getItemByMessageKey('DateColor').set('00FF55');
      clayConfig.getItemByMessageKey('BackgroundColor').set('000000');
    });
  });
};