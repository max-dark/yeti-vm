# Yeti Project

## Yeti VM

RISC V 32bit virtual machine

### release/v0.0.3

#### Changes

 * implement tests for RV32I
 * add vm::bit_utils
 * add instruction encoder
 * CMake: add install targets for libraries

### release/v0.0.2

#### Changes

 * fix offset calculation in `load`/`store`
 * fix `div`/`rem` instructions: handle division by zero / overflows
 * add scripts for building RISC-V executables

### release/v0.0.1

#### Supported instruction sets:

 * RV32I - load/store data, basic math, jumps and branches
 * RV32M - integer multiplication/division

#### Customisation points

 * opcode handlers
 * memory access handlers
 * code loaders
 * system calls

#### Supported formats

 * binary (`objcopy -O binary`)
 * intel hex (`objcopy -O ihex`)
