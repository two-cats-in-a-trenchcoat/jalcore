#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <vector>

#define SDL_MAIN_HANDLED
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "imgui_stdlib.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "emulator.h"

//#define DEBUG
//#define PERF_LOG

char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::string getHex(char c){
    std::string r = "";
    r += hex[(c & 0xF0) >> 4];
    r += hex[c & 0xF];
    return r;
}

std::string dumpMem(uint8_t *memory, unsigned size){
    std::string r = "";
    for (unsigned i = 0; i < size; i++){
        const char ch = memory[i];
        r += getHex(ch);
    }
    return r;
}


const uint8_t PC = 0x0A;
const uint8_t CV8 = 0x0F;
const uint8_t CV16 = 0x10;
const uint8_t MD = 0x11;
const uint8_t MX = 0x12;
const uint8_t MV = 0x13;

JalcoreCPU::JalcoreCPU(){}

JalcoreCPU::~JalcoreCPU(){}

void JalcoreCPU::reset(){
    registers = Registers {};
    cycle_total = 0;
    cycle_counter = 0;
}

void JalcoreCPU::ConnectBus(Bus *b){
    bus = b;
}

void JalcoreCPU::write(uint16_t addr, uint8_t data){
    bus->write(addr, data);
}
uint8_t JalcoreCPU::read(uint16_t addr){
    return bus->read(addr);
}

uint8_t JalcoreCPU::read_byte(){
    uint8_t byte = read(registers.PC);
    registers.PC += 1;
    return byte;
}

unsigned JalcoreCPU::offset_address(uint8_t type, unsigned address){
    if (type == MX) return registers.IX + address;
    return address;
}

unsigned JalcoreCPU::read_number(unsigned numBytes){
    uint8_t b1 = read_byte();
    uint8_t b2 = read_byte();
    unsigned result = (b2 << 8) + (b1);
    //printf("%s %s -> %d\n", getHex(b1).c_str(), getHex(b2).c_str(), result);
    return result;
}

bool JalcoreCPU::is_2byte_register(uint8_t type){
    return (type >= 0x0A && type <= 0x0E) or type == 0x14;
}

void JalcoreCPU::get_types(unsigned amount, uint8_t* result){
    for (unsigned i = 0; i < amount; i++){
        result[i] = read_byte();
    }
}

unsigned JalcoreCPU::loadRegister(uint8_t type){
    switch (type){
        case 0x00: // R0
            return registers.R0;
        case 0x01: // R1
            return registers.R1;
        case 0x02: // R2
            return registers.R2;
        case 0x03: // R3
            return registers.R3;
        case 0x04: // R4
            return registers.R4;
        case 0x05: // R5
            return registers.R5;
        case 0x06: // R6
            return registers.R6;
        case 0x07: // R7
            return registers.R7;
        
        case 0x08: // S0
            return registers.S0;
        case 0x09: // S1
            return registers.S1;
        
        case 0x0A: // PC
            return registers.PC;
        case 0x0B: // SP
            return registers.SP;
        case 0x0C: // IX
            return registers.IX;
    }
    printf("Invalid type %d\n", type);
    return 0;
}

void JalcoreCPU::setRegister(uint8_t type, unsigned value){
    switch (type){
        case 0x00: // R0
            registers.R0 = value;
            break;
        case 0x01: // R1
            registers.R1 = value;
            break;
        case 0x02: // R2
            registers.R2 = value;
            break;
        case 0x03: // R3
            registers.R3 = value;
            break;
        case 0x04: // R4
            registers.R4 = value;
            break;
        case 0x05: // R5
            registers.R5 = value;
            break;
        case 0x06: // R6
            registers.R6 = value;
            break;
        case 0x07: // R7
            registers.R7 = value;
            break;
        
        case 0x08: // S0
            registers.S0 = value;
            BIT_SET(registers.S0, 7, 1); // hardwired 1
            BIT_SET(registers.S0, 6, 0); // hardwired 0
            break;
        case 0x09: // S1
            registers.S1 = value;
            break;
        
        case 0x0A: // PC
            registers.PC = value;
            break;
        case 0x0B: // SP
            registers.SP = value;
            break;
        case 0x0C: // IX
            registers.IX = value;
            break;
    }
}

Operand JalcoreCPU::get_aop(uint8_t type){
    if (type == MD || type == MX){ // Memory Index
        unsigned addr = offset_address(type, read_number(2));
        unsigned value = read(addr);
        return Operand {value, addr, "Mx"};
    }
    else if (type == MV){
        uint8_t _type = read_byte();
        Operand addr = get_aop(_type);
        addr.type = "Mx";
        addr.target = addr.value;
        return addr;
    }
    else if (type == CV8) return Operand {read_byte(), 0, "CV8"}; // 8 bit Constant
    else if (type == CV16) return Operand {read_number(2), 0, "CV16"}; // 16 bit Constant
    
    return Operand {loadRegister(type), type, "REG"};
}

void JalcoreCPU::get_params(unsigned amount, Operand *result){
    uint8_t types[amount];
    get_types(amount, types);
    for (unsigned i = 0; i < amount; i++){
        //printf("%d\n", i);
        Operand p = get_aop(types[i]);
        result[i] = p;
    }
}

void JalcoreCPU::store(Operand operand, unsigned value){
    if (operand.type == "Mx"){
        write(operand.target, value);
    }
    else if (operand.type == "REG"){
        setRegister(operand.target, value);
    }
}

bool JalcoreCPU::getFlag(FlagRegister target, unsigned bit){
    if (target == S0) return BIT_CHECK(registers.S0, bit);
    if (target == S1) return BIT_CHECK(registers.S1, bit);
    throw std::runtime_error("Unexpected FlagRegister target");
}

void JalcoreCPU::setFlag(FlagRegister target, unsigned bit, bool value){
    if (target == S0) BIT_SET(registers.S0, bit, value);
    if (target == S1) BIT_SET(registers.S1, bit, value);
}

// Instruction definitions

void JalcoreCPU::op_inc(){
    // inc <tar>(8R, 16R, Mx)
    Operand params[1];
    get_params(1, params);
    Operand tar = params[0];
    unsigned result = tar.value + 1;
    store(tar, result);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_dec(){
    // dec <tar>(8R, 16R, Mx)
    Operand params[1];
    get_params(1, params);
    Operand tar = params[0];
    unsigned result = tar.value - 1;
    store(tar, result);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_add(){
    // add <src>(CV, 8R, Mx) <tar>(8R, Mx)
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = src.value + tar.value;
    store(tar, result);
    setFlag(S0, 0, result > 0xFF);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_addc(){
    // addc <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = src.value + tar.value + getFlag(S0, 0);
    store(tar, result);
    setFlag(S0, 0, result > 0xFF);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_sub(){
    // sub <src>(CV, 8R, Mx) <tar>(8R, Mx)
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = tar.value - src.value;
    store(tar, result);
    setFlag(S0, 0, src.value > tar.value);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_subb(){
    // subb <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = tar.value - (src.value + getFlag(S0, 0));
    store(tar, result);
    setFlag(S0, 0, (src.value + getFlag(S0, 0)) > tar.value);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_rol(){
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
    setFlag(S0, 1, result == 0); // zero flag
    setFlag(S0, 0, carry); // carry bit
}

void JalcoreCPU::op_rolc(){
    // rolc <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
    // (rotate left through carry)
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = src.value;
    bool oldcarry = getFlag(S0, 1);
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
    setFlag(S0, 1, result == 0); // zero flag
    setFlag(S0, 0, carry); // carry bit
}

void JalcoreCPU::op_ror(){
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
    setFlag(S0, 1, result == 0); // zero flag
    setFlag(S0, 0, carry); // carry bit
}

void JalcoreCPU::op_rorc(){
    // rorc <src>(CV, 8R, Mx) <tar>(8R, Mx) <src-aop> <tar-aop>
    // (rotate right through carry)
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = src.value;
    bool oldcarry = getFlag(S0, 1);
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
    setFlag(S0, 1, result == 0); // zero flag
    setFlag(S0, 0, carry); // carry bit
}

void JalcoreCPU::op_and(){
    // and <src>(CV, 8R, Mx) <tar>(8R, Mx)
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = src.value & tar.value;
    store(tar, result);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_or(){
    // or <src>(CV, 8R, Mx) <tar>(8R, Mx)
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = src.value | tar.value;
    store(tar, result);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_xor(){
    // xor <src>(CV, 8R, Mx) <tar>(8R, Mx)
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    unsigned result = src.value ^ tar.value;
    store(tar, result);
    setFlag(S0, 1, result == 0); // zero flag
}

void JalcoreCPU::op_cmp(){
    // cmp <src1>(CV, 8R, 16R, Mx) <src2>(CV, 8R, 16R, Mx) <src1-aop> <src2-aop>
    Operand params[2];
    get_params(2, params);
    Operand src1 = params[0];
    Operand src2 = params[1];
    setFlag(S0, 0, src1.value > src2.value); // carry flag
    setFlag(S0, 1, src1.value == src2.value); // zero flag
}

void JalcoreCPU::op_push(){
    // push <src>(CV, 8R, 16R, Mx)
    Operand params[1];
    get_params(1, params);
    Operand src = params[0];
    registers.SP++;
    store(Operand { 0, registers.SP, "Mx" }, src.value);
    if (src.type == "CV16" || is_2byte_register(src.target)){
        registers.SP++;
        store(Operand { 0, registers.SP, "Mx" }, src.value >> 8);
    }
}

void JalcoreCPU::op_pop(){
    // pop <tar>(8R, 16R, Mx)
    Operand params[1];
    get_params(1, params);
    Operand tar = params[0];
    unsigned result = 0;
    result += read(registers.SP);
    registers.SP--;
    if (is_2byte_register(tar.target)){
        result <<= 8;
        result += read(registers.SP);
        registers.SP--;
    }
    store(tar, result);
}

void JalcoreCPU::op_jmp(){
    // jmp <src>(CV) <tar>(Mx) <src-aop> <tar-aop>
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    //printf("src: %x target: %d\n", src.value, tar.target);
    bool flip = BIT_CHECK(src.value, 7);
    BIT_SET(src.value, 7, 0);
    bool flagSet;
    if (src.value <= 7) flagSet = getFlag(S0, src.value);
    else flagSet = getFlag(S1, src.value-7);
    flagSet ^= flip;
    if (flagSet) registers.PC = tar.target;
}

void JalcoreCPU::op_jsr(){
    // jsr <src>(CV) <tar>(Mx) <src-aop> <tar-aop>
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    //printf("src: %x target: %d\n", src.value, tar.target);
    bool flip = BIT_CHECK(src.value, 7);
    BIT_SET(src.value, 7, 0);
    bool flagSet;
    if (src.value <= 7) flagSet = getFlag(S0, src.value);
    else flagSet = getFlag(S1, src.value-7);
    flagSet ^= flip;
    if (flagSet){
        // push PC to stack before jumping
        registers.SP++;
        write(registers.SP, registers.PC);
        registers.SP++;
        write(registers.SP, registers.PC >> 8);
        registers.PC = tar.target;
    }
}

void JalcoreCPU::op_mov(){
    // mov <src>(CV, 8R, 16R, Mx) <tar>(8R, 16R, Mx) <src-aop> <tar-aop>
    Operand params[2];
    get_params(2, params);
    Operand src = params[0];
    Operand tar = params[1];
    //printf("new val: %d address: %#x  - ", src.value, tar.target);
    store(tar, src.value);
}

void JalcoreCPU::op_ret(){
    unsigned result = 0;
    result += read(registers.SP);
    registers.SP--;
    result <<= 8;
    result += read(registers.SP);
    registers.SP--;
    registers.PC = result;
}

void JalcoreCPU::op_rdw(){
    // Tell the Picture Processing Unit to redraw the display
    bus->ppu.Redraw();
}

// Execution loop

void JalcoreCPU::runtime_loop(){
    while (bus->guiRunning) {
        execute();
    }
}

void JalcoreCPU::execute(){
    while (bus->cpuRunning){
        auto cycle_start = std::chrono::high_resolution_clock::now();
        uint8_t opcode = read_byte();
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
            case 0x11: op_jsr(); break;
            case 0x12: break; // nop
            case 0x13: op_mov(); break;
            case 0x14: op_ret(); break;
            case 0x15: op_rdw(); break;
            // TODO: implement bit shifting instructions and other logic ops
            default:
                printf("Unexpected opcode %#x, PC: %d\n", opcode, registers.PC);
                printf("SP: %d\n", registers.SP);
                printf("IX: %d\n", registers.IX);
                bus->cpuRunning = false;
                return;
        }
        auto cycle_end = std::chrono::high_resolution_clock::now();
        cycle_counter += 1;
        instruction_counts[opcode] += 1;
        std::chrono::duration<double> cycle_time = cycle_end - cycle_start;
        cycle_total += cycle_time.count();
        instruction_times[opcode] += cycle_time.count();

        // halt check
        if (getFlag(S0, 4)){
            printf("Halted\n");
            //printf(memory.hex())
            bus->cpuRunning = false;
        }
    }
}

JalcorePPU::JalcorePPU(){}

JalcorePPU::~JalcorePPU(){}

void JalcorePPU::ConnectBus(Bus *b){
    bus = b;
    vram = bus->ram + vram_offset;
}

std::chrono::duration<double> JalcorePPU::UpdateDisplay(){
    //if (!bus->cpuRunning) return std::chrono::duration<double>{0};
    auto start = std::chrono::high_resolution_clock::now();
    uint8_t value;
    unsigned pos;
    uint8_t r, g, b;

    for (short y = 0; y < WINDOW_WIDTH; y++){
        for (short x = 0; x < WINDOW_WIDTH; x++){
            pos = ((128 * y) + x);
            value = vram[pos];
            
            r = ((value >> 5) / 7.0) * 255;
            g = (((value >> 2) & 0b111) / 7.0) * 255;
            b = ((value & 0b11) / 3.0) * 255;

            pixels[pos] = (r << 16) + (b << 8) + g;

        }
    }
    int res = SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * sizeof(Uint32));
    if (res != 0){
        printf("Texture Error: %d\n", res);
    }
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    auto finish = std::chrono::high_resolution_clock::now();
    return finish - start;
}

void JalcorePPU::Redraw(){
    #ifdef DEBUG
    printf("Redraw... ");
    std::cout << dumpMem(vram, 0x4000);
    #endif
    
    auto elapsed = UpdateDisplay();
    redraw_total += elapsed.count();
    redraw_count += 1;
    
    #ifdef PERF_LOG
    std::cout << "Average Display Update Time: " << redraw_total/redraw_count << " s\n";
    #endif
    
    #ifdef DEBUG
    printf("Redraw complete\n");
    #endif
}

void JalcorePPU::Prepare(){
    // prepare main display window
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0){
        printf("Error: %s\n", SDL_GetError());
        bus->cpuRunning = false;
        return;
    }
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH*SCALE, WINDOW_WIDTH*SCALE, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Jalcore v0.1.0");
    SDL_RenderSetScale(renderer, SCALE, SCALE);
    texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_WIDTH
    );
    memset(pixels, 0, WINDOW_WIDTH * WINDOW_WIDTH * sizeof(Uint32));
}

void JalcorePPU::SDL_eventloop(){
    // initialise ImGui window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    
    SDL_Window *im_window = SDL_CreateWindow("Controls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, window_flags);
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(im_window);
    SDL_GL_MakeCurrent(im_window, gl_context);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(im_window, gl_context);
    ImGui_ImplOpenGL2_Init();
    
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static std::string rom_location;
    while (bus->guiRunning){
        while (SDL_PollEvent(&event)){
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                bus->cpuRunning = false;
                bus->guiRunning = false;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE){
                bus->cpuRunning = false;
                bus->guiRunning = false;
            }
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::Begin("Statistics", NULL/*, ImGuiWindowFlags_NoResize*/);

            ImGui::Text("This panel displays realtime statistics for the CPU");
            
            ImGui::Text("CPU status: %s", bus->cpuRunning ? "Running" : "Halted");
            ImGui::Text("Display Redraws: %llu\n", redraw_count);
            ImGui::Text("Time spent Drawing: %.3fs\n", redraw_total);
            ImGui::Text("Average Display Update Time: %.3fms\n", (redraw_total/redraw_count)*1000);
            //ImGui::Text("Average Framerate: %.3f\n\n", redraw_count / (bus->exec_time.count() / (double)1000000000));
            
            ImGui::Text("Cycles: %llu\n", bus->cpu.cycle_counter);
            ImGui::Text("Time spent on cycles: %.3fs\n", bus->cpu.cycle_total);
            ImGui::Text("Average Cycle Time: %.3fns\n", (bus->cpu.cycle_total/bus->cpu.cycle_counter)*1000*1000*1000);
            ImGui::Text("Instructions per second: %.3fmhz\n\n", (1 / (bus->cpu.cycle_total/bus->cpu.cycle_counter)) / 1000 / 1000);
            
            if (ImGui::Button("Toggle CPU Status")){
                bus->cpuRunning = !bus->cpuRunning;
            }

            if (ImGui::Button("Reset CPU")){
                bus->cpu.reset();
            }

            ImGui::InputText("ROM filepath", &rom_location, 0, (ImGuiInputTextCallback)__null, (void *)__null);
            ImGui::SameLine();
            if (ImGui::Button("Load ROM")){
                FILE* file = fopen(rom_location.c_str(), "r+");
                if (file == NULL) {
                    printf("File is null");
                }
                else {
                    bus->loadRom(file);
                    fclose(file);
                }
            }

            ImGui::Text("ImGUI average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            
            ImGui::Text("Instruction times:");
            
            for (size_t i = 0; i < bus->cpu.instruction_counts.size(); i++){
                //printf("%lld ", i);
                //printf("\n");
                ImGui::Text("0x%llx  :  %.3fns", i, (bus->cpu.instruction_times[i] / bus->cpu.instruction_counts[i])*1000*1000*1000);
            }
            
            ImGui::End();
        }
        
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(im_window);

        SDL_Delay(floor(abs((1000/FRAMERATE) - io.DeltaTime)));
    }

    // teardown work
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyWindow(im_window);
    SDL_Quit();
}

Bus::Bus(){
    cpu.ConnectBus(this);
    ppu.ConnectBus(this);
}

Bus::~Bus(){}

void Bus::write(uint16_t addr, uint8_t data){
    ram[addr] = data;
}

uint8_t Bus::read(uint16_t addr){
    return ram[addr];
}

void Bus::loadRom(FILE* rom){
    fseek(rom, 0, SEEK_END);
    unsigned long size = ftell(rom);
    fseek(rom, 0, SEEK_SET);
    if (size > ramSize){
        size = ramSize;
        printf("Rom cut off from %ld to %lld", size, ramSize);
    }
    fread(ram, sizeof(uint8_t), size, rom);
}

void Bus::Startup(){
    cpuRunning = true;
    guiRunning = true;
    auto exec_start = std::chrono::high_resolution_clock::now();
    ppu.Prepare();
    cpu_thread = std::thread(&JalcoreCPU::runtime_loop, &cpu);
    ppu.SDL_eventloop();
    guiRunning = false;
    cpuRunning = false;
    cpu_thread.join();
    std::chrono::duration<int64_t, std::nano> exec_time = std::chrono::high_resolution_clock::now() - exec_start;


    
    printf("Display Redraws: %llu\n", ppu.redraw_count);
    printf("Time spent Drawing: %.3fs\n", ppu.redraw_total);
    printf("Average Display Update Time: %.3fms\n", (ppu.redraw_total/ppu.redraw_count)*1000);
    printf("Average Framerate: %.3f\n\n", ppu.redraw_count / (exec_time.count() / (double)1000000000));
    
    printf("Cycles: %llu\n", cpu.cycle_counter);
    printf("Time spent on cycles: %.3fs\n", cpu.cycle_total);
    printf("Average Cycle Time: %.3fns\n", (cpu.cycle_total/cpu.cycle_counter)*1000*1000*1000);
    printf("Instructions per second: %.3fmhz\n\n", (1 / (cpu.cycle_total/cpu.cycle_counter)) / 1000 / 1000);
    
    
    
    printf("Total execution time: %.3fs\n", exec_time.count() / (double)1000000000);
}


int main(int argc, char* argv[]){
    Bus bus;
    if (argc >= 2){
        FILE* file = fopen(argv[1], "r+");
        if (file == NULL) {
            printf("File is null");
            return 1;
        }
        bus.loadRom(file);
        fclose(file);

    }
    bus.Startup();
    // dump to core.bin
    FILE* file = fopen("core.bin", "w+");
    if (file != NULL){
        fwrite(bus.ram, sizeof(uint8_t), bus.ramSize, file);
        fclose(file);
        printf("Memory dumped to 'core.bin'\n");
    }
    return 0;
}