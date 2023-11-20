use try_rust_bevy::consts::*;
use try_rust_bevy::utils::*;

pub mod ending_scene {
    use bevy::prelude::*;

    use super::{despawn_screen, BossState, GameState, StageState};

    pub struct EndingPlugin;

    impl Plugin for EndingPlugin {
        fn build(&self, app: &mut App) {
            app.insert_resource(SceneNumber { number: 5 })
                .insert_resource(SleepTimer(Timer::from_seconds(1.0, TimerMode::Once)))
                .add_systems(OnEnter(GameState::Ending), ending_setup)
                .add_systems(
                    Update,
                    (show_scene_image, control_keys).run_if(in_state(GameState::Ending)),
                )
                .add_systems(OnExit(GameState::Ending), despawn_screen::<OnEndingScreen>);
        }
    }

    // シーン移動時にコンポーネントを消すためのタグとして使う
    #[derive(Component)]
    struct OnEndingScreen;

    #[derive(Component)]
    struct EndingImage;

    #[derive(Resource)]
    struct SceneNumber {
        number: u8,
    }

    #[derive(Resource, Deref, DerefMut)]
    struct SleepTimer(Timer);

    fn ending_setup(mut commands: Commands, asset_server: Res<AssetServer>) {
        commands.spawn((
            SpriteBundle {
                texture: asset_server.load("images/scene/scene_5.png"),
                sprite: Sprite::default(),
                ..default()
            },
            EndingImage,
            OnEndingScreen,
        ));
    }

    fn show_scene_image(
        scene_number: Res<SceneNumber>,
        mut query: Query<&mut Handle<Image>, With<EndingImage>>,
        asset_server: Res<AssetServer>,
    ) {
        let mut asset = query.single_mut();
        *asset = asset_server.load(format!("images/scene/scene_{}.png", scene_number.number));
    }

    fn control_keys(
        keyboard_input: Res<Input<KeyCode>>,
        gamepads: Res<Gamepads>,
        button_inputs: Res<Input<GamepadButton>>,
        mut scene_number: ResMut<SceneNumber>,
        time: Res<Time>,
        mut timer: ResMut<SleepTimer>,
        mut game_state: ResMut<NextState<GameState>>,
        mut stage_state: ResMut<NextState<StageState>>,
        mut boss_state: ResMut<NextState<BossState>>,
    ) {
        // タイマーを進める
        timer.tick(time.delta());

        let mut pressed = keyboard_input.just_pressed(KeyCode::Z);
        for gamepad in gamepads.iter() {
            if button_inputs.just_pressed(GamepadButton::new(gamepad, GamepadButtonType::South)) {
                pressed = true;
            }
        }

        // １秒スリープののち、Zを押したら次のscene imageへ
        if pressed && timer.finished() {
            if scene_number.number == 14 {
                // State全部リセットする
                game_state.set(GameState::Title);
                stage_state.set(StageState::default());
                boss_state.set(BossState::default());
                scene_number.number = 5;
            } else {
                scene_number.number += 1;
                timer.reset();
            }
        }
    }
}
