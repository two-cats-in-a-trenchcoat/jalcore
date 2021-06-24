#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <chrono>
#include <vector>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#define SCALE 4
#define WINDOW_WIDTH 128

//#define DEBUG
#define PERF_LOG

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

std::string dumpMem(unsigned char *memory, unsigned size){
    std::string r = "";
    for (unsigned i = 0; i < size; i++){
        const char ch = memory[i];
        r += getHex(ch);
    }
    return r;
}

struct Operand {
    unsigned value;
    unsigned target;
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
    unsigned short RF = 1024; 
    // CV, MD, MX
};

const unsigned char PC = 0x0A;
const unsigned char CV8 = 0x0F;
const unsigned char CV16 = 0x10;
const unsigned char MD = 0x11;
const unsigned char MX = 0x12;
const unsigned char MV = 0x13;

struct State {
    Registers registers;
    unsigned char *memory;
    unsigned size;
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

        unsigned offset_address(unsigned char type, unsigned address){
            if (type == MX) return state.registers.IX + address;
            return address;
        }

        unsigned read_number(unsigned numBytes){
            unsigned char b1 = read_byte();
            unsigned char b2 = read_byte();
            unsigned result = (b2 << 8) + (b1);
            //printf("%s %s -> %d\n", getHex(b1).c_str(), getHex(b2).c_str(), result);
            return result;
        }

        bool is_2byte_register(unsigned char type){
            return (type >= 0x0A && type <= 0x0E) or type == 0x14;
        }

        void get_types(unsigned amount, unsigned char* result){
            for (unsigned i = 0; i < amount; i++){
                result[i] = read_byte();
            }
        }

        unsigned loadRegister(unsigned char type){
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
                case 0x14:
                    return state.registers.RF;
            }
            printf("Invalid type %d\n", type);
            return 0;
        }

        void setRegister(unsigned char type, unsigned value){
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
                    BIT_SET(state.registers.S0, 7, 1); // hardwired 1
                    BIT_SET(state.registers.S0, 6, 0); // hardwired 0
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
                case 0x14:
                    state.registers.RF = value;
                    break;
            }
        }

        Operand get_aop(unsigned char type){
            if (type == MD || type == MX){ // Memory Index
                unsigned addr = offset_address(type, read_number(2));
                unsigned value = state.memory[addr];
                return Operand {value, addr, "Mx"};
            }
            else if (type == MV){
                unsigned char _type = read_byte();
                Operand addr = get_aop(_type);
                addr.type = "Mx";
                addr.target = addr.value;
                return addr;
            }
            else if (type == CV8) return Operand {read_byte(), 0, "CV8"}; // 8 bit Constant
            else if (type == CV16) return Operand {read_number(2), 0, "CV16"}; // 16 bit Constant
            
            return Operand {loadRegister(type), type, "REG"};
        }

        void get_params(unsigned amount, Operand *result){
            unsigned char types[amount];
            get_types(amount, types);
            for (unsigned i = 0; i < amount; i++){
                //printf("%d\n", i);
                Operand p = get_aop(types[i]);
                result[i] = p;
            }
        }

        void store(Operand operand, unsigned value){
            if (operand.type == "Mx"){
                state.memory[operand.target] = value;
            }
            else if (operand.type == "REG"){
                setRegister(operand.target, value);
            }
        }

        bool getFlag(std::string target, unsigned bit){
            if (target == "S0") return BIT_CHECK(state.registers.S0, bit);
            if (target == "S1") return BIT_CHECK(state.registers.S1, bit);
        }

        void setFlag(std::string target, unsigned bit, bool value){
            if (target == "S0") BIT_SET(state.registers.S0, bit, value);
            if (target == "S1") BIT_SET(state.registers.S1, bit, value);
        }

        // Instruction definitions

        void op_inc(){
            // inc <tar>(8R, 16R, Mx)
            Operand params[1];
            get_params(1, params);
            Operand tar = params[0];
            unsigned result = tar.value + 1;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_dec(){
            // dec <tar>(8R, 16R, Mx)
            Operand params[1];
            get_params(1, params);
            Operand tar = params[0];
            unsigned result = tar.value - 1;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_add(){
            // add <src>(CV, 8R, Mx) <tar>(8R, Mx)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value + tar.value;
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
            unsigned result = src.value + tar.value + getFlag("S0", 0);
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
            unsigned result = tar.value - src.value;
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
            unsigned result = tar.value - (src.value + getFlag("S0", 0));
            store(tar, result);
            setFlag("S0", 0, (src.value + getFlag("S0", 0)) > tar.value);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_rol(){
            // rol <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
            // (rotate left)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value;
            bool carry;
            if (src.type == "CV8"){
                carry = result >> 7;
                result  = (result << 1) | carry;
            }
            else {
                carry = result >> 15;
                result  = (result << 1) | carry;
            }
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
            setFlag("S0", 0, carry); // carry bit
        }

        void op_rolc(){
            // rolc <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
            // (rotate left through carry)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value;
            bool oldcarry = getFlag("S0", 1);
            bool carry;
            if (src.type == "CV8"){
                carry = result >> 7;
                result  = (result << 1) | oldcarry;
            }
            else {
                carry = result >> 15;
                result  = (result << 1) | oldcarry;
            }
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
            setFlag("S0", 0, carry); // carry bit
        }

        void op_ror(){
            // ror <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
            // (rotate right)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value;
            bool carry;
            if (src.type == "CV8"){
                carry = result & 1;
                result  = (result >> 1) | (carry << 7);
            }
            else {
                carry = result & 1;
                result  = (result >> 1) | (carry << 15);
            }
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
            setFlag("S0", 0, carry); // carry bit
        }

        void op_rorc(){
            // rorc <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
            // (rotate right through carry)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value;
            bool oldcarry = getFlag("S0", 1);
            bool carry;
            if (src.type == "CV8"){
                carry = result & 1;
                result  = (result << 1) | (oldcarry << 7);
            }
            else {
                carry = result & 1;
                result  = (result << 1) | (oldcarry << 15);
            }
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
            setFlag("S0", 0, carry); // carry bit
        }

        void op_and(){
            // and <src>(CV, 8R, Mx) <tar>(8R, Mx)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value & tar.value;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_or(){
            // or <src>(CV, 8R, Mx) <tar>(8R, Mx)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value | tar.value;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_xor(){
            // xor <src>(CV, 8R, Mx) <tar>(8R, Mx)
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            unsigned result = src.value ^ tar.value;
            store(tar, result);
            setFlag("S0", 1, result == 0); // zero flag
        }

        void op_cmp(){
            // cmp <src1>(CV, 8R, 16R, Mx) <src2>(CV, 8R, 16R, Mx) <src1-aop> <src2-aop>
            Operand params[2];
            get_params(2, params);
            Operand src1 = params[0];
            Operand src2 = params[1];
            setFlag("S0", 0, src1.value > src2.value); // carry flag
            setFlag("S0", 1, src1.value == src2.value); // zero flag
        }

        void op_push(){
            // push <src>(CV, 8R, 16R, Mx)
            Operand params[1];
            get_params(1, params);
            Operand src = params[0];
            store(Operand { 0, state.registers.SP + 1U, "Mx" }, src.value);
            state.registers.SP++;
            if (src.type == "CV16" || is_2byte_register(src.target)){
                store(Operand { 0, state.registers.SP + 2U, "Mx" }, src.value >> 8);
                state.registers.SP++;
            }
        }

        void op_pop(){
            // pop <tar>(8R, 16R, Mx)
            Operand params[1];
            get_params(1, params);
            Operand tar = params[0];
            unsigned result;
            result += state.memory[state.registers.SP];
            state.registers.SP--;
            if (is_2byte_register(tar.target)){
                result <<= 8;
                result += state.memory[state.registers.SP];
                state.registers.SP--;
            }
            store(tar, result);
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

        void op_mov(){
            // mov <src>(CV, 8R, 16R, Mx) <tar>(8R, 16R, Mx) <src-aop> <tar-aop>
            Operand params[2];
            get_params(2, params);
            Operand src = params[0];
            Operand tar = params[1];
            //printf("new val: %d address: %#x  - ", src.value, tar.target);
            store(tar, src.value);
        }


        std::chrono::duration<double> UpdateDisplay(SDL_Renderer *renderer){
            auto start = std::chrono::high_resolution_clock::now();
            const int startAddr = 0xC000;
            unsigned char value;
            unsigned pos;
            uint8_t p_r = 0, p_g = 0, p_b = 0;
            uint8_t r, g, b;

            for (short y = 0; y < WINDOW_WIDTH; y++){
                for (short x = 0; x < WINDOW_WIDTH; x++){
                    pos = ((128 * y) + x);
                    value = state.memory[startAddr + pos];
                    
                    r = ((value >> 5) / 7.0) * 255;
                    g = (((value >> 2) & 0b111) / 7.0) * 255;
                    b = ((value & 0b11) / 3.0) * 255;
                    
                    if (r != p_r or g != p_g or b != p_b){
                        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                        p_r = r, p_g = g, p_b = b;
                    }
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
            SDL_RenderPresent(renderer);
            auto finish = std::chrono::high_resolution_clock::now();
            return finish - start;
        }


        // Execution loop

        void execute(){
            // initialise window
            SDL_Event event;
            SDL_Renderer *renderer;
            SDL_Window *window;
            SDL_Init(SDL_INIT_VIDEO);
            SDL_CreateWindowAndRenderer(WINDOW_WIDTH*SCALE, WINDOW_WIDTH*SCALE, 0, &window, &renderer);
            SDL_RenderSetScale(renderer, SCALE, SCALE);
            
            int counter = 0;
            double total;
            int redraw_cout;

            while (1){
                unsigned char opcode = read_byte();
                #ifdef DEBUG
                printf("Opcode: %s\n", getHex(opcode).c_str());
                #endif
                switch (opcode){
                    case 0x00: op_inc(); break;
                    case 0x01: op_dec(); break;
                    case 0x02: op_add(); break;
                    case 0x03: op_addc(); break;
                    case 0x04: op_sub(); break;
                    case 0x05: op_subb(); break;
                    case 0x06: op_rol(); break;
                    case 0x07: op_rolc(); break;
                    case 0x08: op_ror(); break;
                    case 0x09: op_rorc(); break;
                    case 0x0A: op_and(); break;
                    case 0x0B: op_or(); break;
                    case 0x0C: op_xor(); break;
                    case 0x0D: op_cmp(); break;
                    case 0x0E: op_push(); break;
                    case 0x0F: op_pop(); break;
                    case 0x10: op_jmp(); break;
                    //case 0x11: op_jsr(); break;
                    case 0x12: break; // nop
                    case 0x13: op_mov(); break;
                    // TODO: implement bit shifting instructions and other logic ops
                    default:
                        printf("Unexpected opcode %#x, PC: %d", opcode, state.registers.PC);
                        exit(1);
                }

                // halt check
                if (getFlag("S0", 4)){
                    printf("Halted\n");
                    //printf(memory.hex())
                    break;
                }
                if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
                    break;

                counter += 1;
                if (counter % state.registers.RF == 0){
                    counter = 1;
                    auto elapsed = UpdateDisplay(renderer);
                    total += elapsed.count();
                    redraw_cout += 1;
                    #ifdef PERF_LOG
                    std::cout << "Average Display Update Time: " << total/redraw_cout << " s\n";
                    #endif
                }
            }
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
        }
};


int main(int argc, char* argv[]){
    unsigned char memory[0xFFFF] = {};
    if (argc >= 2){
        FILE* file = fopen(argv[1], "r+");
        if (file == NULL) {
            printf("File is null");
            return 1;
        }
        fseek(file, 0, SEEK_END);
        long int size = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (size > sizeof(memory))
        {
            size = sizeof(memory);
        }
        fread(memory, sizeof(unsigned char), size, file);
        fclose(file);
    }
    State state {Registers {}, memory, 0xFFFF};
    Emulator emu {state};
    emu.execute();
    // dump to core.bin
    FILE* file = fopen("core.bin", "w+");
    if (file != NULL){
        fwrite(state.memory, sizeof(unsigned char), emu.state.size, file);
        fclose(file);
        printf("Memory dumped to 'core.bin'\n");
    }
    return 0;
}