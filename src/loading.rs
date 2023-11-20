use try_rust_bevy::consts::*;
use try_rust_bevy::utils::*;

pub mod loading_scene {
    use bevy::prelude::*;

    use super::{despawn_screen, GameState};

    pub struct LoadingPlugin;

    impl Plugin for LoadingPlugin {
        fn build(&self, app: &mut App) {
            app.add_systems(OnEnter(GameState::Loading), loading_setup)
                .add_systems(Update, countdown.run_if(in_state(GameState::Loading)))
                .add_systems(
                    OnExit(GameState::Loading),
                    despawn_screen::<OnLoadingScreen>,
                );
        }
    }

    #[derive(Component)]
    struct OnLoadingScreen;

    #[derive(Resource, Deref, DerefMut)]
    struct LoadingTimer(Timer);

    fn loading_setup(mut commands: Commands, asset_server: Res<AssetServer>) {
        commands.spawn((
            SpriteBundle {
                texture: asset_server.load("images/scene/scene_2.png"),
                sprite: Sprite::default(),
                ..default()
            },
            OnLoadingScreen,
        ));

        commands.insert_resource(LoadingTimer(Timer::from_seconds(0.5, TimerMode::Once)));
    }

    fn countdown(
        mut game_state: ResMut<NextState<GameState>>,
        time: Res<Time>,
        mut timer: ResMut<LoadingTimer>,
    ) {
        if timer.tick(time.delta()).finished() {
            game_state.set(GameState::StageTitle);
        }
    }
}
