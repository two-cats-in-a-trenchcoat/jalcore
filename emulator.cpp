#include <stdio.h>
#include <string>
#include <iostream>

// Set nth bit of target to value
#define BIT_SET(target, n, value) (target = (target & ~(1ULL << n)) | (value << n))
// check if nth bit of target is set
#define BIT_CHECK(target, n) ((target >> n) & 1ULL)

char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::string getHex(char c){
    std::string r = "";
    r += hex[(c & 0xF0) >> 4];
    r += hex[c & 0xF];
    return r;
}

std::string dumpMem(unsigned char *memory, int size){
    std::string r = "";
    for (int i = 0; i < size; i++){
        const char ch = memory[i];
        r += getHex(ch);
    }
    return r;
}

struct Operand {
    int value;
    int target;
    std::string type;
};

struct Registers {
    unsigned char R0 = 0;
    unsigned char R1 = 0;
    unsigned char R2 = 0;
    unsigned char R3 = 0;
    unsigned char R4 = 0;
    unsigned char R5 = 0;
    unsigned char R6 = 0;
    unsigned char R7 = 0;
    unsigned char S0 = 0;
    unsigned char S1 = 0;
    /*
        Flags in S0 (MSB to LSB)
        1   Hardwired 1
        0	Hardwired 0
        -	
        H	Halt
        -	
        N	Negative *not implemented
        Z	Zero
        C	Carry

        Flags in S1 (MSB to LSB)
        -
        -
        -
        -
        -
        -
        -
        -

    */
    unsigned short PC = 0;
    unsigned short SP = 0;
    unsigned short IX = 0;
    // CV, MD, MX
};

const unsigned char PC = 0x0A;
const unsigned char CV8 = 0x0F;
const unsigned char CV16 = 0x10;
const unsigned char MD = 0x11;
const unsigned char MX = 0x12;

struct State {
    Registers registers;
    unsigned char *memory;
    int size;
};

class Emulator {
    public:
        State state;
        
        Emulator(State _state){
            state = _state;
        }
        
        unsigned char read_byte(){
            unsigned char byte = state.memory[state.registers.PC];
            state.registers.PC += 1;
            return byte;
        }

        int offset_address(unsigned char type, int address){
            if (type == MX) return state.registers.IX + address;
            return address;
        }

        int read_number(int numBytes){
            unsigned char b1 = read_byte();
            unsigned char b2 = read_byte();
            int result = (b2 << 4) + (b1);
            //printf("%s %s -> %d\n", getHex(b1).c_str(), getHex(b2).c_str(), result);
            return result;
        }

        bool is_2byte_register(unsigned char type){
            return type >= 0x0A && type <= 0x0E;
        }

        void get_types(int amount, unsigned char* result){
            for (int i = 0; i < amount; i++){
                result[i] = read_byte();
            }
        }

        int loadRegister(unsigned char type){
            switch (type){
                case 0x00: // R0
                    return state.registers.R0;
                case 0x01: // R1
                    return state.registers.R1;
                case 0x02: // R2
                    return state.registers.R2;
                case 0x03: // R3
                    return state.registers.R3;
                case 0x04: // R4
                    return state.registers.R4;
                case 0x05: // R5
                    return state.registers.R5;
                case 0x06: // R6
                    return state.registers.R6;
                case 0x07: // R7
                    return state.registers.R7;
                
                case 0x08: // S0
                    return state.registers.S0;
                case 0x09: // S1
                    return state.registers.S1;
                
                case 0x0A: // PC
                    return state.registers.PC;
                case 0x0B: // SP
                    return state.registers.SP;
                case 0x0C: // IX
                    return state.registers.IX;
            }
            printf("Invalid type %d\n", type);
            return 0;
        }

        void setRegister(unsigned char type, int value){
            switch (type){
                case 0x00: // R0
                    state.registers.R0 = value;
                    break;
                case 0x01: // R1
                    state.registers.R1 = value;
                    break;
                case 0x02: // R2
                    state.registers.R2 = value;
                    break;
                case 0x03: // R3
                    state.registers.R3 = value;
                    break;
                case 0x04: // R4
                    state.registers.R4 = value;
                    break;
                case 0x05: // R5
                    state.registers.R5 = value;
                    break;
                case 0x06: // R6
                    state.registers.R6 = value;
                    break;
                case 0x07: // R7
                    state.registers.R7 = value;
                    break;
                
                case 0x08: // S0
                    state.registers.S0 = value;
                    break;
                case 0x09: // S1
                    state.registers.S1 = value;
                    break;
                
                case 0x0A: // PC
                    state.registers.PC = value;
                    break;
                case 0x0B: // SP
                    state.registers.SP = value;
                    break;
                case 0x0C: // IX
                    state.registers.IX = value;
                    break;
            }
        }

        Operand get_aop(unsigned char type){
            if (type == MD || type == MX){ // Memory Index
                int addr = offset_address(type, read_number(2));
                int value = state.memory[addr];
                return Operand {value, addr, "Mx"};
            }
            else if (type == CV8) return Operand {read_byte(), 0, "CV"}; // 8 bit Constant
            else if (type == CV16) return Operand {read_number(2), 0, "CV"}; // 16 bit Constant
            
            return Operand {loadRegister(type), type, "REG"};
        }

        void get_params(int amount, Operand *result){
            unsigned char types[amount];
            get_types(amount, types);
            for (int i = 0; i < amount; i++){
                //printf("%d\n", i);
                Operand p = get_aop(types[i]);
                result[i] = p;
            }
        }

        void store(Operand operand, int value){
            if (operand.type == "Mx"){
                state.memory[operand.target] = value;
            }
            else if (operand.type == "REG"){
                setRegister(operand.target, value);
            }
        }

        bool getFlag(std::string target, unsigned int bit){
            if (target == "S0") return BIT_CHECK(state.registers.S0, bit);
            if (target == "S1") return BIT_CHECK(state.registers.S1, bit);
        }

        void setFlag(std::string target, unsigned int bit, bool value){
            if (target == "S0") BIT_SET(state.registers.S0, bit, value);
            if (target == "S1") BIT_SET(state.registers.S1, bit, value);
        }

        // Instruction definitions

        void op_inc(){
            // inc <tar>(8R, 16R, Mx)
            Operand params[1];
            get_params(1, params);
            Operand tar = params[0];
            int result = tar.value - 1;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_dec(){
            // dec <tar>(8R, 16R, Mx)
            Operand params[1];
            get_params(1, params);
            Operand tar = params[0];
            int result = tar.value - 1;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_add(){
            // add <src>(CV, 8R, Mx) <tar>(8R, Mx)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            int result = src.value + tar.value;
            store(tar, result);
            setFlag("S0", 0, result > 0xFF);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_addc(){
            // addc <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            int result = src.value + tar.value + getFlag("S0", 0);
            store(tar, result);
            setFlag("S0", 0, result > 0xFF);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_sub(){
            // sub <src>(CV, 8R, Mx) <tar>(8R, Mx)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            int result = tar.value - src.value;
            store(tar, result);
            setFlag("S0", 0, src.value > tar.value);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_subb(){
            // subb <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            int result = tar.value - (src.value + getFlag("S0", 0));
            store(tar, result);
            setFlag("S0", 0, (src.value + getFlag("S0", 0)) > tar.value);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_or(){
            // or <src>(CV, 8R, Mx) <tar>(8R, Mx)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            int result = src.value | tar.value;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_jmp(){
            // jmp <src>(CV) <tar>(Mx) <src-aop> <tar-aop>
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            //printf("src: %x target: %d\n", src.value, tar.target);
            bool flip = BIT_CHECK(src.value, 7);
            BIT_SET(src.value, 7, 0);
            bool flagSet;
            if (src.value <= 7) flagSet = getFlag("S0", src.value);
            else getFlag("S1", src.value);
            flagSet ^= flip;
            if (flagSet) state.registers.PC = tar.target;
        }



        // Execution loop

        void execute(){
            while (1){
                unsigned char opcode = read_byte();
                printf("Opcode: %s\n", getHex(opcode).c_str());
                switch (opcode){
                    case 0x00: op_inc(); break;
                    case 0x01: op_dec(); break;
                    case 0x02: op_add(); break;
                    case 0x03: op_addc(); break;
                    case 0x04: op_sub(); break;
                    case 0x05: op_subb(); break;
                    // TODO: implement bit shifting instructions and other logic ops
                    case 0x0B: op_or(); break;
                    case 0x10: op_jmp(); break;
                    default:
                        printf("Unexpected opcode %#x, PC: %d", opcode, state.registers.PC);
                        exit(1);
                }

                // halt check
                if (getFlag("S0", 4)){
                    printf("Halted\n");
                    //print(memory.hex())
                    break;
                }
            }
        }
};


int main(){
    unsigned char memory[0x100] = {
    //  add    CV8   R0    10                    ; add 10 to R0
        0x02, 0x0F, 0x00, 0x0C,
    //  dec    R0
        0x01, 0x00,
    //  jmp   CV8    MD  %0b10000001  0x0004     ; jump to address 0x0004 if not zero
    //                                           ; branching if flag bit 1 is *not* set
        0x10, 0x0F, 0x11, 0b10000001, 0x04, 0x00,
    //  or     CV8   S0    %00010000
        0x0B, 0x0F, 0x08, 0b00010000 // set halt bit
    };
    State state {Registers {}, memory, 0x100};
    Emulator emu {state};
    emu.execute();
    std::cout << dumpMem(state.memory, state.size) << "\n";
    return 0;
}