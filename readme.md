# Yeti Project

## Yeti VM

RISC V 32bit virtual machine

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
