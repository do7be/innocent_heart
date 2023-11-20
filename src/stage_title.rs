use try_rust_bevy::consts::*;
use try_rust_bevy::utils::*;

pub mod stage_title_scene {
    use bevy::prelude::*;
    use try_rust_bevy::consts::BossState;

    use super::{despawn_screen, GameState, StageState};

    pub struct StageTitlePlugin;

    impl Plugin for StageTitlePlugin {
        fn build(&self, app: &mut App) {
            app.add_systems(OnEnter(GameState::StageTitle), stage_title_setup)
                .add_systems(Update, control_keys.run_if(in_state(GameState::StageTitle)))
                .add_systems(
                    OnExit(GameState::StageTitle),
                    despawn_screen::<OnStageTitleScreen>,
                );
        }
    }

    // シーン移動時にコンポーネントを消すためのタグとして使う
    #[derive(Component)]
    struct OnStageTitleScreen;

    fn stage_title_setup(
        mut commands: Commands,
        asset_server: Res<AssetServer>,
        stage_state: Res<State<StageState>>,
        mut boss_state: ResMut<NextState<BossState>>,
    ) {
        commands.spawn((
            SpriteBundle {
                texture: asset_server.load(match stage_state.get() {
                    StageState::Stage1 => "images/scene/scene_3.png",
                    StageState::Stage2 | StageState::Boss => "images/scene/scene_4.png",
                }),
                sprite: Sprite::default(),
                ..default()
            },
            OnStageTitleScreen,
        ));

        // ボスの出現状態は毎回リセットしておく
        boss_state.set(BossState::InActive);
    }

    fn control_keys(
        mut game_state: ResMut<NextState<GameState>>,
        keyboard_input: Res<Input<KeyCode>>,
        gamepads: Res<Gamepads>,
        button_inputs: Res<Input<GamepadButton>>,
    ) {
        let mut pressed = keyboard_input.just_pressed(KeyCode::Z);
        for gamepad in gamepads.iter() {
            if button_inputs.just_pressed(GamepadButton::new(gamepad, GamepadButtonType::South)) {
                pressed = true;
            }
        }
        if pressed {
            game_state.set(GameState::Game);
        }
    }
}
