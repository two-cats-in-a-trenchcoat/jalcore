from .assembler import *


def main():
    import argparse
    parser = argparse.ArgumentParser("asm", description="assembler")
    parser.add_argument("inputfile", type=argparse.FileType("r", encoding="utf-8"), nargs="?")
    parser.add_argument("-o", type=str, metavar="outputfile")
    parser.add_argument("--regen", action="store_true")

    options = parser.parse_args()
    if options.inputfile is None:
        if options.regen:
            from . import regen_parser
            return
        print("No file specified. Please give a file to assemble.")
        return
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
        #print(result)
        result = assemble(result)
        with open(options.o, "wb") as f:
            f.write(result)
        #print(", ".join(hex(v) for v in result))
        

if __name__ == "__main__":
    main()
