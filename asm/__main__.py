from parsergen import Lexer, Token, token
from parsergen.parser_utils import TokenStream
from .asm_parser import CustomParser as AsmParser
from .parser_helpers import *

class AsmLexer(Lexer):

    @token(r"\n+")
    def NEWLINE(self, t: Token):
        self.lineno += len(t.value)
    
    @token(r"[0-9]+")
    def INT(self, t: Token):
        t.value = int(t.value)
        return t
    
    @token(r"\$[0-9a-fA-F]+")
    def HEX(self, t: Token):
        t.value = int(t.value[1:], base=16)
        return t
    
    @token(r"\%[01]+")
    def BIN(self, t: Token):
        t.value = int(t.value[1:], base=2)
        return t

    INSTRUCTION = r'(inc)|(dec)|(add)|(addc)|(sub)|(subb)|(rol)|(rolc)|(ror)|(rorc)|(and)|(or)|(xor)|(cmp)|(push)|(pop)|(jmp)|(jsr)|(nop)|(mov)'

    ID = r"[a-zA-Z_][a-zA-Z0-9_]*"
    COMMA = r","
    COLON = r":"
    LPAREN = r"\["
    RPAREN = r"\]"
    
    ignore = " \t"
    ignore_comment = r"\;.*"


    

def assemble(instructions: list):
    result = bytearray(0)
    jmp_points = {}
    
    
    def get_opcode(opcode: str):
        return INSTRUCTIONS.index(opcode.lower())

    def get_reg(t: str):
        return REGISTERS.index(t)

    def get_identifier(t: str, v):
        if t == "addr" or t == "jmp_pointer":
            return 0x11
        elif t == "cv":
            if v > 255:
                return 0x10 # CV16
            return 0x0F # CV8
        return get_reg(v)

    def split(t, v):
        if t == "reg":
            return b""
        i = get_identifier(t, v).to_bytes(2, "little")
        if t == "jmp_pointer":
            return jmp_points[v].to_bytes(2, "little")
        if t == "addr" or i == 0x10:
            return v.to_bytes(2, "little")
        return v.to_bytes(1, "little")
    
    for opcode, params in instructions:
        if opcode == "jmp_point":
            jmp_points[params] = len(result)
            continue
        o = get_opcode(opcode)
        #print(hex(o), end=" ")
        result.append(o)
        for t, v in params: # add identifiers
            #print(t, end=" ")
            result.append(get_identifier(t, v))
        #print()
        for t, v in params: # push values
            result.extend(split(t, v))
    
    return result



def main():
    import argparse
    parser = argparse.ArgumentParser("asm", description="assembler")
    parser.add_argument("inputfile", type=argparse.FileType("r", encoding="utf-8"), nargs="?")
    parser.add_argument("-o", type=str, metavar="outputfile")

    options = parser.parse_args()
    data = options.inputfile.read()
    options.inputfile.close()


    lexer_result = AsmLexer().lex_string(data)
    stream = TokenStream(lexer_result) # create token stream
    parser = AsmParser(stream)
    result = parser.program()
    error = parser.error()
    if result is None and error is not None:
        print(error) # error handling
    else:
        print(result)
        result = assemble(result)
        with open(options.o, "wb") as f:
            f.write(result)
        print(", ".join(hex(v) for v in result))

if __name__ == "__main__":
    main()