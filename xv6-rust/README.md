# xv6-rust

This is a project to port the popular 32-bit learner's OS, xv6, over to the Rust programming language. I started this project as an undergraduate research project to gain more experience with operating systems and Rust.

One of the motivating academic factors behind this project (which has played a huge role in making this project possible for school credit) is assessing Rust's viability as a low level systems language.

[Click here](README) to see the original README that accompanied the xv6 revision 10 distribution.

[Click here](README-PDX) to see the original Portland State README that accompanied the course-specific xv6 distribution.

# Building and Running

Prerequisites:

1. A linux environment.

1. The QEMU simulator.

1. The `gcc` compiler suite.

1. The Rust compiler.

1. `cargo-xbuild` (`cargo install cargo-xbuild`).

1. A nightly override for the cloned repository (`rustup override set nightly`).

1. The Rust source (`rustup component add rust-src`).

Building:

1. Run `make`.

Running:

1. Run `make run`.

Debugging:

1. Run `make debug`; QEMU will expose a debugging port for GDB to attach to.

1. In another terminal session, run `rust-gdb`.
