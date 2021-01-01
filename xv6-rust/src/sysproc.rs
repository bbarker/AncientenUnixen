use syscall::argint;
use proc::{growproc, exit, kill, fork, sleep, wait, myproc};
use trap::ticks;
use x86::io::outw;

use core::ffi::c_void;

#[no_mangle]
pub unsafe extern "C" fn sys_fork() -> i32 {
    fork()
}

#[no_mangle]
pub unsafe extern "C" fn sys_exit() -> i32 {
    exit();
    0
}

#[no_mangle]
pub unsafe extern "C" fn sys_kill() -> i32 {
    let mut pid: i32 = 0;
    
    if argint(0, &mut pid as *mut i32) < 0 {
        -1
    } else {
        kill(pid)
    }
}

#[no_mangle]
pub unsafe extern "C" fn sys_sbrk() -> i32 {
    let mut n: i32 = 0;

    if argint(0, &mut n as *mut i32) < 0 {
        return -1;
    }

    let addr = (*myproc()).sz;
    if growproc(n) < 0 {
        return -1;
    }

    addr as i32
}

#[no_mangle]
pub unsafe extern "C" fn sys_sleep() -> i32 {
    let mut n: i32 = 0;

    if argint(0, &mut n as *mut i32) < 0 {
        return -1;
    }

    let ticks0 = ticks;

    while ((ticks - ticks0) as i32) < n {
        if (*myproc()).killed != 0 {
            return -1;
        }

        sleep(&ticks as *const u32 as *const c_void, core::ptr::null());
    }

    0
}

#[no_mangle]
pub unsafe extern "C" fn sys_wait() -> i32 {
    wait()
}

#[no_mangle]
pub unsafe extern "C" fn sys_getpid() -> i32 {
    (*myproc()).pid as i32
}

#[no_mangle]
pub unsafe extern "C" fn sys_uptime() -> i32 {
    let xticks = ticks as i32;
    return xticks;
}

#[no_mangle]
pub unsafe extern "C" fn sys_halt() -> i32 {
    outw(0x604, 0x0 | 0x2000);
    0
}
