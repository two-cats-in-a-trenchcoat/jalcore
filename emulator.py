from os import read
from typing import *
from dataclasses import dataclass



@dataclass
class Operand:
    value: int
    target: int
    type: str

def execute(memory: bytearray):
    registers = {
        n: 0 for n in range(20)
    }
    
    # registers
    PC = 0x0A
    S0 = 0x08
    IX = 0x0C
    CV = 0x0F
    MD = 0x10
    MX = 0x11

    # opcodes
    INC = 0x01
    DEC = 0x02
    ADD = 0x03

    SUB = 0x04
    
    OR  = 0x0B

    def offset_address(type, address: int):
        if type == MX:
            return registers[IX] + address
        return address
    
    def read_byte():
        b = memory[registers[PC]]
        registers[PC] += 1
        #print(b)
        return b

    def read_number(num_bytes: int):
        return int.from_bytes(bytearray([read_byte() for _ in range(num_bytes)]), "little")
    
    def is_2byte_register(reg: int):
        return 0x0A <= reg <= 0x0E

    def get_types(amount: int):
        return [read_byte() for _ in range(amount)]
    
    def get_aop(type: int, long=False):
        if type == MD or type == MX: # Memory Index
            addr = offset_address(type, read_number(num_bytes=2))
            value = memory[addr]
            return Operand(value, addr, "Mx")
        elif type == CV: # Constant Value
            if long:
                return Operand(read_number(num_bytes=2), 0, "CV")
            return Operand(read_byte(), 0, "CV")
        else:
            return Operand(registers[type], type, "REG")

    def store(operand: Operand, value):
        if operand.type == "Mx":
            memory[operand.target] = value
        elif operand.type == "REG":
            registers[operand.target] = value


    while True:
        opcode = memory[registers[PC]]
        registers[PC] += 1
        if opcode == 0x00: # inc <tar>(8R, 16R, Mx)
            reg, = get_types(1)
            tar = get_aop(reg)
            store(tar, tar.value + 1)
        
        elif opcode == 0x01: # dec <tar>(8R, 16R, Mx)
            reg, = get_types(1)
            tar = get_aop(reg)
            store(tar, tar.value - 1)
        
        elif opcode == 0x02: # add <src>(CV, 8R, Mx) <tar>(8R, Mx)
            src_r, tar_r = get_types(2)
            long = is_2byte_register(tar_r)
            src = get_aop(src_r, long=long)
            tar = get_aop(tar_r)
            result = src.value + tar.value
            carry = (result & 0xF00) != 0
            store(tar, result)
            registers[S0] |= carry # set carry bit
        

        elif opcode == 0x0B: # or <src>(CV, 8R, Mx) <tar>(8R, Mx)
            src_r, tar_r = get_types(2)
            long = is_2byte_register(tar_r)
            src = get_aop(src_r, long=long)
            tar = get_aop(tar_r)
            store(tar, src.value | tar.value)
        
        
        if registers[S0] and 0b00010000: # halt bit
            print("Halted")
            print(registers)
            #print(memory.hex())
            break

memory = bytearray(256)
program = [
#   inc    MD   $FF   $00
    0x00, 0x10, 0xFF, 0x00,
#   add    CV    R0    5
    0x02, 0x0F, 0x00, 0x05,
#   or     CV    S0    %00010000
    0x0B, 0x0F, 0x08, 0b00010000 # set halt bit
]
for i, v in enumerate(program):
    memory[i] = v

execute(memory)

# TODO:
# make registers not be able to be negative
# add other commands