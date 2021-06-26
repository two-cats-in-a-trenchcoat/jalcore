from typing import *
from dataclasses import dataclass
# define ast nodes

class AST:
    def __repr__(self) -> str:
        values = ", ".join(f"{k}={v}" for k, v in vars(self).items())
        return f"{self.__class__.__qualname__}({values})"


class Operand:
    type: int = None

@dataclass
class Address(Operand):
    target: Operand

@dataclass
class Register(Operand):
    target: str

@dataclass
class JumpPointer(Operand):
    target: str

@dataclass
class Number(Operand): # Constant
    value: int

@dataclass
class Block(AST):
    body: List[AST]

@dataclass
class JumpPoint(AST):
    name: str

@dataclass
class Instruction(AST):
    opcode: str
    operands: List[Operand]



def process_params(first, rest):
    result = [first]
    for _, p in rest:
        result.append(p)
    return result

PC = 0x0A
CV8 = 0x0F
CV16 = 0x10
MD = 0x11
MX = 0x12
MV = 0x13

REGISTERS = [
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7",
    "s0",
    "s1",
    "pc",
    "sp",
    "ix",
    "-",
    "-",
    "cv8",
    "cv16",
    "md",
    "mx",
    "mv",
    "rf",
]

INSTRUCTIONS = [
    "inc",
    "dec",
    "add",
    "addc",
    "sub",
    "subb",
    "rol",
    "rolc",
    "ror",
    "rorc",
    "and",
    "or",
    "xor",
    "cmp",
    "push",
    "pop",
    "jmp",
    "jsr",
    "nop",
    "mov",
    "ret",
]