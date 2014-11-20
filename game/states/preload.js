(function() {
  var Preload;

  Preload = function() {
    this.asset = null;
    return this.ready = false;
  };

  "use strict";

  Preload.prototype = {
    preload: function() {
      this.asset = this.add.sprite(this.width / 2, this.height / 2, "preloader");
      this.asset.anchor.setTo(0.5, 0.5);
      this.load.onLoadComplete.addOnce(this.onLoadComplete, this);
      this.load.setPreloadSprite(this.asset);
      return this.load.image("yeoman", "assets/yeoman-logo.png");
    },
    create: function() {
      return this.asset.cropEnabled = false;
    },
    update: function() {
      if (!!this.ready) {
        return this.game.state.start("menu");
      }
    },
    onLoadComplete: function() {
      return this.ready = true;
    }
  };

  module.exports = Preload;

}).call(this);
