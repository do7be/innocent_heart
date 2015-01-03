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
    @load.image "char1", "assets/char_1.png"
    @load.image "char2", "assets/char_2.png"
    @load.image "thunder2", "assets/thunder_2.png"

    @load.image   "stage1tile", "assets/tilemaps/tiles/stage1.png"
    @load.tilemap "stage1csv",  "assets/tilemaps/csv/stage1.csv", null, Phaser.Tilemap.TILED_CSV

  create: ->
    @asset.cropEnabled = false

  update: ->
    @game.state.start "menu"  unless not @ready

  onLoadComplete: ->
    @ready = true

module.exports = Preload
