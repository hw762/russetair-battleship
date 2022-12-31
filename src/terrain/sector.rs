use hecs::Entity;

pub struct Sector;

impl Sector{
    const SECTOR_SIZE: u32 = 64;
}

pub struct SectorCoordinate {
    pub x: i32,
    pub y: i32,
}

pub struct SectorHeight(pub f32);

pub struct SectorData {
    pub chunks: [Entity;(Sector::SECTOR_SIZE * Sector::SECTOR_SIZE) as usize],
}