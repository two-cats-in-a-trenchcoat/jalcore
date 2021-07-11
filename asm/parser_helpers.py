from typing import *
from dataclasses import dataclass

from parsergen.parser_utils import Filler
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
class LabelPointer(Operand):
    target: str

@dataclass
class Number(Operand): # Constant
    value: int

class Statement(AST):
    pass

@dataclass
class Block(AST):
    body: List[Statement]

@dataclass
class Label(Statement):
    name: str

@dataclass
class Instruction(Statement):
    opcode: str
    operands: List[Operand]

@dataclass
class MacroParameter(AST):
    name: str
    allowed_types: List[AST]

@dataclass
class MacroDefinition(Statement):
    name: str
    params: List[MacroParameter]
    block: Block

@dataclass
class BuiltinMacro(MacroDefinition):
    action: Callable

@dataclass
class MacroCall(Statement):
    opcode: str
    operands: List[Operand]

@dataclass
class IfStatement(Statement):
    condition: Operand
    block: Block



def process_params(p):
    if isinstance(p, Filler):
        return []
    result = [p[0]]
    for _, p in p[1]:
        result.append(p)
    return result

constraint_map = {
    "addr": Address,
    "reg": Register,
    "cv": Number,
    "__m__": MacroParameter
}

def construct_macro_param(name: str, constraints):
    # constraints  :  (LPAREN ID (COMMA ID)* RPAREN)?
    if isinstance(constraints, Filler):
        return MacroParameter(name, [Operand])
    
    constraint_strings = [constraints[1].value]
    constraint_strings += [t.value for _, t in constraints[2]]
    converted_constraints = []
    for string in constraint_strings:
        converted_constraints.append(constraint_map[string])
    
    return MacroParameter(name, converted_constraints)

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
    "rdw",
]