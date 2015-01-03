# Generated on 2014-03-28 using generator-phaser-official 0.0.8-rc-2
"use strict"
config = require("./config.json")
_ = require("underscore")
_.str = require("underscore.string")

# Mix in non-conflict functions to Underscore namespace if you want
_.mixin _.str.exports()
LIVERELOAD_PORT = 35729
lrSnippet = require("connect-livereload")(port: LIVERELOAD_PORT)
mountFolder = (connect, dir) ->
  connect.static require("path").resolve(dir)

module.exports = (grunt) ->

  # load all grunt tasks
  require("matchdep").filterDev("grunt-*").forEach grunt.loadNpmTasks
  grunt.initConfig
    watch:
      scripts:
        files: [
          "src/**/*.coffee"
        ]
        options:
          spawn: false
          livereload: LIVERELOAD_PORT

        tasks: ["build"]

    connect:
      options:
        port: 9000

        # change this to '0.0.0.0' to access the server from outside
        hostname: "localhost"

      livereload:
        options:
          middleware: (connect) ->
            [
              lrSnippet
              mountFolder(connect, "dist")
            ]

    open:
      server:
        path: "http://localhost:9000"

    clean:
      js: 'game/**/*.js'

    coffee:
      compile:
        files: [
          expand: true
          cwd:  'src'
          src:  '**/*.coffee'
          dest: 'game'
          ext:  '.js'
        ]

    copy:
      dist:
        files: [
          {

            # includes files within path and its sub-directories
            expand: true
            src: ["assets/**"]
            dest: "dist/"
          }
          {
            expand: true
            flatten: true
            src: ["game/plugins/*.js"]
            dest: "dist/js/plugins/"
          }
          {
            expand: true
            flatten: true
            src: ["bower_components/**/build/*.js"]
            dest: "dist/js/"
          }
          {
            expand: true
            src: ["css/**"]
            dest: "dist/"
          }
          {
            expand: true
            src: ["index.html"]
            dest: "dist/"
          }
        ]

    browserify:
      build:
        src: ["game/main.js"]
        dest: "dist/js/game.js"

    'gh-pages':
      options:
        base: 'dist'
      src: ['**']

  grunt.registerTask "build", [
    "clean"
    "coffee"
    "buildBootstrapper"
    "browserify"
    "copy"
  ]
  grunt.registerTask "serve", [
    "build"
    "connect:livereload"
    "open"
    "watch"
  ]
  grunt.registerTask "default", ["serve"]
  grunt.registerTask "prod", [
    "build"
    "copy"
  ]
  grunt.registerTask "buildBootstrapper", "builds the bootstrapper file correctly", ->
    stateFiles = grunt.file.expand("game/states/*.js")
    gameStates = []
    statePattern = new RegExp(/(\w+).js$/)
    stateFiles.forEach (file) ->
      state = file.match(statePattern)[1]
      unless not state
        gameStates.push
          shortName: state
          stateName: _.capitalize(state) + "State"

    config.gameStates = gameStates
    console.log config
    bootstrapper = grunt.file.read("templates/_main.js.tpl")
    bootstrapper = grunt.template.process(bootstrapper,
      data: config
    )
    grunt.file.write "game/main.js", bootstrapper
