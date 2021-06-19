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
const unsigned char CV = 0x0F;
const unsigned char MD = 0x10;
const unsigned char MX = 0x11;

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

        Operand get_aop(unsigned char type, bool big=false){
            if (type == MD || type == MX){ // Memory Index
                int addr = offset_address(type, read_number(2));
                int value = state.memory[addr];
                return Operand {value, addr, "Mx"};
            }
            else if (type == CV){ // Constant Value
                if (big) return Operand {read_number(2), 0, "CV"};
                return Operand {read_byte(), 0, "CV"};
            }
            return Operand {loadRegister(type), type, "REG"};
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

        void execute(){
            while (1){
                unsigned char opcode = read_byte();
                printf("Opcode: %s\n", getHex(opcode).c_str());
                unsigned char* types;
                unsigned char src_r, src1_r, src2_r, tar_r;
                bool big, flip;
                int result;
                Operand src, src1, src2, tar;
                switch (opcode){
                    case 0x00: // inc <tar>(8R, 16R, Mx)
                        get_types(1, types);
                        tar_r = types[0];
                        tar = get_aop(tar_r);
                        result = tar.value + 1;
                        store(tar, result);
                        setFlag("S0", 1, result == 0); // zero flag
                        break;
                    case 0x01: // dec <tar>(8R, 16R, Mx)
                        get_types(1, types);
                        tar_r = types[0];
                        tar = get_aop(tar_r);
                        result = tar.value - 1;
                        store(tar, result);
                        setFlag("S0", 1, result == 0); // zero flag
                        break;
                    case 0x02: // add <src>(CV, 8R, Mx) <tar>(8R, Mx)
                        get_types(2, types);
                        src_r = types[0], tar_r = types[1];
                        big = is_2byte_register(tar_r);
                        src = get_aop(src_r, big);
                        tar = get_aop(tar_r);
                        result = src.value + tar.value;
                        store(tar, result);
                        setFlag("S0", 0, result > 0xFF);
                        setFlag("S0", 1, result == 0); // zero flag
                        break;
                    case 0x03: // addc <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
                        get_types(2, types);
                        src_r = types[0], tar_r = types[1];
                        big = is_2byte_register(tar_r);
                        src = get_aop(src_r, big);
                        tar = get_aop(tar_r);
                        result = src.value + tar.value + getFlag("S0", 0);
                        store(tar, result);
                        setFlag("S0", 0, result > 0xFF);
                        setFlag("S0", 1, result == 0); // zero flag
                        break;
                    case 0x04: // sub <src>(CV, 8R, Mx) <tar>(8R, Mx)
                        get_types(2, types);
                        src_r = types[0], tar_r = types[1];
                        big = is_2byte_register(tar_r);
                        src = get_aop(src_r, big);
                        tar = get_aop(tar_r);
                        result = tar.value - src.value;
                        store(tar, result);
                        setFlag("S0", 0, src.value > tar.value);
                        setFlag("S0", 1, result == 0); // zero flag
                        break;
                    case 0x05: // subb <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
                        get_types(2, types);
                        src_r = types[0], tar_r = types[1];
                        big = is_2byte_register(tar_r);
                        src = get_aop(src_r, big);
                        tar = get_aop(tar_r);
                        result = tar.value - (src.value + getFlag("S0", 0));
                        store(tar, result);
                        setFlag("S0", 0, (src.value + getFlag("S0", 0)) > tar.value);
                        setFlag("S0", 1, result == 0); // zero flag
                        break;
                    // TODO: implement bit shifting instructions and other logic ops
                    case 0x0B: // or <src>(CV, 8R, Mx) <tar>(8R, Mx)
                        get_types(2, types);
                        src_r = types[0], tar_r = types[1];
                        big = is_2byte_register(tar_r);
                        src = get_aop(src_r, big);
                        tar = get_aop(tar_r);
                        result = src.value | tar.value;
                        store(tar, result);
                        setFlag("S0", 1, result == 0); // zero flag
                        break;
                    case 0x10: // jmp <src>(CV) <tar>(Mx) <src-aop> <tar-aop>
                        get_types(2, types);
                        src_r = types[0], tar_r = types[1];
                        src = get_aop(src_r);
                        tar = get_aop(tar_r);
                        //printf("src: %x target: %d\n", src.value, tar.target);
                        flip = BIT_CHECK(src.value, 7);
                        BIT_SET(src.value, 7, 0);
                        bool flagSet;
                        if (src.value <= 7) flagSet = getFlag("S0", src.value);
                        else getFlag("S1", src.value);
                        flagSet ^= flip;
                        if (flagSet) state.registers.PC = tar.target;
                        break;

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
    //  add    CV    R0    10                    ; add 10 to R0
        0x02, 0x0F, 0x00, 0x0C,
    //  dec    R0
        0x01, 0x00,
    //  jmp    CV    MD  %0b10000001  0x0004     ; jump to address 0x0004 if not zero
    //                                           ; branching if flag bit 1 is *not* set
        0x10, 0x0F, 0x10, 0b10000001, 0x04, 0x00,
    //  or     CV    S0    %00010000
        0x0B, 0x0F, 0x08, 0b00010000 // set halt bit
    };
    State state {Registers {}, memory, 0x100};
    Emulator emu {state};
    emu.execute();
    std::cout << dumpMem(state.memory, state.size) << "\n";
    return 0;
}