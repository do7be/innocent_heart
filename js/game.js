(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);throw new Error("Cannot find module '"+o+"'")}var f=n[o]={exports:{}};t[o][0].call(f.exports,function(e){var n=t[o][1][e];return s(n?n:e)},f,f.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
'use strict';

//global variables
window.onload = function () {
  var game = new Phaser.Game(800, 600, Phaser.AUTO, 'innocent-heart');

  // Game States
  game.state.add('boot', require('./states/boot'));
  game.state.add('gameover', require('./states/gameover'));
  game.state.add('menu', require('./states/menu'));
  game.state.add('play', require('./states/play'));
  game.state.add('preload', require('./states/preload'));
  

  game.state.start('boot');
};
},{"./states/boot":2,"./states/gameover":3,"./states/menu":4,"./states/play":5,"./states/preload":6}],2:[function(require,module,exports){
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

},{}],3:[function(require,module,exports){
(function() {
  var GameOver;

  GameOver = function() {};

  "use strict";

  GameOver.prototype = {
    preload: function() {},
    create: function() {
      var style;
      style = {
        font: "65px Arial",
        fill: "#ffffff",
        align: "center"
      };
      this.titleText = this.game.add.text(this.game.world.centerX, 100, "Game Over!", style);
      this.titleText.anchor.setTo(0.5, 0.5);
      this.congratsText = this.game.add.text(this.game.world.centerX, 200, "You Win!", {
        font: "32px Arial",
        fill: "#ffffff",
        align: "center"
      });
      this.congratsText.anchor.setTo(0.5, 0.5);
      this.instructionText = this.game.add.text(this.game.world.centerX, 300, "Click To Play Again", {
        font: "16px Arial",
        fill: "#ffffff",
        align: "center"
      });
      return this.instructionText.anchor.setTo(0.5, 0.5);
    },
    update: function() {
      if (this.game.input.activePointer.justPressed()) {
        return this.game.state.start("play");
      }
    }
  };

  module.exports = GameOver;

}).call(this);

},{}],4:[function(require,module,exports){
(function() {
  var Menu;

  Menu = function() {};

  "use strict";

  Menu.prototype = {
    preload: function() {},
    create: function() {
      var style;
      style = {
        font: "65px Arial",
        fill: "#ffffff",
        align: "center"
      };
      this.sprite = this.game.add.sprite(this.game.world.centerX, 138, "yeoman");
      this.sprite.anchor.setTo(0.5, 0.5);
      this.titleText = this.game.add.text(this.game.world.centerX, 300, "'Allo, 'Allo!", style);
      this.titleText.anchor.setTo(0.5, 0.5);
      this.instructionsText = this.game.add.text(this.game.world.centerX, 400, "Click anywhere to play \"Click The Yeoman Logo\"", {
        font: "16px Arial",
        fill: "#ffffff",
        align: "center"
      });
      this.instructionsText.anchor.setTo(0.5, 0.5);
      this.sprite.angle = -20;
      return this.game.add.tween(this.sprite).to({
        angle: 20
      }, 1000, Phaser.Easing.Linear.NONE, true, 0, 1000, true);
    },
    update: function() {
      if (this.game.input.activePointer.justPressed()) {
        return this.game.state.start("play");
      }
    }
  };

  module.exports = Menu;

}).call(this);

},{}],5:[function(require,module,exports){
(function() {
  var Play;

  Play = function() {};

  "use strict";

  Play.prototype = {
    TILE_SIZE: 32,
    create: function() {
      var spaceKey;
      this.game.physics.startSystem(Phaser.Physics.ARCADE);
      this.layer = this.createGround();
      this.player = this.createPlayer();
      this.cursors = this.game.input.keyboard.createCursorKeys();
      spaceKey = this.game.input.keyboard.addKey(Phaser.Keyboard.SPACEBAR);
      return spaceKey.onDown.add(this.jump, this);
    },
    createGround: function() {
      var layer, map;
      map = this.game.add.tilemap('stage1csv', this.TILE_SIZE, this.TILE_SIZE);
      map.addTilesetImage('stage1tile');
      map.setCollision(2);
      layer = map.createLayer(0);
      layer.resizeWorld();
      return layer;
    },
    createPlayer: function() {
      var player;
      player = this.game.add.sprite(0, this.TILE_SIZE * 12, "char2");
      this.game.physics.arcade.enable(player, Phaser.Physics.ARCADE);
      player.body.gravity.y = 1000;
      this.game.camera.follow(player);
      return player;
    },
    jump: function() {
      if (this.player.body.onFloor()) {
        return this.player.body.velocity.y = -400;
      }
    },
    update: function() {
      this.game.physics.arcade.collide(this.player, this.layer);
      this.player.body.velocity.x = 0;
      switch (false) {
        case !this.cursors.left.isDown:
          return this.player.body.velocity.x = -200;
        case !this.cursors.right.isDown:
          return this.player.body.velocity.x = 200;
      }
    }
  };

  module.exports = Play;

}).call(this);

},{}],6:[function(require,module,exports){
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
      this.load.image("yeoman", "assets/yeoman-logo.png");
      this.load.image("char1", "assets/char_1.png");
      this.load.image("char2", "assets/char_2.png");
      this.load.image("thunder2", "assets/thunder_2.png");
      this.load.image("stage1tile", "assets/tilemaps/tiles/stage1.png");
      return this.load.tilemap("stage1csv", "assets/tilemaps/csv/stage1.csv", null, Phaser.Tilemap.TILED_CSV);
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

},{}]},{},[1])