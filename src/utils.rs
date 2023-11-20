use bevy::prelude::{
    Camera2d, Commands, Component, DespawnRecursiveExt, Entity, Query, Transform, With,
};

pub fn despawn_screen<T: Component>(
    to_despawn: Query<Entity, With<T>>,
    mut commands: Commands,
    mut camera_query: Query<&mut Transform, With<Camera2d>>,
) {
    for entity in &to_despawn {
        commands.entity(entity).despawn_recursive();
    }

    // カメラ位置をリセット（NOW LOADINGなどを中心に表示するため）
    let mut camera = camera_query.single_mut();
    camera.translation.x = 0.;
    camera.translation.y = 0.;
}
