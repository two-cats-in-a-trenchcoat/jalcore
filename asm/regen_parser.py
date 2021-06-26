import os
folder = os.path.dirname(__file__)
from parsergen.parsergen import Generator

HEADER = """from .parser_helpers import *

"""

with open(f"{folder}/asm.gram") as f:
    grammar = f.read()

result = Generator().generate(grammar)

with open(f"{folder}/asm_parser.py", "w") as f:
    f.write(HEADER + result)