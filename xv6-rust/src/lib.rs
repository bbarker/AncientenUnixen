#![no_std]

#[macro_use]
extern crate bitfield;
#[macro_use]
extern crate lazy_static;
extern crate spin;
extern crate x86;

pub mod arch;
#[macro_use]
pub mod console;
pub mod file;
pub mod fs;
pub mod ioapic;
pub mod lapic;
pub mod kbd;
pub mod string;
pub mod syscall;
pub mod sysproc;
pub mod mmu;
pub mod param;
pub mod pipe;
pub mod proc;
pub mod spinlock;
pub mod trap;
pub mod traps;
pub mod types;
pub mod uart;


use core::panic::PanicInfo;

#[no_mangle]
pub unsafe extern "C" fn kmain() {
    println!("Hello from {}", "Rust");
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
