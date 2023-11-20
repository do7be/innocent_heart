use try_rust_bevy::consts::*;
use try_rust_bevy::utils::*;

// WebGLだとその画像が初めて表示されるときに画像ロードが始まって表示が遅れるので最初から全て読み込んでおくためのScene
pub mod initial_load_scene {
    use bevy::prelude::*;

    use super::{despawn_screen, GameState};

    pub struct InitialLoadPlugin;

    impl Plugin for InitialLoadPlugin {
        fn build(&self, app: &mut App) {
            app.add_systems(OnEnter(GameState::InitialLoad), loading_setup)
                .add_systems(Update, countdown.run_if(in_state(GameState::InitialLoad)))
                .add_systems(
                    OnExit(GameState::InitialLoad),
                    despawn_screen::<OnInitialLoadScreen>,
                );
        }
    }

    #[derive(Component)]
    struct OnInitialLoadScreen;

    #[derive(Resource, Deref, DerefMut)]
    struct LoadingTimer(Timer);

    fn loading_setup(mut commands: Commands, asset_server: Res<AssetServer>) {
        // WebGL用のビルドでない場合は初期ロードがいらないので飛ばす
        if option_env!("WASM_BUILD").is_none() {
            commands.insert_resource(LoadingTimer(Timer::from_seconds(0., TimerMode::Once)));
            return;
        }

        commands.spawn((
            SpriteBundle {
                transform: Transform {
                    translation: Vec3::new(0., 0., 1.),
                    scale: Vec3::new(640., 480., 1.),
                    ..default()
                },
                sprite: Sprite {
                    color: Color::rgb(0.3, 0.3, 0.3),
                    ..default()
                },
                ..default()
            },
            OnInitialLoadScreen,
        ));

        // scenes
        for i in 1..=14 {
            commands.spawn((
                SpriteBundle {
                    texture: asset_server.load(format!("images/scene/scene_{}.png", i)),
                    sprite: Sprite::default(),
                    transform: Transform::from_translation(Vec3::new(0., 0., -10.)),
                    ..default()
                },
                OnInitialLoadScreen,
            ));
        }

        // maps
        for i in 1..=2 {
            for j in 1..=3 {
                commands.spawn((
                    SpriteBundle {
                        texture: asset_server.load(format!(
                            "images/map/map{}_{}.png",
                            if i == 2 {
                                i.to_string()
                            } else {
                                "".to_string()
                            },
                            j
                        )),
                        sprite: Sprite::default(),
                        transform: Transform::from_translation(Vec3::new(0., 0., -10.)),
                        ..default()
                    },
                    OnInitialLoadScreen,
                ));
            }
        }

        // status
        for i in 1..=3 {
            commands.spawn((
                SpriteBundle {
                    texture: asset_server.load(format!("images/status/item_{}.png", i)),
                    sprite: Sprite::default(),
                    transform: Transform::from_translation(Vec3::new(0., 0., -10.)),
                    ..default()
                },
                OnInitialLoadScreen,
            ));
        }
        for file in ["life", "number"] {
            commands.spawn((
                SpriteBundle {
                    texture: asset_server.load(format!("images/status/{}.png", file)),
                    sprite: Sprite::default(),
                    transform: Transform::from_translation(Vec3::new(0., 0., -10.)),
                    ..default()
                },
                OnInitialLoadScreen,
            ));
        }

        // characters
        for file in [
            "boss",
            "char",
            "mohican_lizard",
            "red_demon",
            "slime",
            "wizard",
        ] {
            commands.spawn((
                SpriteBundle {
                    texture: asset_server.load(format!("images/character/{}.png", file)),
                    sprite: Sprite::default(),
                    transform: Transform::from_translation(Vec3::new(0., 0., -10.)),
                    ..default()
                },
                OnInitialLoadScreen,
            ));
        }

        // effects
        for file in [
            "boss_attack_bluefire",
            "boss_attack_darkthunder",
            "boss_attack_meteor",
            "boss_attack_waterballoon",
            "enemy_attack_shockwave",
            "enemy_attack_wind",
            "fire",
            "ice",
            "sword",
            "thunder",
        ] {
            commands.spawn((
                SpriteBundle {
                    texture: asset_server.load(format!("images/effect/{}.png", file)),
                    sprite: Sprite::default(),
                    transform: Transform::from_translation(Vec3::new(0., 0., -10.)),
                    ..default()
                },
                OnInitialLoadScreen,
            ));
        }

        // TODO: とりあえず3秒とっているが、すべての画像が読み込まれたかどうか判定できるようにする
        commands.insert_resource(LoadingTimer(Timer::from_seconds(3., TimerMode::Once)));
    }

    fn countdown(
        mut game_state: ResMut<NextState<GameState>>,
        time: Res<Time>,
        mut timer: ResMut<LoadingTimer>,
    ) {
        if timer.tick(time.delta()).finished() {
            game_state.set(GameState::Title);
        }
    }
}
