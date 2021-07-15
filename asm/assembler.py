from parsergen import Lexer, Token, token
from parsergen.parser_utils import TokenStream
from .asm_parser import CustomParser as AsmParser
from .parser_helpers import *
from copy import deepcopy

class AsmLexer(Lexer):

    @token(r"\n")
    def N(self, t: Token):
        self.lineno += 1
        self.column = 0
        return t
    
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

    IF = r"IF"
    ENDIF = r"ENDIF"

    @token(r"[a-zA-Z_\.][a-zA-Z0-9_]*")
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
        self.if_label_index = 0

        self.macros: List[MacroDefinition] = [
            # register builtin macros
            BuiltinMacro(".data", [], None, self._macro_data),
            BuiltinMacro(".align", [], None, self._macro_align),
        ]
        self.result = bytearray(0)
    
    def _macro_data(self, call: MacroCall):
        size_lookups = {
            "u8": 1,
            "u16": 2
        }
        # process first parameter for identifying how many bytes:
        if isinstance(call.operands[0], LabelPointer):
            size = size_lookups[call.operands[0].target]
        else:
            size = call.operands[0].value
        target = call.operands[1].value & (2 ** (size * 8) - 1) # mask with max value
        value = target.to_bytes(size, "little")
        #print("PUSH: ", value)
        self.push_lots(value)
    
    def _macro_align(self, call: MacroCall):
        address = call.operands[0].value # has to be a number at the moment
        amount = address - len(self.result)
        self.push_lots(bytearray(amount))
    
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
        elif isinstance(operand, LabelPointer):
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
        elif isinstance(operand, LabelPointer):
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
        if macro.name == call.opcode and isinstance(macro, BuiltinMacro):
            return True
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
                if isinstance(macro, BuiltinMacro):
                    macro.action(call)
                    return
                self.populate_macro(macro, call)
                return
        raise Exception(f"Failed to find matching macro for call {call}")
    
    def process_if_statement(self, part: IfStatement):
        """
        IF 0:
            something
        ENDIF
        ; translated to:
        
        jmp 0 <if_label_0>
        jmp 7 <if_label_1>
        <if_label_0>:
            something
        <if_label_1>
        """
        execute_if = f"<if_label_{self.if_label_index}>"
        self.if_label_index += 1
        skip_if = f"<if_label_{self.if_label_index}>"
        self.if_label_index += 1
        self.process_instruction(Instruction("jmp", [part.condition, LabelPointer(execute_if)]))
        self.process_instruction(Instruction("jmp", [Number(7), LabelPointer(skip_if)]))
        self.jump_pointer_addresses[execute_if] = len(self.result)
        self.process_block(part.block)
        self.jump_pointer_addresses[skip_if] = len(self.result)



    def process_block(self, block: Block):
        for part in block.body:
            print(part)
            if isinstance(part, Instruction):
                self.process_instruction(part)
            elif isinstance(part, Label):
                # set memory address of the JumpPoint
                self.jump_pointer_addresses[part.name] = len(self.result)
            elif isinstance(part, MacroDefinition):
                self.macros.append(part)
            elif isinstance(part, MacroCall):
                self.process_macro_call(part)
            elif isinstance(part, IfStatement):
                self.process_if_statement(part)

    def first_pass(self):
        self.process_block(self.ast)
    
    def second_pass(self):
        for address, name in self.second_pass_targets.items():
            self.result[address:address+2] = self.jump_pointer_addresses[name].to_bytes(2, "little")
    
    def assemble(self):
        self.first_pass()
        self.second_pass()
        #print(self.jump_pointer_addresses)
        #print(self.second_pass_targets)
        return self.result

def assemble(ast: Block):
    assembler = Assembler(ast)
    return assembler.assemble()