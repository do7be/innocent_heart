Play = ->
"use strict"
Play:: =
  create: ->
    @game.physics.startSystem Phaser.Physics.ARCADE
    @bird = @game.add.sprite(100, 245, "char2")

    @game.physics.arcade.enable @bird

    @bird.body.gravity.y = 1000

    spaceKey = @game.input.keyboard.addKey(Phaser.Keyboard.SPACEBAR)
    spaceKey.onDown.add(@jump, @)

    @pipes = @game.add.group()
    @pipes.enableBody = true
    @pipes.createMultiple(20, 'thunder2')

    @timer = @game.time.events.loop(1500, @addRowOfPipes, @)

    @score = 0
    @labelScore = @game.add.text(20, 20, "0", font: "30px Arial", fill: "#ffffff")

  addOnePipe: (x, y) ->
    pipe = @pipes.getFirstDead()
    pipe.reset(x, y)
    pipe.body.velocity.x = -200
    pipe.checkWorldBounds = true
    pipe.outOfBoundsKill = true

  addRowOfPipes: ->
    hole = Math.floor(Math.random() * 5) + 1

    for i in [1..6]
      if i != hole && i != hole + 1
        @addOnePipe(400, i * 60 + 10)

    @score += 1
    @labelScore.text = @score

  update: ->
    if @bird.inWorld == false
      @restartGame()

    @game.physics.arcade.overlap(@bird, @pipes, @restartGame, null, @)

  jump: ->
    @bird.body.velocity.y = -350

  restartGame: ->
    @game.state.start('gameover')

module.exports = Play
