from parsergen.parsergen import Generator

HEADER = """from .parser_helpers import *

"""

with open("asm.gram") as f:
    grammar = f.read()

result = Generator().generate(grammar)

with open("asm_parser.py", "w") as f:
    f.write(HEADER + result)