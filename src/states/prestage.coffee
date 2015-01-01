Prestage = ->
"use strict"
Prestage:: =
  preload: ->

  create: ->
    style =
      font: "11px Arial"
      fill: "#ffffff"
      align: "center"

    switch @game.var.stage
      when 1
        @sprite = @game.add.sprite(0, 0, "stage1")
      when 2
        @sprite = @game.add.sprite(0, 0, "stage2")
      else
        @errorText = @game.add.text(300, 300, "stage is invalid num", style)

    @stageText = @game.add.text(200, 300, "stage = " + @game.var.stage, style)

  update: ->
    @game.state.start "play"  if @game.input.activePointer.justPressed()

module.exports = Prestage
