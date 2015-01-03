Play = ->
"use strict"
Play:: =
  TILE_SIZE: 32

  create: ->
    @game.physics.startSystem Phaser.Physics.ARCADE

    @layer = @createGround()
    @player = @createPlayer()

    @cursors = @game.input.keyboard.createCursorKeys()

    spaceKey = @game.input.keyboard.addKey(Phaser.Keyboard.SPACEBAR)
    spaceKey.onDown.add(@jump, @)

  createGround: ->
    map = @game.add.tilemap('stage1csv', @TILE_SIZE, @TILE_SIZE)
    map.addTilesetImage('stage1tile')

    map.setCollision(2)

    layer = map.createLayer(0)
    layer.resizeWorld()

    layer

  createPlayer: ->
    player = @game.add.sprite(0, @TILE_SIZE * 12, "char2")

    @game.physics.arcade.enable player, Phaser.Physics.ARCADE

    player.body.gravity.y = 1000

    @game.camera.follow(player)

    player

  jump: ->
    if @player.body.onFloor()
      @player.body.velocity.y = -400

  update: ->
    @game.physics.arcade.collide(@player, @layer)

    @player.body.velocity.x = 0

    switch
      when @cursors.left.isDown
        @player.body.velocity.x = -200
      when @cursors.right.isDown
        @player.body.velocity.x = 200

module.exports = Play
