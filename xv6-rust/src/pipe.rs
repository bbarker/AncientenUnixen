const PIPESIZE: usize = 512;

#[repr(C)]
pub struct Pipe {
    // Lock goes here
    data: [u8; PIPESIZE],
    nread: u32,
    nwrite: u32,
    readopen: i32,
    writeopen: i32,
}
