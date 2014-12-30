GameOver = ->
"use strict"
GameOver:: =
  preload: ->

  create: ->
    @game.state.start "prestage"

module.exports = GameOver
