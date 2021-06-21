def process_params(first, rest):
    result = [first]
    for _, p in rest:
        result.append(p)
    return result

REGISTERS = [
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "S0",
    "S1",
    "PC",
    "SP",
    "IX",
    "-",
    "-",
    "CV8",
    "CV16",
    "MD",
    "MX",
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
]