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
    @load.image "title", "assets/scene/scene_1.png"
    @load.image "stage1", "assets/scene/scene_3.png"
    @load.image "char1", "assets/char_1.png"
    @load.image "char2", "assets/char_2.png"
    @load.image "thunder2", "assets/thunder_2.png"

  create: ->
    @asset.cropEnabled = false

  update: ->
    @game.state.start "menu"  unless not @ready

  onLoadComplete: ->
    @ready = true

module.exports = Preload
