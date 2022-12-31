use hecs::*;

pub struct Chunk;

impl Chunk {
    const CHUNK_SIZE: u32 = 16;

    pub fn spawn_at(world: &mut World, x: i32, y: i32, h: f32) -> Entity {
        world.spawn((Chunk, ChunkCoordinate{x, y}, ChunkHeight(h)))
    }

    pub fn spawn_with(world: &mut World, x: i32, y: i32, data: ChunkData) -> Entity {
        let h = data.heights.iter().sum::<f32>() / data.heights.len() as f32;
        world.spawn((Chunk, ChunkCoordinate{x, y}, ChunkHeight(h), data))
    }
}

pub struct ChunkCoordinate {
    pub y: i32,
    pub x: i32,
}

pub struct ChunkHeight(pub f32);

pub struct ChunkData {
    pub heights: [f32; (Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE) as usize],
}