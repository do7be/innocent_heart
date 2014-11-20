(function() {
  var Boot;

  Boot = function() {};

  "use strict";

  Boot.prototype = {
    preload: function() {
      return this.load.image("preloader", "assets/preloader.gif");
    },
    create: function() {
      this.game.input.maxPointers = 1;
      return this.game.state.start("preload");
    }
  };

  module.exports = Boot;

}).call(this);
