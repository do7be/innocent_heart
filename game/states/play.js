(function() {
  var Play;

  Play = function() {};

  "use strict";

  Play.prototype = {
    create: function() {
      this.game.physics.startSystem(Phaser.Physics.ARCADE);
      this.sprite = this.game.add.sprite(this.game.width / 2, this.game.height / 2, "yeoman");
      this.sprite.inputEnabled = true;
      this.game.physics.arcade.enable(this.sprite);
      this.sprite.body.collideWorldBounds = true;
      this.sprite.body.bounce.setTo(1, 1);
      this.sprite.body.velocity.x = this.game.rnd.integerInRange(-500, 500);
      this.sprite.body.velocity.y = this.game.rnd.integerInRange(-500, 500);
      return this.sprite.events.onInputDown.add(this.clickListener, this);
    },
    update: function() {},
    clickListener: function() {
      return this.game.state.start("gameover");
    }
  };

  module.exports = Play;

}).call(this);
