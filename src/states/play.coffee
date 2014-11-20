Play = ->
"use strict"
Play:: =
  create: ->
    @game.physics.startSystem Phaser.Physics.ARCADE
    @sprite = @game.add.sprite(@game.width / 2, @game.height / 2, "yeoman")
    @sprite.inputEnabled = true
    @game.physics.arcade.enable @sprite
    @sprite.body.collideWorldBounds = true
    @sprite.body.bounce.setTo 1, 1
    @sprite.body.velocity.x = @game.rnd.integerInRange(-500, 500)
    @sprite.body.velocity.y = @game.rnd.integerInRange(-500, 500)
    @sprite.events.onInputDown.add @clickListener, this

  update: ->

  clickListener: ->
    @game.state.start "gameover"

module.exports = Play
