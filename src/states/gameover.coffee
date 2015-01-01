GameOver = ->
"use strict"
GameOver:: =
  preload: ->

  create: ->
    style =
      font: "65px Arial"
      fill: "#ffffff"
      align: "center"

    @titleText = @game.add.text(@game.world.centerX, 100, "Game Over!", style)
    @titleText.anchor.setTo 0.5, 0.5
    @congratsText = @game.add.text(@game.world.centerX, 200, "You Win!",
      font: "32px Arial"
      fill: "#ffffff"
      align: "center"
    )
    @congratsText.anchor.setTo 0.5, 0.5
    @instructionText = @game.add.text(@game.world.centerX, 300, "Click To Play Again",
      font: "16px Arial"
      fill: "#ffffff"
      align: "center"
    )
    @instructionText.anchor.setTo 0.5, 0.5

  update: ->
    @game.state.start "play"  if @game.input.activePointer.justPressed()

module.exports = GameOver
