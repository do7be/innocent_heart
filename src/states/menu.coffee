Menu = ->
"use strict"
Menu:: =
  preload: ->

  create: ->
    style =
      font: "65px Arial"
      fill: "#ffffff"
      align: "center"

    @sprite = @game.add.sprite(0, 0, "title")
#    @sprite.anchor.setTo 0, 0
    @titleText = @game.add.text(@game.world.centerX, 300, "'Allo, 'Allo!", style)
    @titleText.anchor.setTo 0.5, 0.5
    @instructionsText = @game.add.text(@game.world.centerX, 400, "Click anywhere to play \"Click The Yeoman Logo\"",
      font: "16px Arial"
      fill: "#ffffff"
      align: "center"
    )
    @instructionsText.anchor.setTo 0.5, 0.5
#    @sprite.angle = -20
#    @game.add.tween(@sprite).to
#      angle: 20
#    , 1000, Phaser.Easing.Linear.NONE, true, 0, 1000, true

  update: ->
    @stage = 1
    @game.state.start "prestage"  if @game.input.activePointer.justPressed()

module.exports = Menu
