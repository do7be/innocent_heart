Preload = ->
  @asset = null
  @ready = false

"use strict"
Preload:: =
  preload: ->
    @asset = @add.sprite(@width / 2, @height / 2, "preloader")
    @asset.anchor.setTo 0.5, 0.5
    @load.onLoadComplete.addOnce @onLoadComplete, this
    @load.setPreloadSprite @asset
    @load.image "yeoman", "assets/yeoman-logo.png"

  create: ->
    @asset.cropEnabled = false

  update: ->
    @game.state.start "menu"  unless not @ready

  onLoadComplete: ->
    @ready = true

module.exports = Preload
