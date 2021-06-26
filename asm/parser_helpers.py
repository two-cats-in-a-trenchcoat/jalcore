def process_params(first, rest):
    result = [first]
    for _, p in rest:
        result.append(p)
    return result

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