use proc::Cpu;

use core::ffi;

#[repr(C)]
pub struct Spinlock {
    locked: u32,
    name: *const u8,
    cpu: *const Cpu,
    pcs: [u32; 10],
}

extern "C" {
    pub fn acquire(s: *const Spinlock);
    pub fn release(s: *const Spinlock);
    pub fn getcallerpcs(v: ffi::c_void, pcs: *const u32);
}
