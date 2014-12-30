Prestage = ->
"use strict"
Prestage:: =
  preload: ->

  create: ->
    style =
      font: "11px Arial"
      fill: "#ffffff"
      align: "center"

    @sprite = @game.add.sprite(0, 0, "stage1")

  update: ->
    @game.state.start "play"  if @game.input.activePointer.justPressed()

module.exports = Prestage
