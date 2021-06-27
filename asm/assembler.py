from parsergen import Lexer, Token, token
from parsergen.parser_utils import TokenStream
from .asm_parser import CustomParser as AsmParser
from .parser_helpers import *
from copy import deepcopy

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

    MACRO = r"MACRO"
    ENDMACRO = r"ENDMACRO"
    MACRO_PARAM = r"\%[a-zA-Z_][a-zA-Z0-9_]*"

    @token(r"[a-zA-Z_][a-zA-Z0-9_]*")
    def ID(self, t: Token):
        if t.value in INSTRUCTIONS:
            t.type = "INSTRUCTION"
        return t
    
    COMMA = r","
    COLON = r":"
    LS_PAREN = r"\["
    RS_PAREN = r"\]"
    LPAREN = r"\("
    RPAREN = r"\)"
    
    ignore = " \t"
    ignore_comment = r"\;.*"


"""

Assembling Steps:
    - First Pass
        Generate most of machine code
    - Second Pass
        Fill in memory addresses of the JumpPointer's

"""

class Assembler:
    def __init__(self, ast: Block) -> None:
        self.ast = ast
        self.jump_pointer_addresses = {}
        self.second_pass_targets = {}
        self.macros: List[MacroDefinition] = []
        self.result = bytearray(0)
    
    def push_byte(self, value):
        self.result.append(value)
    
    def push_lots(self, value):
        self.result.extend(value)
    
    def get_type(self, operand: Operand):
        if isinstance(operand, Address):
            if isinstance(operand.target, Number):
                # Compressed memory index for constant values
                operand.type = MD
            else:
                operand.type = MV
        elif isinstance(operand, JumpPointer):
            operand.type = MD
        elif isinstance(operand, Number):
            if operand.value < 256:
                operand.type = CV8
            else:
                operand.type = CV16
        elif isinstance(operand, Register):
            operand.type = REGISTERS.index(operand.target)
        else:
            raise Exception(f"Unexpected Operand: {operand}")
        return operand.type
    
    def push_types(self, operands: List[Operand]):
        for operand in operands:
            self.get_type(operand)
            self.push_byte(operand.type)
    
    def get_aop(self, operand: Operand) -> bytes:
        if isinstance(operand, Address):
            self.get_type(operand.target) # process type for Address target Operand
            if operand.type == MD:
                return operand.target.value.to_bytes(2, "little")
            else:
                return bytes([MV, operand.target.type, *self.get_aop(operand.target)])
        elif isinstance(operand, JumpPointer):
            self.second_pass_targets[len(self.result)] = operand.target
            return bytes(2) # This address is populated in Second Pass
        elif isinstance(operand, Number):
            if operand.type == CV8:
                return operand.value.to_bytes(1, "little")
            return operand.value.to_bytes(2, "little")
        
        return b""
    
    def push_additional(self, operands: List[Operand]):
        for operand in operands:
            self.push_lots(self.get_aop(operand))

    def process_instruction(self, instruction: Instruction):
        # push opcode
        self.result.append(INSTRUCTIONS.index(instruction.opcode))

        # push operand types
        self.push_types(instruction.operands)

        # push operand additional parts
        self.push_additional(instruction.operands)
    
    def macro_fits(self, macro: MacroDefinition, call: MacroCall):
        if macro.name != call.opcode:
            return False
        if len(macro.params) != len(call.operands):
            return False
        for param, operand in zip(macro.params, call.operands):
            if not isinstance(operand, tuple(param.allowed_types)):
                return False

        return True
    
    
    def populate_macro(self, macro: MacroDefinition, call: MacroCall):
        macro = deepcopy(macro)
        # clone macro body and populate parameters with actual Operand's
        def get_replacement(item: MacroParameter):
            for macro_param, operand in zip(macro.params, call.operands):
                if macro_param.name == item.name:
                    return operand

        def populate_list(list: List):
            for index, item in enumerate(list):
                if isinstance(item, MacroParameter):
                    # lookup and populate
                    rep = get_replacement(item)
                    if rep:
                        list[index] = rep
        
        def populate_operands(node: AST):
            if isinstance(node, Block):
                for part in node.body:
                    populate_operands(part)
            elif isinstance(node, (Instruction, MacroCall,)):
                populate_list(node.operands)
            elif isinstance(node, MacroDefinition):
                populate_list(node.params)
                populate_operands(node.block)
        
        populate_operands(macro.block)
        #print("Expended Macro to", macro.block)
        self.process_block(macro.block)

    def process_macro_call(self, call: MacroCall):
        # match parameters and name with defined macros
        for macro in self.macros:
            if self.macro_fits(macro, call):
                self.populate_macro(macro, call)
                return
        raise Exception(f"Failed to find matching macro for call {call}")

    def process_block(self, block: Block):
        for part in block.body:
            print(part)
            if isinstance(part, Instruction):
                self.process_instruction(part)
            elif isinstance(part, JumpPoint):
                # set memory address of the JumpPoint
                self.jump_pointer_addresses[part.name] = len(self.result)
            elif isinstance(part, MacroDefinition):
                self.macros.append(part)
            elif isinstance(part, MacroCall):
                self.process_macro_call(part)

    def first_pass(self):
        self.process_block(self.ast)
    
    def second_pass(self):
        for address, name in self.second_pass_targets.items():
            print(self.jump_pointer_addresses)
            self.result[address:address+2] = self.jump_pointer_addresses[name].to_bytes(2, "little")
    
    def assemble(self):
        self.first_pass()
        self.second_pass()
        return self.result

def assemble(ast: Block):
    assembler = Assembler(ast)
    return assembler.assemble()