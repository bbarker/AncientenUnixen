use core::ffi;

#[repr(C)]
pub struct TaskState {
    link: u32,
    esp0: u32,
    ss0: u16,
    padding1: u16,
    esp1: *const u32,
    ss1: u16,
    padding2: u16,
    esp2: *const u32,
    ss2: u16,
    padding3: u16,
    cr3: *const ffi::c_void,
    eip: *const u32,
    eflags: u32,
    eax: u32,
    ecx: u32,
    edx: u32,
    ebx: u32,
    esp: *const u32,
    ebp: *const u32,
    esi: u32,
    edi: u32,
    es: u16,
    padding4: u16,
    cs: u16,
    padding5: u16,
    ss: u16,
    padding6: u16,
    ds: u16,
    padding7: u16,
    fs: u16,
    padding8: u16,
    gs: u16,
    padding9: u16,
    ldt: u16,
    padding10: u16,
    t: u16,
    iomb: u16,
}

bitfield!{
    #[repr(C)]
    pub struct SegDesc(MSB0 [u8]);
    u32;
    get_lim_15_0, _: 15, 0;
    get_base_15_0, _: 31, 16;
    base_23_16, _: 39, 32;
    segtype, _: 43, 40;
    s, _: 44;
    dpl, _: 46, 45;
    p, _: 47;
    lim_19_16, _: 51, 48;
    avl, _: 52;
    rsv1, _: 53;
    db, _: 54;
    g, _: 55;
    base_31_24, _: 63, 56;
}
