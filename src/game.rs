pub mod game_scene {
    use std::time::Duration;

    use bevy::prelude::*;
    use bevy::sprite::collide_aabb::{collide, Collision};
    use rand::Rng;
    use try_rust_bevy::consts::*;
    use try_rust_bevy::utils::*;

    const FPS: usize = 60;
    const TIME_1F: f32 = 1. / FPS as f32;
    const CHARACTER_SIZE: f32 = 32.;
    const BOSS_SIZE: f32 = 64.;
    const TILE_SIZE: f32 = 32.;
    const LIFE_SIZE: f32 = 16.;
    const PLAYER_JUMP_FORCE: f32 = 44.;
    const PLAYER_WALK_STEP: f32 = 4.;
    const PLAYER_WEAPON_STEP: f32 = 8.;
    const PLAYER_WEAPON_THUNDER_STEP: f32 = 12.;
    const PLAYER_WEAPON_LIFETIME_FOR_SWORD: f32 = 17. * TIME_1F;
    const PLAYER_WEAPON_LIFETIME_FOR_FIRE_ICE: f32 = 30. * TIME_1F;
    const PLAYER_WEAPON_LIFETIME_FOR_THUNDER: f32 = 45. * TIME_1F;
    const ENEMY_WEAPON_LIFETIME: f32 = 60. * TIME_1F;
    const ENEMY_WALK_STEP: f32 = 1.;
    const ENEMY_RIZZARD_WALK_STEP: f32 = 4.;
    const BOSS_WEAPON_STEP: f32 = 4.;
    const BOSS_WEAPON_LIFETIME: f32 = 90. * TIME_1F;
    const BOSS_DAMAGE_COOLTIME: f32 = 30. * TIME_1F;
    const BOSS_WALK_STEP: f32 = 2.;
    const BOSS_MOVE_LIFETIME: usize = 40;
    const GRAVITY: f32 = 9.81;
    const GRAVITY_TIME_STEP: f32 = 0.24; // FPS通りだと重力加速が少ないので経過時間を補正
    const MAP_WIDTH_TILES: u32 = 100;
    const MAP_HEIGHT_TILES: u32 = 15;

    #[derive(Component)]
    struct OnGameScreen;

    #[derive(Component, Deref, DerefMut)]
    struct Velocity(Vec2);

    #[derive(Component)]
    struct Character;

    #[derive(Component)]
    struct Collider;

    #[derive(Component)]
    struct Wall;

    #[derive(Component)]
    struct AnimationIndices {
        first: usize,
        last: usize,
    }

    #[derive(Component, Deref, DerefMut)]
    struct AnimationTimer(Timer);

    #[derive(Event, Default)]
    struct CollisionEvent;

    #[derive(Component)]
    struct Boss {
        damage_cooldown: Timer,
        life: i32,
    }

    #[derive(Component)]
    struct BossLife {
        index: u8,
    }

    #[derive(Clone, PartialEq)]
    enum BossWeaponKind {
        BlueFire,
        DarkThunder,
        WaterBalloon,
        Meteor,
    }

    #[derive(Component)]
    struct BossWeapon {
        kind: BossWeaponKind,
        dark_thunder_timer: Timer,
        index: usize,
    }

    #[derive(PartialEq)]
    enum EnemyKind {
        Slime,
        Lizard,
        Wizard,
        RedDemon,
    }

    #[derive(Component)]
    struct Enemy {
        kind: EnemyKind,
    }

    #[derive(Component)]
    struct EnemyCharacter {
        direction: AllDirection,
        move_lifetime: usize,
        stop: bool,
        weapon_cooldown: Timer,
        walk_step: f32,
    }

    #[derive(Clone, PartialEq)]
    enum EnemyWeaponKind {
        Wind,
        ShockWave,
    }

    #[derive(Component)]
    struct EnemyWeapon {
        lifetime: Timer,
        step: Vec2,
    }

    #[derive(Component)]
    struct Player {
        direction: Direction,
        walk: bool,
        grounded: bool,
        live: bool,
        jump_status: PlayerJumpStatus,
        weapon_limit: PlayerWeaponLimit,
    }

    #[derive(Debug)]
    struct PlayerWeaponLimit {
        fire: u8,
        ice: u8,
        thunder: u8,
    }

    #[derive(Resource, Deref, DerefMut)]
    struct ThunderStopTimer(Timer);

    #[derive(Clone, PartialEq)]
    enum PlayerWeaponKind {
        Sword,
        Fire,
        Ice,
        Thunder,
    }

    #[derive(Component)]
    struct PlayerWeapon {
        kind: PlayerWeaponKind,
        lifetime: Timer,
    }

    #[derive(Component)]
    struct PlayerWeaponLimitStatus;

    #[derive(Component)]
    struct PlayerWeaponLimitStatusNumber {
        kind: PlayerWeaponKind,
        current: u8,
    }

    struct PlayerJumpStatus {
        jump: bool,
        fall_time: f32,
        jump_start_y: f32,
    }

    #[derive(Component)]
    struct PlayerWeaponLimitItem {
        kind: PlayerWeaponKind,
    }

    enum Direction {
        Left,
        Right,
    }

    enum AllDirection {
        Left,
        Right,
        Up,
        Down,
    }

    pub struct GamePlugin;

    impl Plugin for GamePlugin {
        fn build(&self, app: &mut App) {
            app.insert_resource(FixedTime::new_from_secs(TIME_1F)) // 60FPS
                .add_state::<BossState>()
                .add_event::<CollisionEvent>()
                .add_systems(OnEnter(GameState::Game), (game_setup, spawn_enemy))
                .add_systems(OnEnter(BossState::Active), boss_setup)
                .add_systems(
                    Update,
                    (
                        animate_sprite,
                        move_camera,
                        move_player_weapon_limit.after(move_camera),
                        die_counter,
                    )
                        .run_if(in_state(GameState::Game)),
                )
                .add_systems(
                    Update,
                    (check_stage1_clear_system)
                        .run_if(in_state(GameState::Game))
                        .run_if(in_state(StageState::Stage1)),
                )
                .add_systems(
                    Update,
                    (check_stage2_appear_boss_system)
                        .run_if(in_state(GameState::Game))
                        .run_if(in_state(StageState::Stage2).or_else(in_state(StageState::Boss)))
                        .run_if(in_state(BossState::InActive)),
                )
                .add_systems(
                    FixedUpdate,
                    (
                        check_cllision_boss_system,
                        check_cllision_player_weapon_for_boss_system,
                        check_defeat_boss_system,
                        control_boss_system,
                        turn_around_boss_system,
                        move_boss_weapon_system,
                        boss_flash_system,
                    )
                        .run_if(in_state(GameState::Game))
                        .run_if(in_state(StageState::Boss))
                        .run_if(in_state(BossState::Active)),
                )
                .add_systems(
                    FixedUpdate,
                    (
                        control_player_system,
                        control_player_system_for_gamepad,
                        check_collision_wall_system
                            .after(control_player_system)
                            .after(control_player_system_for_gamepad),
                        check_collision_enemy_system,
                        check_collision_player_weapon_system,
                        check_collision_enemy_weapon_system,
                        check_collision_player_weapon_limit_item_system,
                        check_player_weapon_limit_status_system,
                        control_enemy_system,
                        move_enemy_system
                            .after(control_enemy_system)
                            .after(control_boss_system),
                        move_enemy_weapon_system,
                        move_player_weapon_system,
                    )
                        .run_if(in_state(GameState::Game)),
                )
                .add_systems(OnExit(GameState::Game), despawn_screen::<OnGameScreen>);
        }
    }

    fn game_setup(
        mut commands: Commands,
        asset_server: Res<AssetServer>,
        mut texture_atlases: ResMut<Assets<TextureAtlas>>,
        stage_state: Res<State<StageState>>,
    ) {
        // デスタイマー
        commands.insert_resource(DeathTimer(Timer::from_seconds(2.0, TimerMode::Once)));
        // サンダーを最初だけ一瞬止めるためのタイマー
        commands.insert_resource(ThunderStopTimer(Timer::from_seconds(
            0.0167 * 5., // 5F
            TimerMode::Once,
        )));

        // Player
        let texture_handle = asset_server.load("images/character/char.png");
        let texture_atlas = TextureAtlas::from_grid(
            texture_handle,
            Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
            5,
            1,
            None,
            None,
        );
        let texture_atlas_handle = texture_atlases.add(texture_atlas);
        let animation_indices = AnimationIndices { first: 2, last: 3 };
        commands.spawn((
            OnGameScreen,
            SpriteSheetBundle {
                texture_atlas: texture_atlas_handle,
                sprite: TextureAtlasSprite::new(animation_indices.first),
                transform: Transform::from_xyz(
                    match stage_state.get() {
                        // ボス戦のリスポーン位置はステージ途中
                        StageState::Boss => TILE_SIZE * 75.,
                        _ => TILE_SIZE * 2.,
                    },
                    TILE_SIZE * 2.,
                    2.,
                ),
                ..default()
            },
            animation_indices,
            AnimationTimer(Timer::from_seconds(0.33, TimerMode::Repeating)),
            Player {
                direction: Direction::Right,
                walk: false,
                grounded: true,
                live: true,
                jump_status: PlayerJumpStatus {
                    jump: false,
                    fall_time: 0.,
                    jump_start_y: 0.,
                },
                weapon_limit: PlayerWeaponLimit {
                    fire: 3,
                    ice: 3,
                    thunder: 3,
                },
            },
            Character,
            Velocity(Vec2::new(0.0, 0.0)),
        ));

        let mut map = match stage_state.get() {
            StageState::Stage1 => STAGE1_MAP,
            StageState::Stage2 | StageState::Boss => STAGE2_MAP,
        };
        map.reverse();

        // マップ描画
        for (row, map_str) in map.iter().enumerate() {
            let map_chars = map_str.chars().collect::<Vec<char>>();
            for (column, map_char) in map_chars.iter().enumerate() {
                if *map_char == 'A' || *map_char == 'B' {
                    // Background
                    commands.spawn((
                        OnGameScreen,
                        SpriteBundle {
                            texture: asset_server.load(if *map_char == 'A' {
                                match stage_state.get() {
                                    StageState::Stage1 => "images/map/map_1.png",
                                    StageState::Stage2 | StageState::Boss => {
                                        "images/map/map2_1.png"
                                    }
                                }
                            } else {
                                match stage_state.get() {
                                    StageState::Stage1 => "images/map/map_2.png",
                                    StageState::Stage2 | StageState::Boss => {
                                        "images/map/map2_2.png"
                                    }
                                }
                            }),
                            transform: Transform {
                                translation: Vec3::new(
                                    TILE_SIZE * column as f32,
                                    CHARACTER_SIZE * row as f32,
                                    -1.,
                                ),
                                ..default()
                            },
                            ..default()
                        },
                    ));
                }
                if *map_char == 'C' {
                    // Wall
                    commands.spawn((
                        OnGameScreen,
                        SpriteBundle {
                            texture: asset_server.load(match stage_state.get() {
                                StageState::Stage1 => "images/map/map_3.png",
                                StageState::Stage2 | StageState::Boss => "images/map/map2_3.png",
                            }),
                            transform: Transform {
                                translation: Vec3::new(
                                    TILE_SIZE * column as f32,
                                    CHARACTER_SIZE * row as f32,
                                    0.,
                                ),
                                ..default()
                            },
                            ..default()
                        },
                        Wall,
                        Collider,
                    ));
                }
            }
        }

        // プレイヤーの武器の残数表示
        let texture_handle = asset_server.load("images/status/number.png");
        let texture_atlas = TextureAtlas::from_grid(
            texture_handle,
            Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
            4,
            1,
            None,
            None,
        );
        let texture_atlas_handle = texture_atlases.add(texture_atlas);
        for i in 1..=3 {
            let y = CHARACTER_SIZE * (14 - i + 1) as f32;
            // 残数の背景
            commands.spawn((
                OnGameScreen,
                SpriteBundle {
                    texture: asset_server.load(format!("images/status/item_{}.png", i)),
                    transform: Transform {
                        translation: Vec3::new(0., y, 2.),
                        ..default()
                    },
                    ..default()
                },
                PlayerWeaponLimitStatus,
            ));

            // 残数の数字
            let animation_indices = AnimationIndices { first: 3, last: 3 };
            commands.spawn((
                OnGameScreen,
                SpriteSheetBundle {
                    texture_atlas: texture_atlas_handle.clone(),
                    sprite: TextureAtlasSprite::new(animation_indices.last),
                    transform: Transform {
                        translation: Vec3::new(0., y, 3.),
                        ..default()
                    },
                    ..default()
                },
                animation_indices,
                PlayerWeaponLimitStatusNumber {
                    kind: match i {
                        1 => PlayerWeaponKind::Fire,
                        2 => PlayerWeaponKind::Ice,
                        3 => PlayerWeaponKind::Thunder,
                        _ => PlayerWeaponKind::Fire,
                    },
                    current: 0,
                },
            ));
        }
    }

    // ボス戦開始時のセットアップ
    fn boss_setup(
        mut commands: Commands,
        mut enemy_query: Query<Entity, With<Enemy>>,
        asset_server: Res<AssetServer>,
        mut texture_atlases: ResMut<Assets<TextureAtlas>>,
    ) {
        // 壁を出現
        let walls = [
            (78, 2),
            (78, 3),
            (99, 2),
            (99, 3),
            (99, 4),
            (99, 5),
            (99, 6),
            (99, 7),
            (99, 8),
        ];
        for (column, row) in walls {
            commands.spawn((
                OnGameScreen,
                SpriteBundle {
                    texture: asset_server.load("images/map/map2_3.png"),
                    transform: Transform {
                        translation: Vec3::new(
                            TILE_SIZE * column as f32,
                            CHARACTER_SIZE * row as f32,
                            0.,
                        ),
                        ..default()
                    },
                    ..default()
                },
                Wall,
                Collider,
            ));
        }

        // ザコ敵はすべて消す
        for enemy_entity in enemy_query.iter_mut() {
            commands.entity(enemy_entity).despawn();
        }

        // ボスを出現
        let texture_handle = asset_server.load("images/character/boss.png");
        let texture_atlas = TextureAtlas::from_grid(
            texture_handle,
            Vec2::new(BOSS_SIZE, BOSS_SIZE),
            4,
            1,
            None,
            None,
        );
        let texture_atlas_handle = texture_atlases.add(texture_atlas);
        let animation_indices = AnimationIndices { first: 0, last: 1 };
        commands.spawn((
            OnGameScreen,
            SpriteSheetBundle {
                texture_atlas: texture_atlas_handle,
                sprite: TextureAtlasSprite::new(animation_indices.first),
                transform: Transform::from_xyz(TILE_SIZE * 96., TILE_SIZE * 2.5, 0.),
                ..default()
            },
            animation_indices,
            AnimationTimer(Timer::from_seconds(0.33, TimerMode::Repeating)),
            Boss {
                life: 20,
                damage_cooldown: Timer::from_seconds(BOSS_DAMAGE_COOLTIME, TimerMode::Once)
                    .tick(Duration::from_secs_f32(BOSS_DAMAGE_COOLTIME))
                    .clone(), // TODO
            },
            EnemyCharacter {
                direction: AllDirection::Left,
                stop: false,
                move_lifetime: BOSS_MOVE_LIFETIME,
                walk_step: BOSS_WALK_STEP,
                weapon_cooldown: Timer::from_seconds(BOSS_WEAPON_LIFETIME, TimerMode::Once), // TODO
            },
        ));

        // ボスの体力
        for index in 1..=20 {
            commands.spawn((
                OnGameScreen,
                SpriteBundle {
                    texture: asset_server.load("images/status/life.png"),
                    // 画面右上端から表示する。カメラを16pxずらしているのでややこしい
                    transform: Transform::from_xyz(
                        TILE_SIZE * (MAP_WIDTH_TILES - 1) as f32
                            - TILE_SIZE / 2.
                            - LIFE_SIZE / 2.
                            - LIFE_SIZE * ((index - 1) % 10) as f32,
                        480. - TILE_SIZE / 2.
                            - LIFE_SIZE / 2.
                            - (if index > 10 { LIFE_SIZE } else { 0. }),
                        0.,
                    ),
                    ..default()
                },
                BossLife { index },
            ));
        }
    }

    fn spawn_enemy(
        mut commands: Commands,
        asset_server: Res<AssetServer>,
        mut texture_atlases: ResMut<Assets<TextureAtlas>>,
        stage_state: Res<State<StageState>>,
    ) {
        let spawn_position = match stage_state.get() {
            StageState::Stage1 => STAGE1_ENEMY_POSITION.to_vec(),
            StageState::Stage2 => STAGE2_ENEMY_POSITION.to_vec(),
            StageState::Boss => vec![],
        };

        for (i, position) in spawn_position.iter().enumerate() {
            let kind = match i % 4 {
                0 => EnemyKind::Slime,
                1 => EnemyKind::Lizard,
                2 => EnemyKind::RedDemon,
                3 => EnemyKind::Wizard,
                _ => EnemyKind::Slime,
            };
            let image = match kind {
                EnemyKind::Slime => "images/character/slime.png",
                EnemyKind::Lizard => "images/character/mohican_lizard.png",
                EnemyKind::RedDemon => "images/character/red_demon.png",
                EnemyKind::Wizard => "images/character/wizard.png",
            };
            let texture_handle = asset_server.load(image);
            let texture_atlas = TextureAtlas::from_grid(
                texture_handle,
                Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
                2,
                1,
                None,
                None,
            );
            let texture_atlas_handle = texture_atlases.add(texture_atlas);
            let animation_indices = AnimationIndices { first: 0, last: 1 };
            let move_lifetime = match kind {
                EnemyKind::Slime => 30, // TODO
                _ => 20,
            };
            let walk_step = match kind {
                EnemyKind::Lizard => ENEMY_RIZZARD_WALK_STEP,
                _ => ENEMY_WALK_STEP,
            };
            commands.spawn((
                OnGameScreen,
                SpriteSheetBundle {
                    texture_atlas: texture_atlas_handle,
                    sprite: TextureAtlasSprite::new(animation_indices.first),
                    transform: Transform {
                        translation: Vec3::new(
                            TILE_SIZE * position.x as f32,
                            TILE_SIZE * (14 - position.y) as f32,
                            0.,
                        ),
                        scale: Vec3::new(-1., 1., 1.),
                        ..default()
                    },
                    ..default()
                },
                animation_indices,
                AnimationTimer(Timer::from_seconds(0.33, TimerMode::Repeating)),
                Character,
                Enemy { kind },
                EnemyCharacter {
                    direction: AllDirection::Right,
                    move_lifetime,
                    walk_step,
                    stop: false,
                    weapon_cooldown: Timer::from_seconds(ENEMY_WEAPON_LIFETIME, TimerMode::Once),
                },
            ));
        }
    }

    fn move_camera(
        query: Query<&Transform, (With<Player>, Without<Camera2d>)>,
        mut camera_query: Query<&mut Transform, With<Camera2d>>,
        boss_state: Res<State<BossState>>,
    ) {
        // ボス戦中は右端でカメラ固定
        if boss_state.get() == &BossState::Active {
            return;
        }

        let player_transform = query.single();
        let mut transform = camera_query.single_mut();
        transform.translation.x = player_transform
            .translation
            .x
            .max(304.) // 320 - 32 / 2 (タイルの中心が0,0座標なため)
            .min(TILE_SIZE * (MAP_WIDTH_TILES - 11) as f32 - 16.);
        transform.translation.y = 224.; // 240 - 32 / 2
    }

    #[allow(clippy::type_complexity)]
    fn move_player_weapon_limit(
        mut background_query: Query<
            &mut Transform,
            (
                With<PlayerWeaponLimitStatus>,
                Without<Camera2d>,
                Without<PlayerWeaponLimitStatusNumber>,
            ),
        >,
        mut number_query: Query<
            &mut Transform,
            (
                With<PlayerWeaponLimitStatusNumber>,
                Without<Camera2d>,
                Without<PlayerWeaponLimitStatus>,
            ),
        >,
        camera_query: Query<&Transform, With<Camera2d>>,
    ) {
        let camera_transform = camera_query.single();
        let x = camera_transform.translation.x - (320. - TILE_SIZE / 2.);
        for mut transform in background_query.iter_mut() {
            transform.translation.x = x;
        }
        for mut transform in number_query.iter_mut() {
            transform.translation.x = x;
        }
    }

    fn animate_sprite(
        time: Res<Time>,
        mut query: Query<(
            &AnimationIndices,
            &mut AnimationTimer,
            &mut TextureAtlasSprite,
        )>,
    ) {
        for (indices, mut timer, mut sprite) in &mut query {
            timer.tick(time.delta());
            if timer.just_finished() {
                sprite.index = if sprite.index == indices.last {
                    indices.first
                } else {
                    sprite.index + 1
                };
            }
        }
    }

    fn check_stage1_clear_system(
        mut game_state: ResMut<NextState<GameState>>,
        mut stage_state: ResMut<NextState<StageState>>,
        mut query: Query<&Transform, With<Player>>,
    ) {
        let transform = query.single_mut();
        if transform.translation.x > TILE_SIZE * (MAP_WIDTH_TILES - 2) as f32 {
            stage_state.set(StageState::Stage2);
            game_state.set(GameState::Loading);
        }
    }

    fn check_stage2_appear_boss_system(
        mut stage_state: ResMut<NextState<StageState>>,
        mut boss_state: ResMut<NextState<BossState>>,
        mut query: Query<&Transform, With<Player>>,
    ) {
        let transform = query.single_mut();
        if transform.translation.x > TILE_SIZE * (MAP_WIDTH_TILES - 11) as f32 {
            stage_state.set(StageState::Boss);
            boss_state.set(BossState::Active);
        }
    }

    fn check_defeat_boss_system(
        mut game_state: ResMut<NextState<GameState>>,
        mut query: Query<&Boss, With<Boss>>,
    ) {
        let boss = query.single_mut();
        if boss.life <= 0 {
            game_state.set(GameState::Ending);
        }
    }

    fn boss_flash_system(mut query: Query<(&Boss, &mut TextureAtlasSprite), With<Boss>>) {
        let (boss, mut texture) = query.single_mut();
        if boss.damage_cooldown.finished() {
            return;
        }

        let alpha = if (boss.damage_cooldown.remaining_secs() / TIME_1F) % 10. > 7. {
            0.
        } else {
            1.
        };
        texture.color.set_a(alpha);
    }

    fn check_player_weapon_limit_status_system(
        mut query: Query<
            (&mut PlayerWeaponLimitStatusNumber, &mut TextureAtlasSprite),
            With<PlayerWeaponLimitStatusNumber>,
        >,
        player_query: Query<&Player, With<Player>>,
    ) {
        let player = player_query.single();
        for (mut status, mut texture) in query.iter_mut() {
            let limit = match status.kind {
                PlayerWeaponKind::Fire => player.weapon_limit.fire,
                PlayerWeaponKind::Ice => player.weapon_limit.ice,
                PlayerWeaponKind::Thunder => player.weapon_limit.thunder,
                _ => 0,
            };
            if status.current != limit {
                status.current = limit;
                texture.index = limit as usize;
            }
        }
    }

    fn trigger_player_action_jump(
        player: &mut Player,
        transform: &mut Transform,
        velocity: &mut Velocity,
    ) {
        player.grounded = false;
        velocity.y = PLAYER_JUMP_FORCE;
        player.jump_status.jump = true;
        player.jump_status.jump_start_y = transform.translation.y;
        player.jump_status.fall_time = 0.;
    }

    fn trigger_player_action_weapon(
        weapon_kind: PlayerWeaponKind,
        player: &mut Player,
        transform: &mut Transform,
        weapon_query: &Query<&PlayerWeapon>,
        mut thunder_timer: &mut ResMut<ThunderStopTimer>,
        asset_server: &Res<AssetServer>,
        mut texture_atlases: &mut ResMut<Assets<TextureAtlas>>,
        mut commands: &mut Commands,
    ) {
        if weapon_query.iter().any(|weapon| weapon.kind == weapon_kind) {
            // すでに同じ武器を出しているなら何もしない
            return;
        }

        // 使用可能回数がもう0なら撃てない
        let limit = match weapon_kind {
            PlayerWeaponKind::Fire => player.weapon_limit.fire,
            PlayerWeaponKind::Ice => player.weapon_limit.ice,
            PlayerWeaponKind::Thunder => player.weapon_limit.thunder,
            _ => 1,
        };
        if limit == 0 {
            return;
        }

        let texture_handle = match weapon_kind {
            PlayerWeaponKind::Fire => asset_server.load("images/effect/fire.png"),
            PlayerWeaponKind::Ice => asset_server.load("images/effect/ice.png"),
            PlayerWeaponKind::Thunder => asset_server.load("images/effect/thunder.png"),
            PlayerWeaponKind::Sword => asset_server.load("images/effect/sword.png"),
        };
        let texture_atlas = TextureAtlas::from_grid(
            texture_handle,
            Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
            3,
            1,
            None,
            None,
        );
        let texture_atlas_handle = texture_atlases.add(texture_atlas);
        let animation_indices = AnimationIndices {
            first: 0,
            last: if weapon_kind == PlayerWeaponKind::Thunder {
                0
            } else {
                2
            },
        };
        let scale = match player.direction {
            Direction::Right => Vec3::new(1., 1., 0.),
            Direction::Left => Vec3::new(-1., 1., 0.),
        };
        let translation = match weapon_kind {
            PlayerWeaponKind::Thunder => Vec3::new(
                transform.translation.x,
                TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32,
                // 壁よりも手前に表示
                1.,
            ),
            _ => Vec3::new(
                match player.direction {
                    Direction::Right => transform.translation.x + TILE_SIZE,
                    Direction::Left => transform.translation.x - TILE_SIZE,
                },
                transform.translation.y,
                // 壁よりも手前に表示
                1.,
            ),
        };

        commands.spawn((
            OnGameScreen,
            SpriteSheetBundle {
                texture_atlas: texture_atlas_handle,
                sprite: TextureAtlasSprite::new(animation_indices.first),
                transform: Transform {
                    translation,
                    scale,
                    ..default()
                },
                ..default()
            },
            animation_indices,
            // TODO: 描画フレームは検討の余地あり
            AnimationTimer(Timer::from_seconds(TIME_1F * 6., TimerMode::Repeating)),
            PlayerWeapon {
                kind: weapon_kind.clone(),
                lifetime: Timer::from_seconds(
                    match weapon_kind {
                        PlayerWeaponKind::Fire | PlayerWeaponKind::Ice => {
                            PLAYER_WEAPON_LIFETIME_FOR_FIRE_ICE
                        }
                        PlayerWeaponKind::Thunder => PLAYER_WEAPON_LIFETIME_FOR_THUNDER,
                        PlayerWeaponKind::Sword => PLAYER_WEAPON_LIFETIME_FOR_SWORD,
                    },
                    TimerMode::Once,
                ),
            },
        ));

        // 使用したら回数を1減らす
        match weapon_kind {
            PlayerWeaponKind::Fire => player.weapon_limit.fire -= 1,
            PlayerWeaponKind::Ice => player.weapon_limit.ice -= 1,
            PlayerWeaponKind::Thunder => player.weapon_limit.thunder -= 1,
            _ => {}
        };

        // サンダーは最初だけ一瞬止めるのでタイマーをセット
        if weapon_kind == PlayerWeaponKind::Thunder {
            thunder_timer.reset();
        }
    }

    #[allow(clippy::type_complexity)]
    #[allow(clippy::too_many_arguments)]
    fn control_player_system_for_gamepad(
        gamepads: Res<Gamepads>,
        button_inputs: Res<Input<GamepadButton>>,
        axes: Res<Axis<GamepadAxis>>,
        mut query: Query<(&mut Player, &mut Transform, &mut Velocity), With<Player>>,
        weapon_query: Query<&PlayerWeapon>,
        mut thunder_timer: ResMut<ThunderStopTimer>,
        asset_server: Res<AssetServer>,
        mut texture_atlases: ResMut<Assets<TextureAtlas>>,
        mut commands: Commands,
    ) {
        let (mut player, mut transform, mut velocity) = query.single_mut();
        // デス中は何も受け付けない
        if !player.live {
            return;
        }

        for gamepad in gamepads.iter() {
            // Walk
            let left_stick_x = axes
                .get(GamepadAxis::new(gamepad, GamepadAxisType::LeftStickX))
                .unwrap();

            if left_stick_x < 0.
                || button_inputs.pressed(GamepadButton::new(gamepad, GamepadButtonType::DPadLeft))
            {
                transform.scale.x = -1.0;
                player.direction = Direction::Left;
                player.walk = true;
            } else if left_stick_x > 0.
                || button_inputs.pressed(GamepadButton::new(gamepad, GamepadButtonType::DPadRight))
            {
                transform.scale.x = 1.0;
                player.direction = Direction::Right;
                player.walk = true;
            }

            // Jump
            if player.grounded
                && button_inputs.just_pressed(GamepadButton::new(gamepad, GamepadButtonType::South))
            {
                trigger_player_action_jump(&mut player, &mut transform, &mut velocity);
            }

            // Weapon
            let weapon_kind = if button_inputs
                .just_pressed(GamepadButton::new(gamepad, GamepadButtonType::RightTrigger))
            {
                Some(PlayerWeaponKind::Fire)
            } else if button_inputs
                .just_pressed(GamepadButton::new(gamepad, GamepadButtonType::North))
            {
                Some(PlayerWeaponKind::Ice)
            } else if button_inputs
                .just_pressed(GamepadButton::new(gamepad, GamepadButtonType::East))
            {
                Some(PlayerWeaponKind::Thunder)
            } else if button_inputs
                .just_pressed(GamepadButton::new(gamepad, GamepadButtonType::West))
            {
                Some(PlayerWeaponKind::Sword)
            } else {
                None
            };
            if let Some(weapon_kind) = weapon_kind {
                trigger_player_action_weapon(
                    weapon_kind,
                    &mut player,
                    &mut transform,
                    &weapon_query,
                    &mut thunder_timer,
                    &asset_server,
                    &mut texture_atlases,
                    &mut commands,
                );
            }
        }
    }

    fn control_player_system(
        keyboard_input: Res<Input<KeyCode>>,
        mut query: Query<(&mut Player, &mut Transform, &mut Velocity), With<Player>>,
        weapon_query: Query<&PlayerWeapon>,
        mut thunder_timer: ResMut<ThunderStopTimer>,
        asset_server: Res<AssetServer>,
        mut texture_atlases: ResMut<Assets<TextureAtlas>>,
        mut commands: Commands,
    ) {
        let (mut player, mut transform, mut velocity) = query.single_mut();

        // デス中は何も受け付けない
        if !player.live {
            return;
        }

        // Walk
        if keyboard_input.pressed(KeyCode::Left) {
            transform.scale.x = -1.0;
            player.direction = Direction::Left;
            player.walk = true;
        } else if keyboard_input.pressed(KeyCode::Right) {
            transform.scale.x = 1.0;
            player.direction = Direction::Right;
            player.walk = true;
        }

        // Jump
        if player.grounded && keyboard_input.just_pressed(KeyCode::X) {
            trigger_player_action_jump(&mut player, &mut transform, &mut velocity);
        }

        // Weapon
        let weapon_kind = if keyboard_input.just_pressed(KeyCode::A) {
            Some(PlayerWeaponKind::Fire)
        } else if keyboard_input.just_pressed(KeyCode::S) {
            Some(PlayerWeaponKind::Ice)
        } else if keyboard_input.just_pressed(KeyCode::D) {
            Some(PlayerWeaponKind::Thunder)
        } else if keyboard_input.just_pressed(KeyCode::Z) {
            Some(PlayerWeaponKind::Sword)
        } else {
            None
        };
        if let Some(weapon_kind) = weapon_kind {
            trigger_player_action_weapon(
                weapon_kind,
                &mut player,
                &mut transform,
                &weapon_query,
                &mut thunder_timer,
                &asset_server,
                &mut texture_atlases,
                &mut commands,
            );
        }
    }

    // プレイヤーの移動先の壁の判定と移動の実施
    #[allow(clippy::type_complexity)]
    fn check_collision_wall_system(
        mut player_query: Query<
            (
                &mut Velocity,
                &mut Transform,
                &mut Player,
                &mut AnimationIndices,
                &mut TextureAtlasSprite,
            ),
            With<Character>,
        >,
        collider_query: Query<
            (Entity, &Transform),
            (With<Collider>, With<Wall>, Without<Character>),
        >,
        mut collision_events: EventWriter<CollisionEvent>,
        mut death_timer: ResMut<DeathTimer>,
    ) {
        let (
            mut player_velocity,
            mut player_transform,
            mut player,
            mut player_animation,
            mut player_texture_atlas,
        ) = player_query.single_mut();
        let player_size = Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE);
        let tile_size = Vec2::new(TILE_SIZE, TILE_SIZE);
        let mut next_time_translation = player_transform.translation;

        // 横移動の判定
        if player.walk {
            next_time_translation.x = match player.direction {
                Direction::Left => player_transform.translation.x - PLAYER_WALK_STEP,
                Direction::Right => player_transform.translation.x + PLAYER_WALK_STEP,
            };
            // 画面外には移動できない
            next_time_translation.x = next_time_translation.x.max(0.);

            // 地面に接しているか検査
            if player.grounded {
                // TODO: 独自実装にすることで全Wallを判定しないようにする
                let mut grounded_translation = player_transform.translation;
                grounded_translation.y -= 1.;
                grounded_translation.x = next_time_translation.x;

                let mut fall_flag = true;
                for (_collider_entity, transform) in &collider_query {
                    let collision = collide(
                        grounded_translation,
                        player_size,
                        transform.translation,
                        tile_size,
                    );

                    // 接してる壁があるなら落ちない
                    if collision.is_some() {
                        collision_events.send_default();
                        fall_flag = false;
                    }
                }
                // 接してる壁がないなら落ちる
                if fall_flag {
                    player.grounded = false;
                    player.jump_status.jump = false;
                    player.jump_status.jump_start_y = player_transform.translation.y;
                    player.jump_status.fall_time = 0.;
                }
            }
        };

        for (_collider_entity, transform) in &collider_query {
            let collision = collide(
                next_time_translation,
                player_size,
                transform.translation,
                tile_size,
            );
            if let Some(collision) = collision {
                collision_events.send_default();

                match collision {
                    // 左右なら止める
                    Collision::Left | Collision::Right => {
                        next_time_translation.x = player_transform.translation.x;
                    }
                    Collision::Top | Collision::Bottom | Collision::Inside => {}
                }
            }
        }

        // 左右への移動
        if player.walk {
            // 左右移動を反映
            player_transform.translation.x = next_time_translation.x;
            player.walk = false;
        }

        // ジャンプ or 落下していなければこの先の判定をする必要はない
        if player.grounded {
            return;
        }

        player_velocity.y -= GRAVITY * GRAVITY_TIME_STEP;
        player.jump_status.fall_time += GRAVITY_TIME_STEP;

        let t = player.jump_status.fall_time;
        next_time_translation.y = if player.jump_status.jump {
            player.jump_status.jump_start_y + PLAYER_JUMP_FORCE * t - 0.5 * GRAVITY * t * t
        } else {
            player.jump_status.jump_start_y - 0.5 * GRAVITY * t * t
        };

        // 縦方向の判定
        let is_fall = player_velocity.y < 0.;
        let is_jump = player_velocity.y > 0.;
        // TODO: collideだとどうしてもジャンプしながら壁にぶつかったときにTOPやBOTTOMが発生しておかしくなるので独自実装に切り替える
        for (_collider_entity, transform) in &collider_query {
            let collision = collide(
                next_time_translation,
                player_size,
                transform.translation,
                tile_size,
            );
            if let Some(collision) = collision {
                collision_events.send_default();

                if is_fall {
                    match collision {
                        // 落ちた先が壁なら下降をやめる
                        Collision::Top | Collision::Inside | Collision::Left | Collision::Right => {
                            player.grounded = true;
                            player_velocity.y = 0.;

                            // めり込まないように位置調整
                            if next_time_translation.y % CHARACTER_SIZE != 0.0 {
                                next_time_translation.y = next_time_translation.y
                                    + (CHARACTER_SIZE - (next_time_translation.y % CHARACTER_SIZE));
                            }
                        }
                        _ => {}
                    }
                }
                if is_jump {
                    match collision {
                        // 壁の下側に頭を当てたら上昇をやめる
                        Collision::Bottom
                        | Collision::Inside
                        | Collision::Left
                        | Collision::Right => {
                            player_velocity.y = 0.;
                            player.jump_status.jump = false;
                            player.jump_status.fall_time = 0.;

                            // めり込まないように位置調整
                            if next_time_translation.y % CHARACTER_SIZE != 0.0 {
                                next_time_translation.y = next_time_translation.y
                                    - (next_time_translation.y % CHARACTER_SIZE);
                            }
                            player.jump_status.jump_start_y = next_time_translation.y
                        }
                        _ => {}
                    }
                }
            }
        }

        // 上部の画面外にジャンプしようとしたら天井にぶつかったときと同じ処理にする
        if next_time_translation.y >= TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32 {
            player_velocity.y = 0.;
            player.jump_status.jump = false;
            player.jump_status.fall_time = 0.;
            next_time_translation.y = TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32;
            player.jump_status.jump_start_y = next_time_translation.y
        }

        // 移動を反映
        player_transform.translation.y = next_time_translation.y;

        // 落ちたときはデス処理
        if player_transform.translation.y < 0. && player.live {
            die(
                &mut player,
                &mut player_transform,
                &mut player_animation,
                &mut player_texture_atlas,
                &mut death_timer,
                true,
            );
        }
    }

    #[allow(clippy::type_complexity)]
    fn check_collision_enemy_system(
        mut player_query: Query<
            (
                &mut Transform,
                &mut Player,
                &mut AnimationIndices,
                &mut TextureAtlasSprite,
            ),
            (With<Player>, Without<Enemy>, Without<PlayerWeapon>),
        >,
        enemy_query: Query<(Entity, &Transform), With<Enemy>>,
        mut collision_events: EventWriter<CollisionEvent>,
        mut timer: ResMut<DeathTimer>,
    ) {
        let character_size = Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE);
        let (mut player_transform, mut player, mut player_animation, mut player_texture_atlas) =
            player_query.single_mut();

        // 自分と敵の接触判定
        for (_enemy_entity, enemy_transform) in &enemy_query {
            let collision = collide(
                player_transform.translation,
                character_size,
                enemy_transform.translation,
                character_size,
            );
            if collision.is_some() && player.live {
                collision_events.send_default();
                die(
                    &mut player,
                    &mut player_transform,
                    &mut player_animation,
                    &mut player_texture_atlas,
                    &mut timer,
                    false,
                );
            }
        }
    }

    // 自分の武器と敵の接触判定
    #[allow(clippy::type_complexity)]
    fn check_collision_player_weapon_system(
        mut commands: Commands,
        enemy_query: Query<(Entity, &Transform), With<Enemy>>,
        mut player_weapon_query: Query<
            (Entity, &mut Transform, &mut PlayerWeapon),
            (
                With<PlayerWeapon>,
                Without<Player>,
                Without<Enemy>,
                Without<Camera2d>,
            ),
        >,
        camera_query: Query<&Transform, With<Camera2d>>,
        mut collision_events: EventWriter<CollisionEvent>,
        asset_server: Res<AssetServer>,
    ) {
        let character_size = Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE);
        let camera_transform = camera_query.single();

        for (player_weapon_entity, player_weapon_transform, player_weapon) in
            &mut player_weapon_query
        {
            for (enemy_entity, enemy_transform) in &enemy_query {
                // カメラ外の敵に攻撃判定はしない
                if !is_inner_camera(camera_transform.translation, enemy_transform.translation) {
                    continue;
                }
                let collision = collide(
                    player_weapon_transform.translation,
                    character_size,
                    enemy_transform.translation,
                    character_size,
                );
                if collision.is_some() {
                    collision_events.send_default();
                    // FireとIceなら敵に当たったらdespawnする
                    if player_weapon.kind == PlayerWeaponKind::Fire
                        || player_weapon.kind == PlayerWeaponKind::Ice
                    {
                        commands.entity(player_weapon_entity).despawn();
                    }
                    commands.entity(enemy_entity).despawn();

                    // 20%の確率で武器を回復させるアイテムをドロップする
                    let mut rng = rand::thread_rng();
                    let weapon_drop_random = rng.gen_range(0..=4);
                    if weapon_drop_random == 0 {
                        let weapon_kind_index = rng.gen_range(1..=3);

                        // 武器使用可能回数を増やすアイテムをドロップ
                        commands.spawn((
                            OnGameScreen,
                            SpriteBundle {
                                texture: asset_server
                                    .load(format!("images/status/item_{}.png", weapon_kind_index)),
                                transform: Transform {
                                    translation: Vec3::new(
                                        enemy_transform.translation.x,
                                        enemy_transform.translation.y,
                                        1.,
                                    ),
                                    ..default()
                                },
                                ..default()
                            },
                            PlayerWeaponLimitItem {
                                kind: match weapon_kind_index {
                                    1 => PlayerWeaponKind::Fire,
                                    2 => PlayerWeaponKind::Ice,
                                    3 => PlayerWeaponKind::Thunder,
                                    _ => PlayerWeaponKind::Fire,
                                },
                            },
                        ));
                    }
                }
            }
        }
    }

    #[allow(clippy::type_complexity)]
    fn check_cllision_boss_system(
        mut player_query: Query<
            (
                &mut Transform,
                &mut Player,
                &mut AnimationIndices,
                &mut TextureAtlasSprite,
            ),
            (With<Player>, Without<Boss>),
        >,
        boss_query: Query<&Transform, With<Boss>>,
        mut collision_events: EventWriter<CollisionEvent>,
        mut timer: ResMut<DeathTimer>,
    ) {
        let character_size = Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE);
        let boss_size = Vec2::new(BOSS_SIZE, BOSS_SIZE);
        let (mut player_transform, mut player, mut player_animation, mut player_texture_atlas) =
            player_query.single_mut();

        // 自分とボスの接触判定
        let boss_transform = boss_query.single();
        let collision = collide(
            player_transform.translation,
            character_size,
            boss_transform.translation,
            boss_size,
        );
        if collision.is_some() && player.live {
            collision_events.send_default();
            die(
                &mut player,
                &mut player_transform,
                &mut player_animation,
                &mut player_texture_atlas,
                &mut timer,
                false,
            );
        }
    }

    #[allow(clippy::type_complexity)]
    fn check_cllision_player_weapon_for_boss_system(
        mut commands: Commands,
        mut boss_query: Query<(&mut Boss, &Transform), With<Boss>>,
        mut player_weapon_query: Query<
            (Entity, &mut Transform, &mut PlayerWeapon),
            (With<PlayerWeapon>, Without<Boss>),
        >,
        mut boss_life_query: Query<(Entity, &BossLife), With<BossLife>>,
        mut collision_events: EventWriter<CollisionEvent>,
        timer: Res<Time>,
    ) {
        let weapon_size = Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE);
        let boss_size = Vec2::new(BOSS_SIZE, BOSS_SIZE);
        let (mut boss, boss_transform) = boss_query.single_mut();
        boss.damage_cooldown.tick(timer.delta());

        for (player_weapon_entity, player_weapon_transform, player_weapon) in
            &mut player_weapon_query
        {
            let collision = collide(
                player_weapon_transform.translation,
                weapon_size,
                boss_transform.translation,
                boss_size,
            );
            if collision.is_some() && boss.damage_cooldown.finished() {
                collision_events.send_default();

                // ボスの体力を減少させる
                boss.life -= match player_weapon.kind {
                    PlayerWeaponKind::Fire | PlayerWeaponKind::Ice => 1,
                    _ => 2,
                };
                // 数秒ダメージを受けない無敵時間になる
                boss.damage_cooldown.reset();

                // FireとIceなら敵に当たったらdespawnする
                if player_weapon.kind == PlayerWeaponKind::Fire
                    || player_weapon.kind == PlayerWeaponKind::Ice
                {
                    commands.entity(player_weapon_entity).despawn();
                }

                // ダメージ受けた分のライフの表示も消す
                for (boss_life_entity, boss_life) in boss_life_query.iter_mut() {
                    if boss_life.index > boss.life as u8 {
                        commands.entity(boss_life_entity).despawn();
                    }
                }

                return;
            }
        }
    }

    // 武器の移動
    #[allow(clippy::type_complexity)]
    fn move_player_weapon_system(
        mut commands: Commands,
        mut player_query: Query<
            (&Transform, &Player),
            (With<Player>, Without<Enemy>, Without<PlayerWeapon>),
        >,
        mut player_weapon_query: Query<
            (
                Entity,
                &mut Transform,
                &mut PlayerWeapon,
                &mut AnimationIndices,
            ),
            (With<PlayerWeapon>, Without<Player>, Without<Enemy>),
        >,
        time: Res<Time>,
        mut thunder_timer: ResMut<ThunderStopTimer>,
    ) {
        let (player_transform, player) = player_query.single_mut();

        for (
            player_weapon_entity,
            mut player_weapon_transform,
            mut player_weapon,
            mut player_weapon_animation,
        ) in &mut player_weapon_query
        {
            match player_weapon.kind {
                PlayerWeaponKind::Fire => {
                    player_weapon_transform.translation.x += PLAYER_WEAPON_STEP
                        * if player_weapon_transform.scale.x == -1. {
                            -1.
                        } else {
                            1.
                        };
                }
                PlayerWeaponKind::Ice => {
                    player_weapon_transform.translation.x += PLAYER_WEAPON_STEP
                        * if player_weapon_transform.scale.x == -1. {
                            -1.
                        } else {
                            1.
                        };
                    player_weapon_transform.translation.y += PLAYER_WEAPON_STEP;
                }
                PlayerWeaponKind::Thunder => {
                    // サンダーは最初だけ一瞬止める
                    thunder_timer.tick(time.delta());
                    if thunder_timer.finished() {
                        // アニメーション画像を動くものに差し替える
                        if player_weapon_animation.first == 0 {
                            player_weapon_animation.first = 1;
                            player_weapon_animation.last = 2;
                        }
                        player_weapon_transform.translation.y -= PLAYER_WEAPON_THUNDER_STEP;
                    }
                }
                PlayerWeaponKind::Sword => {
                    player_weapon_transform.translation.x = match player.direction {
                        Direction::Right => player_transform.translation.x + CHARACTER_SIZE,
                        Direction::Left => player_transform.translation.x - CHARACTER_SIZE,
                    };
                    player_weapon_transform.translation.y = player_transform.translation.y;
                    player_weapon_transform.scale.x = match player.direction {
                        Direction::Right => 1.,
                        Direction::Left => -1.,
                    };
                }
            }

            player_weapon.lifetime.tick(time.delta());
            if player_weapon.lifetime.finished() {
                commands.entity(player_weapon_entity).despawn();
            }
        }
    }

    // 敵の武器と自分の接触判定
    #[allow(clippy::type_complexity)]
    fn check_collision_enemy_weapon_system(
        mut player_query: Query<
            (
                &mut Transform,
                &mut Player,
                &mut AnimationIndices,
                &mut TextureAtlasSprite,
            ),
            (With<Player>, Without<Enemy>),
        >,
        mut enemy_weapon_query: Query<
            &mut Transform,
            (With<EnemyWeapon>, Without<Enemy>, Without<Player>),
        >,
        mut collision_events: EventWriter<CollisionEvent>,
        mut death_timer: ResMut<DeathTimer>,
    ) {
        let character_size = Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE);
        let (mut player_transform, mut player, mut player_animation, mut player_texture_atlas) =
            player_query.single_mut();

        // デス中なら衝突判定を行わない
        if !player.live {
            return;
        }

        for enemy_weapon_transform in &mut enemy_weapon_query {
            // プレイヤーに当たったら死亡処理
            let collision = collide(
                enemy_weapon_transform.translation,
                character_size,
                player_transform.translation,
                character_size,
            );
            if collision.is_some() {
                collision_events.send_default();
                die(
                    &mut player,
                    &mut player_transform,
                    &mut player_animation,
                    &mut player_texture_atlas,
                    &mut death_timer,
                    false,
                );
            }
        }
    }

    // ザコ敵の武器の移動
    #[allow(clippy::type_complexity)]
    fn move_enemy_weapon_system(
        mut commands: Commands,
        mut enemy_weapon_query: Query<
            (Entity, &mut Transform, &mut EnemyWeapon),
            (
                With<EnemyWeapon>,
                Without<BossWeapon>,
                Without<Enemy>,
                Without<Player>,
            ),
        >,
        time: Res<Time>,
    ) {
        for (enemy_weapon_entity, mut enemy_weapon_transform, mut enemy_weapon) in
            &mut enemy_weapon_query
        {
            enemy_weapon_transform.translation.x += enemy_weapon.step.x;
            enemy_weapon_transform.translation.y += enemy_weapon.step.y;

            enemy_weapon.lifetime.tick(time.delta());
            if enemy_weapon.lifetime.finished() {
                commands.entity(enemy_weapon_entity).despawn();
            }
        }
    }

    // ボスの武器の移動
    #[allow(clippy::type_complexity)]
    fn move_boss_weapon_system(
        mut commands: Commands,
        mut enemy_weapon_query: Query<
            (
                Entity,
                &mut Transform,
                &mut EnemyWeapon,
                &mut BossWeapon,
                &mut AnimationIndices,
            ),
            (
                With<EnemyWeapon>,
                With<BossWeapon>,
                Without<Enemy>,
                Without<Player>,
            ),
        >,
        time: Res<Time>,
    ) {
        for (
            enemy_weapon_entity,
            mut enemy_weapon_transform,
            mut enemy_weapon,
            mut boss_weapon,
            mut boss_weapon_animation,
        ) in &mut enemy_weapon_query
        {
            // TODO: ブルーファイアとウォーターバルーンの挙動は途中で変わるので実装する

            match boss_weapon.kind {
                BossWeaponKind::WaterBalloon => {
                    // 2発目は10F,3発目は20F経過したら動く
                    if enemy_weapon.lifetime.elapsed_secs()
                        > TIME_1F * 10. * boss_weapon.index as f32
                    {
                        enemy_weapon_transform.translation.x += enemy_weapon.step.x;
                    }
                }
                BossWeaponKind::BlueFire => {
                    // 20F経過したら3方向に分かれる
                    if enemy_weapon.lifetime.elapsed_secs() > TIME_1F * 20. {
                        match boss_weapon.index {
                            1 => {
                                enemy_weapon.step.y = BOSS_WEAPON_STEP;
                            }
                            2 => {
                                enemy_weapon.step.x = 0.;
                                enemy_weapon.step.y = BOSS_WEAPON_STEP;
                            }
                            _ => {}
                        }
                    }
                    enemy_weapon_transform.translation.x += enemy_weapon.step.x;
                    enemy_weapon_transform.translation.y += enemy_weapon.step.y;
                }
                BossWeaponKind::DarkThunder => {
                    // ダークサンダーは最初だけ一瞬止める
                    boss_weapon.dark_thunder_timer.tick(time.delta());
                    if boss_weapon.dark_thunder_timer.finished() {
                        // アニメーション画像を動くものに差し替える
                        if boss_weapon_animation.first == 0 {
                            boss_weapon_animation.first = 1;
                            boss_weapon_animation.last = 2;
                        }
                        enemy_weapon_transform.translation.y -= BOSS_WEAPON_STEP * 3.;
                    }
                }
                _ => {
                    enemy_weapon_transform.translation.x += enemy_weapon.step.x;
                    enemy_weapon_transform.translation.y += enemy_weapon.step.y;
                }
            }

            enemy_weapon.lifetime.tick(time.delta());
            if enemy_weapon.lifetime.finished() {
                commands.entity(enemy_weapon_entity).despawn();
            }
        }
    }

    #[allow(clippy::type_complexity)]
    #[allow(clippy::too_many_arguments)]
    fn turn_around_boss_system(
        player_query: Query<&Transform, (With<Player>, Without<EnemyCharacter>)>,
        mut boss_query: Query<
            (&mut Transform, &mut EnemyCharacter, &Boss),
            (With<EnemyCharacter>, Without<Player>, Without<Camera2d>),
        >,
    ) {
        let (mut boss_transform, mut enemy_charactor, boss) = boss_query.single_mut();
        let player_transform = player_query.single();

        if player_transform.translation.x < boss_transform.translation.x {
            boss_transform.scale.x = 1.;
        } else {
            boss_transform.scale.x = -1.;
        }
    }

    #[allow(clippy::type_complexity)]
    #[allow(clippy::too_many_arguments)]
    fn control_boss_system(
        player_query: Query<&Transform, (With<Player>, Without<EnemyCharacter>)>,
        mut boss_query: Query<
            (&mut Transform, &mut EnemyCharacter, &Boss),
            (With<EnemyCharacter>, Without<Player>, Without<Camera2d>),
        >,
        mut commands: Commands,
        asset_server: Res<AssetServer>,
        mut texture_atlases: ResMut<Assets<TextureAtlas>>,
        time: Res<Time>,
    ) {
        let (mut boss_transform, mut enemy_charactor, boss) = boss_query.single_mut();
        let player_transform = player_query.single();

        if !enemy_charactor.weapon_cooldown.finished() {
            enemy_charactor.weapon_cooldown.tick(time.delta());
        }

        enemy_charactor.move_lifetime -= 1;
        // 現在の行動時間（移動）が終了した時
        if enemy_charactor.move_lifetime == 0 {
            enemy_charactor.move_lifetime = 40; // TODO

            // 新たな動作の抽選を始める
            enemy_charactor.stop = false;
            let mut rng = rand::thread_rng();
            let random = rng.gen_range(0..=1);

            match random {
                // 0なら止まって武器を撃つ
                0 => {
                    enemy_charactor.stop = true;
                    enemy_charactor.direction =
                        if boss_transform.translation.x >= player_transform.translation.x {
                            AllDirection::Left
                        } else {
                            AllDirection::Right
                        }
                }
                // 向いている方向に歩く
                1 => { /* nothing to do */ }
                // ランダムをmatchに書くために必要
                _ => { /* nothing to do */ }
            };

            // TODO
            // 止まったら武器を撃つ
            if enemy_charactor.stop && enemy_charactor.weapon_cooldown.finished() {
                // 連発できないよう武器が存在する期間のクールダウンタイムを開始する
                enemy_charactor.weapon_cooldown.reset();

                let random = rng.gen_range(0..=3);
                let kind = match random {
                    0 => BossWeaponKind::BlueFire,
                    1 => BossWeaponKind::DarkThunder,
                    2 => BossWeaponKind::WaterBalloon,
                    3 => BossWeaponKind::Meteor,
                    _ => BossWeaponKind::BlueFire,
                };

                let texture_handle = asset_server.load(match kind {
                    BossWeaponKind::BlueFire => "images/effect/boss_attack_bluefire.png",
                    BossWeaponKind::DarkThunder => "images/effect/boss_attack_darkthunder.png",
                    BossWeaponKind::WaterBalloon => "images/effect/boss_attack_waterballoon.png",
                    BossWeaponKind::Meteor => "images/effect/boss_attack_meteor.png",
                    _ => "images/effect/boss_attack_bluefire.png",
                });
                let texture_atlas = TextureAtlas::from_grid(
                    texture_handle,
                    Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
                    3,
                    1,
                    None,
                    None,
                );
                let texture_atlas_handle = texture_atlases.add(texture_atlas);
                let scale = if boss_transform.scale.x < 0. {
                    Vec3::new(1., 1., 0.)
                } else {
                    Vec3::new(-1., 1., 0.)
                };
                let translations = match kind {
                    BossWeaponKind::Meteor => [
                        Vec3::new(
                            TILE_SIZE * 79. + TILE_SIZE * rng.gen_range(0..=20) as f32,
                            TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32,
                            3.,
                        ),
                        Vec3::new(
                            TILE_SIZE * 79. + TILE_SIZE * rng.gen_range(0..=20) as f32,
                            TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32,
                            3.,
                        ),
                        Vec3::new(
                            TILE_SIZE * 79. + TILE_SIZE * rng.gen_range(0..=20) as f32,
                            TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32,
                            3.,
                        ),
                    ],
                    BossWeaponKind::DarkThunder => [
                        Vec3::new(
                            player_transform.translation.x - TILE_SIZE,
                            TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32,
                            3.,
                        ),
                        Vec3::new(
                            player_transform.translation.x,
                            TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32,
                            3.,
                        ),
                        Vec3::new(
                            player_transform.translation.x + TILE_SIZE,
                            TILE_SIZE * (MAP_HEIGHT_TILES - 1) as f32,
                            3.,
                        ),
                    ],
                    _ => [
                        Vec3::new(
                            if boss_transform.scale.x < 0. {
                                boss_transform.translation.x + TILE_SIZE
                            } else {
                                boss_transform.translation.x - TILE_SIZE
                            },
                            boss_transform.translation.y - TILE_SIZE / 2.,
                            3.,
                        ),
                        Vec3::new(
                            if boss_transform.scale.x < 0. {
                                boss_transform.translation.x + TILE_SIZE
                            } else {
                                boss_transform.translation.x - TILE_SIZE
                            },
                            boss_transform.translation.y - TILE_SIZE / 2.,
                            3.,
                        ),
                        Vec3::new(
                            if boss_transform.scale.x < 0. {
                                boss_transform.translation.x + TILE_SIZE
                            } else {
                                boss_transform.translation.x - TILE_SIZE
                            },
                            boss_transform.translation.y - TILE_SIZE / 2.,
                            3.,
                        ),
                    ],
                };

                for (index, translation) in translations.iter().enumerate() {
                    let animation_indices = AnimationIndices { first: 0, last: 2 };
                    commands.spawn((
                        OnGameScreen,
                        SpriteSheetBundle {
                            texture_atlas: texture_atlas_handle.clone(),
                            sprite: TextureAtlasSprite::new(animation_indices.first),
                            transform: Transform {
                                translation: *translation,
                                scale,
                                ..default()
                            },
                            ..default()
                        },
                        animation_indices,
                        // TODO: 描画フレームは検討の余地あり
                        AnimationTimer(Timer::from_seconds(TIME_1F * 6., TimerMode::Repeating)),
                        BossWeapon {
                            kind: kind.clone(),
                            dark_thunder_timer: Timer::from_seconds(
                                0.0167 * 5., // 5F
                                TimerMode::Once,
                            ),
                            index,
                        },
                        EnemyWeapon {
                            lifetime: Timer::from_seconds(BOSS_WEAPON_LIFETIME, TimerMode::Once),
                            step: {
                                match kind {
                                    BossWeaponKind::Meteor => Vec2::new(0., BOSS_WEAPON_STEP * -4.),
                                    BossWeaponKind::DarkThunder => {
                                        Vec2::new(0., BOSS_WEAPON_STEP * -3.)
                                    }
                                    _ => Vec2::new(
                                        BOSS_WEAPON_STEP * boss_transform.scale.x * -1.,
                                        0.,
                                    ),
                                }
                            },
                        },
                    ));
                }
            }
        }
    }

    #[allow(clippy::type_complexity)]
    #[allow(clippy::too_many_arguments)]
    fn control_enemy_system(
        player_query: Query<&Transform, (With<Player>, Without<EnemyCharacter>)>,
        mut enemy_query: Query<
            (&mut Transform, &mut EnemyCharacter, &Enemy),
            (With<EnemyCharacter>, Without<Player>, Without<Camera2d>),
        >,
        camera_query: Query<&Transform, With<Camera2d>>,
        mut commands: Commands,
        asset_server: Res<AssetServer>,
        mut texture_atlases: ResMut<Assets<TextureAtlas>>,
        time: Res<Time>,
    ) {
        let camera_transform = camera_query.single();
        for (mut enemy_transform, mut enemy_charactor, enemy) in &mut enemy_query {
            // カメラ外の敵は動かさない
            if !is_inner_camera(camera_transform.translation, enemy_transform.translation) {
                continue;
            }

            if !enemy_charactor.weapon_cooldown.finished() {
                enemy_charactor.weapon_cooldown.tick(time.delta());
            }

            enemy_charactor.move_lifetime -= 1;
            // 現在の行動時間（移動）が終了した時
            if enemy_charactor.move_lifetime == 0 {
                enemy_charactor.move_lifetime = 30; // TODO

                // 飛ぶ敵か止まっていたら新たな動作の抽選を始める
                // それ以外は行動を継続
                if enemy.kind == EnemyKind::RedDemon || enemy_charactor.stop {
                    enemy_charactor.stop = false;
                    let mut rng = rand::thread_rng();
                    // 飛ぶ敵は4方向移動可能
                    let random_max = if enemy.kind == EnemyKind::RedDemon {
                        4
                    } else {
                        2
                    };
                    let random = rng.gen_range(0..=random_max);

                    match random {
                        // 0ならどちらかに向いて止まる
                        0 => {
                            enemy_charactor.direction = if rng.gen() {
                                AllDirection::Left
                            } else {
                                AllDirection::Right
                            };
                            enemy_charactor.stop = true;
                        }
                        // 右に向く
                        1 => enemy_charactor.direction = AllDirection::Right,
                        // 左に向く
                        2 => enemy_charactor.direction = AllDirection::Left,
                        // 上を向く(飛ぶ敵のみ)
                        3 => enemy_charactor.direction = AllDirection::Up,
                        // 下を向く(飛ぶ敵のみ)
                        4 => enemy_charactor.direction = AllDirection::Down,
                        // ランダムをmatchに書くために必要
                        _ => { /* nothing to do */ }
                    };
                    match enemy_charactor.direction {
                        AllDirection::Left => enemy_transform.scale.x = 1.,
                        AllDirection::Right => enemy_transform.scale.x = -1.,
                        _ => {}
                    }
                }

                let player_transform = player_query.single();

                // RedDemonとWizardは止まったら武器を撃つ
                if enemy_charactor.stop
                    && (enemy.kind == EnemyKind::RedDemon || enemy.kind == EnemyKind::Wizard)
                    // プレイヤーのいる方向にしか撃たない
                    && player_transform.translation.x >= enemy_transform.translation.x * enemy_transform.scale.x * -1.
                    && enemy_charactor.weapon_cooldown.finished()
                {
                    // 連発できないよう武器が存在する期間のクールダウンタイムを開始する
                    enemy_charactor.weapon_cooldown.reset();

                    let texture_handle = match enemy.kind {
                        EnemyKind::RedDemon => {
                            asset_server.load("images/effect/enemy_attack_shockwave.png")
                        }
                        EnemyKind::Wizard => {
                            asset_server.load("images/effect/enemy_attack_wind.png")
                        }
                        _ => asset_server.load("images/effect/enemy_attack_wind.png"),
                    };
                    let texture_atlas = TextureAtlas::from_grid(
                        texture_handle,
                        Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
                        3,
                        1,
                        None,
                        None,
                    );
                    let texture_atlas_handle = texture_atlases.add(texture_atlas);
                    let animation_indices = AnimationIndices { first: 0, last: 2 };
                    let scale = if enemy_transform.scale.x < 0. {
                        Vec3::new(1., 1., 0.)
                    } else {
                        Vec3::new(-1., 1., 0.)
                    };
                    let translation = Vec3::new(
                        if enemy_transform.scale.x < 0. {
                            enemy_transform.translation.x + TILE_SIZE
                        } else {
                            enemy_transform.translation.x - TILE_SIZE
                        },
                        enemy_transform.translation.y,
                        // プレイヤーよりも手前に表示
                        3.,
                    );

                    commands.spawn((
                        OnGameScreen,
                        SpriteSheetBundle {
                            texture_atlas: texture_atlas_handle,
                            sprite: TextureAtlasSprite::new(animation_indices.first),
                            transform: Transform {
                                translation,
                                scale,
                                ..default()
                            },
                            ..default()
                        },
                        animation_indices,
                        // TODO: 描画フレームは検討の余地あり
                        AnimationTimer(Timer::from_seconds(TIME_1F * 6., TimerMode::Repeating)),
                        EnemyWeapon {
                            lifetime: Timer::from_seconds(ENEMY_WEAPON_LIFETIME, TimerMode::Once),
                            step: match enemy.kind {
                                EnemyKind::Wizard => Vec2::new(
                                    // 横移動のみ
                                    PLAYER_WEAPON_STEP
                                        * if enemy_transform.scale.x > 0. {
                                            -1.
                                        } else {
                                            1.
                                        },
                                    0.,
                                ),
                                EnemyKind::RedDemon => {
                                    // レッドデーモンの攻撃はプレイヤーの位置に目掛けて放つ
                                    // 角度を求める
                                    let temp = ((player_transform.translation.y - translation.y)
                                        / (player_transform.translation.x - translation.x))
                                        .atan();
                                    let x = (player_transform.translation.x - translation.x) / 50.; //xは50回移動でキャラに到達
                                    let y = temp.tan() * x;
                                    Vec2::new(x, y)
                                }
                                _ => Vec2::default(),
                            },
                        },
                    ));
                }
            }
        }
    }

    #[allow(clippy::type_complexity)]
    #[allow(clippy::too_many_arguments)]
    fn move_enemy_system(
        mut enemy_query: Query<
            (
                &mut Transform,
                &mut EnemyCharacter,
                Option<&Enemy>,
                Option<&Boss>,
            ),
            (With<EnemyCharacter>, Without<Player>, Without<Camera2d>),
        >,
        wall_query: Query<&Transform, (With<Wall>, Without<EnemyCharacter>, Without<Camera2d>)>,
        camera_query: Query<&Transform, With<Camera2d>>,
        mut collision_events: EventWriter<CollisionEvent>,
    ) {
        let camera_transform = camera_query.single();
        for (mut enemy_transform, mut enemy_charactor, maybe_enemy, maybe_boss) in &mut enemy_query
        {
            // カメラ外の敵は動かさない
            if !is_inner_camera(camera_transform.translation, enemy_transform.translation) {
                continue;
            }

            // 敵の移動の判定
            if !enemy_charactor.stop {
                // 衝突判定を行うために移動先のtranslationを用意
                let mut next_time_translation = enemy_transform.translation;
                next_time_translation.x = match enemy_charactor.direction {
                    AllDirection::Left => enemy_transform.translation.x - enemy_charactor.walk_step,
                    AllDirection::Right => {
                        enemy_transform.translation.x + enemy_charactor.walk_step
                    }
                    _ => enemy_transform.translation.x,
                };
                next_time_translation.y = match enemy_charactor.direction {
                    AllDirection::Up => enemy_transform.translation.y + enemy_charactor.walk_step,
                    AllDirection::Down => enemy_transform.translation.y - enemy_charactor.walk_step,
                    _ => enemy_transform.translation.y,
                };
                // 画面外
                if next_time_translation.x < 0.
                    || next_time_translation.x > TILE_SIZE * (MAP_WIDTH_TILES - 2) as f32
                    || next_time_translation.y < 0.
                    || next_time_translation.y > TILE_SIZE * 14.
                {
                    enemy_charactor.stop = true;
                    // 移動中止
                    next_time_translation = enemy_transform.translation;
                } else {
                    // 壁判定
                    for wall_transform in &wall_query {
                        let collision = collide(
                            next_time_translation,
                            Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
                            wall_transform.translation,
                            Vec2::new(TILE_SIZE, TILE_SIZE),
                        );
                        if collision.is_some() {
                            collision_events.send_default();
                            enemy_charactor.stop = true;
                            // 移動中止
                            next_time_translation = enemy_transform.translation;
                            break;
                        }
                    }
                }

                // 飛ぶ敵以外は進む先に床がなければ停止させる
                if let Some(enemy) = maybe_enemy {
                    if enemy.kind != EnemyKind::RedDemon {
                        let mut exist_floor = false;
                        for wall_transform in &wall_query {
                            let mut check_floor_position = next_time_translation;
                            check_floor_position.x = match enemy_charactor.direction {
                                AllDirection::Left => {
                                    enemy_transform.translation.x - CHARACTER_SIZE
                                }
                                AllDirection::Right => {
                                    enemy_transform.translation.x + CHARACTER_SIZE
                                }
                                _ => enemy_transform.translation.x,
                            };
                            check_floor_position.y -= 1.;
                            let collision = collide(
                                check_floor_position,
                                Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE),
                                wall_transform.translation,
                                Vec2::new(TILE_SIZE, TILE_SIZE),
                            );
                            if collision.is_some() {
                                collision_events.send_default();
                                exist_floor = true;
                            }
                        }
                        if !exist_floor {
                            enemy_charactor.stop = true;
                            // 移動中止
                            break;
                        }
                    }
                }

                // 移動を反映
                enemy_transform.translation = next_time_translation;
            }
        }
    }

    // 武器の使用可能回数を回復させるアイテムとの衝突判定
    fn check_collision_player_weapon_limit_item_system(
        mut commands: Commands,
        mut player_query: Query<(&Transform, &mut Player), With<Player>>,
        mut weapon_limit_item_query: Query<
            (Entity, &Transform, &PlayerWeaponLimitItem),
            With<PlayerWeaponLimitItem>,
        >,
        mut collision_events: EventWriter<CollisionEvent>,
    ) {
        let character_size = Vec2::new(CHARACTER_SIZE, CHARACTER_SIZE);
        let (player_transform, mut player) = player_query.single_mut();

        // デス中なら衝突判定を行わない
        if !player.live {
            return;
        }

        for (weapon_limit_item_entity, weapon_limit_item_transform, weapon_limit_item) in
            &mut weapon_limit_item_query
        {
            // プレイヤーに当たったら使用可能回数を増やす
            let collision = collide(
                weapon_limit_item_transform.translation,
                character_size,
                player_transform.translation,
                character_size,
            );
            if collision.is_some() {
                collision_events.send_default();

                match weapon_limit_item.kind {
                    PlayerWeaponKind::Fire => {
                        player.weapon_limit.fire = (player.weapon_limit.fire + 1).min(3)
                    }
                    PlayerWeaponKind::Ice => {
                        player.weapon_limit.ice = (player.weapon_limit.ice + 1).min(3)
                    }
                    PlayerWeaponKind::Thunder => {
                        player.weapon_limit.thunder = (player.weapon_limit.thunder + 1).min(3)
                    }
                    _ => {}
                };

                commands.entity(weapon_limit_item_entity).despawn();
            }
        }
    }

    // デス処理
    fn die(
        player: &mut Player,
        transform: &mut Transform,
        animation_indices: &mut AnimationIndices,
        texture_atlas_sprite: &mut TextureAtlasSprite,
        timer: &mut ResMut<DeathTimer>,
        fall: bool,
    ) {
        player.live = false;
        // デス画像に差し替え
        animation_indices.first = 4;
        animation_indices.last = 4;
        transform.scale.x *= -1.; // デス画像は左右逆になっている
        texture_atlas_sprite.index = 4;
        // デスタイマー起動
        if fall {
            // 落下時はデスタイマーは短い
            timer.set_duration(Duration::from_secs_f32(0.5));
        }
        timer.reset();
    }

    #[derive(Resource, Deref, DerefMut)]
    struct DeathTimer(Timer);

    fn die_counter(
        query: Query<&Player, With<Player>>,
        mut game_state: ResMut<NextState<GameState>>,
        time: Res<Time>,
        mut timer: ResMut<DeathTimer>,
    ) {
        let player = query.single();
        if !player.live && timer.tick(time.delta()).finished() {
            game_state.set(GameState::Loading);
        }
    }

    fn is_inner_camera(camera_translation: Vec3, target_translation: Vec3) -> bool {
        target_translation.x >= camera_translation.x - 320. - 16.
            && target_translation.x < camera_translation.x + 320. + 16.
    }
}
