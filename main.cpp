#include <iostream>
#include <fstream>
#include <bitset>
#include <map>
#include <cassert>

using namespace std;

typedef unsigned int u32;
typedef unsigned char u8;

u8 M[1048576] = {0};
u32 x[32] = {0};
u32 pc = 0;

void printu32_2(const u32 thenum) {
    int num = thenum;
    bool pt[32] = {0};
    for (int i = 0; i < 32; ++i) {
        //cout << (num >> (31 - i) & 1);
    }
    //cout << endl;
}


void load();

u32 sext(int code, int len) {
    return code << 32 - len >> 32 - len;
}

u32 getbit(u32 code, int ptr) {
    return code & 1 << ptr;
}

u32 getbit(u32 code, int high, int low) {
    return code >> low << (31 - high + low) >> (31 - high);
}

void parsecode(u32 code);

int main() {
//    freopen("../basicopt1.data", "r", stdin);
//    freopen("../sjmout.txt", "w", stdout);
    load();
    //TODO 解析指令时反序
    u32 instruction;
    while((instruction = M[pc] | M[pc+1] << 8 | M[pc+2] << 16 | M[pc+3] << 24) != 0x0ff00513){
//        //cout << hex << pc << '\t';
//        //cout << instruction << '\t';
        if(x[15] >= 800000000) exit(0);
        parsecode(instruction);
        pc+=4;
    }
    cout << dec << (x[10] & 0b11111111u) << endl;
}

void load() {
    u32 memptr = 0;
    string input;
    string input2, input3, input4;
    while (cin >> input) {
        if (input[0] == '@')
            memptr = stol(input.substr(1), 0, 16);
        else
            M[memptr++] = stol(input, 0, 16);
    }
}

void parsecode(u32 code) {
    static const map<u32, char> opcodeTypeMap = {{0b0110111u, 'U'},
                                                 {0b0010111u, 'U'},
                                                 {0b1101111u, 'J'},
                                                 {0b1100111u, 'I'},
                                                 {0b1100011u, 'B'},
                                                 {0b0000011u, 'I'},
                                                 {0b0100011u, 'S'},
                                                 {0b0010011u, 'I'},
                                                 {0b0110011u, 'R'},
    };
    const u32 opcode = code & 0b1111111u;
    const u32 rd = code >> 7 & 0b11111u;
    const u32 func3 = code >> 12 & 0b111u;
    const u32 rs1 = code >> 15 & 0b11111u;
    const u32 rs2 = code >> 20 & 0b11111u;
    const u32 &shamt = rs2;
    const u32 func7 = code >> 25;
    const u32 imm11_0 = code >> 20;
    const u32 s_imm11_0 = sext(imm11_0, 12);
    const u32 imm31_12 = code >> 12;
//    //cout << "\trd=" << rd << "\trs1=" << rs1  << "\trs2=" << rs2 << "\tx[rd]=" << x[rd] << "\tx[rs1]=" << x[rs1]  << "\tx[rs2]=" << x[rs2] << "\ts_imm11_0=" << s_imm11_0 << "\t";
    char type = opcodeTypeMap.at(opcode);
    switch (type) {
        case 'U': {
            switch (opcode) {
                case 0b0110111u: {//lui
                    //cout << "lui" << endl;
                    x[rd] = imm31_12 << 12;//少sext兼容
                    break;
                }
                case 0b0010111u: {//auipc
                    //cout << "auipc" << endl;
                    x[rd] = pc + (imm31_12 << 12);//少sext兼容
                    break;
                }
                default: {
                    //cout << "error";
                    exit(0);
                }
            }
            break;
        }
        case 'J': {//jal
            //cout << "jal" << endl;
            u32 offset = (getbit(imm31_12, 19) | getbit(imm31_12, 18, 9) >> 9 | getbit(imm31_12, 8) << 2 |
                          getbit(imm31_12, 7, 0) << 11) << 1;
            if (rd != 0)
                x[rd] = pc + 4;//注意这个+4,是指下一条指令的地址，这是是执行完语句再改，考虑与流水的关系，等等。
            pc += sext(offset, 21) - 4;
            break;
        }
        case 'I': {
            switch (opcode) {
                case 0b1100111u: {//jalr
                    //cout << "jalr" << endl;
                    int t = pc + 4;
                    pc = ((x[rs1] + s_imm11_0) & ~1) - 4;
                    if (rd != 0)x[rd] = t;
                    break;
                }
                case 0b0000011u: {
                    int addr = x[rs1] + s_imm11_0;
                    switch (func3) {
                        case 0b000u: {//lb
                            //cout << "lb" << endl;
                            x[rd] = sext(M[addr], 8);
                            break;
                        }
                        case 0b001u: {//lh
                            //cout << "lh" << endl;
                            x[rd] = sext(M[addr] | M[addr + 1] << 8, 16);
                            break;
                        }
                        case 0b010u: {//lw
                            //cout << "lw" << endl;
                            x[rd] = M[addr] | M[addr + 1] << 8 | M[addr + 2] << 16 | M[addr + 3] << 24;
                            break;
                        }
                        case 0b100u: {//lbu
                            //cout << "lbu" << endl;
                            x[rd] = M[addr];
                            break;
                        }
                        case 0b101u: {//lhu
                            //cout << "lhu" << endl;
                            x[rd] = M[addr] | M[addr + 1] << 8;
                            break;
                        }


                        default: {
                            //cout << "error";
                            exit(0);
                        }
                    }
                    break;
                }
                case 0b0010011u: {
                    switch (func3) {
                        case 0b000u: {//addi
                            //cout << "addi" << endl;
                            x[rd] = x[rs1] + s_imm11_0;
                            break;
                        }
                        case 0b010u: {//slti
                            //cout << "slti" << endl;
                            x[rd] = (int(x[rs1]) < int(s_imm11_0));
                            break;
                        }
                        case 0b011u: {//sltiu
                            //cout << "sltiu" << endl;
                            x[rd] = (x[rs1] < s_imm11_0);
                            break;
                        }

                        case 0b100u: {//xori
                            //cout << "xori" << endl;
                            x[rd] = x[rs1] ^ s_imm11_0;
                            break;
                        }
                        case 0b110u: {//ori
                            //cout << "ori" << endl;
                            x[rd] = x[rs1] | s_imm11_0;
                            break;
                        }
                        case 0b111u: {//andi
                            //cout << "andi" << endl;
                            x[rd] = x[rs1] & s_imm11_0;
                            break;
                        }
                        case 0b001u: {//slli
                            if (func7 != 0b0000000u) {
                                //cout << "error";
                                exit(0);
                            }
                            //cout << "andi" << endl;
                            x[rd] = x[rs1] << shamt;
                            break;
                        }
                        case 0b101u: {
                            switch (func7) {
                                case 0b0000000u: {//srli
                                    //cout << "srli" << endl;
                                    x[rd] = (x[rs1] >> shamt);
                                    break;
                                }
                                case 0b0100000u: {//srai
                                    //cout << "srai" << endl;
                                    x[rd] = (int(x[rs1]) >> shamt);
                                    break;
                                }
                                default: {
                                    //cout << "error";
                                    exit(0);
                                }
                            }
                            break;
                        }

                        default: {
                            //cout << "error";
                            exit(0);
                        }
                    }

                    break;
                }
                default: {
                    //cout << "error";
                    exit(0);
                }
            }
            break;
        }
        case 'B': {
            u32 s_offset = sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 | getbit(rd, 4,1)), 13) - 4;
            switch (func3) {
                case 0b000u: {//beq
                    //cout << "beq" << endl;
                    if (x[rs1] == x[rs2]) pc += s_offset;
                    break;
                }
                case 0b001u: {//bne
                    //cout << "bne" << endl;
                    if (x[rs1] != x[rs2])
                        pc += s_offset;
                    break;
                }
                case 0b100u: {//blt
                    //cout << "blt" << endl;
                    if (int(x[rs1]) < int(x[rs2])) pc += s_offset;
                    break;
                }
                case 0b101u: {//bge
                    //cout << "bge" << endl;
                    if (int(x[rs1]) >= int(x[rs2])) pc += s_offset;
                    break;
                }
                case 0b110u: {//bltu
                    //cout << "bltu" << endl;
                    if (x[rs1] < x[rs2]) pc += s_offset;
                    break;
                }
                case 0b111u: {//bgeu
                    //cout << "bgeu" << endl;
                    if (x[rs1] >= x[rs2]) pc += s_offset;

                    break;
                }
                default: {
                    //cout << "error";
                    exit(0);
                }
            }
            break;
        }
        case 'S': {
            int addr = x[rs1] + sext(func7 << 5 | rd, 12);
            switch (func3) {
                case 0b000u: {//sb
                    //cout << "sb" << endl;
                    M[addr] = x[rs2];
                    break;
                }
                case 0b001u: {//sh
                    //cout << "sh" << endl;
                    M[addr] = x[rs2];
                    M[addr + 1] = x[rs2] >> 8;
                    break;
                }
                case 0b010u: {//sw
                    //cout << "sw" << endl;
                    M[addr] = x[rs2];
                    M[addr + 1] = x[rs2] >> 8;
                    M[addr + 2] = x[rs2] >> 16;
                    M[addr + 3] = x[rs2] >> 24;
                    break;
                }
                default: {
                    //cout << "error";
                    exit(0);
                }
            }

            break;
        }
        case 'R': {
            switch (func7) {
                case 0b0000000u: {
                    switch (func3) {
                        case 0b000u: {//add
                            //cout << "add" << endl;
                            x[rd] = x[rs1] + x[rs2];
                            break;
                        }
                        case 0b001u: {//sll
                            //cout << "sll" << endl;
                            x[rd] = x[rs1] << x[rs2];
                            break;
                        }
                        case 0b010u: {//slt
                            //cout << "slt" << endl;
                            x[rd] = (int(x[rs1]) < int(x[rs2]));
                            break;
                        }
                        case 0b011u: {//sltu
                            //cout << "sltu" << endl;
                            x[rd] = (x[rs1] < x[rs2]);
                            break;
                        }
                        case 0b100u: {//xor
                            //cout << "xor" << endl;
                            x[rd] = x[rs1] ^ x[rs2];
                            break;
                        }
                        case 0b101u: {//srl
                            //cout << "srl" << endl;
                            x[rd] = (x[rs1] >> x[rs2]);
                            break;
                        }
                        case 0b110u: {//or
                            //cout << "or" << endl;
                            x[rd] = x[rs1] | x[rs2];
                            break;
                        }
                        case 0b111u: {//and
                            //cout << "and" << endl;
                            x[rd] = x[rs1] & x[rs2];
                            break;
                        }
                        default: {
                            //cout << "error";
                            exit(0);
                        }
                    }
                    break;
                }
                case 0b0100000u: {
                    switch (func3) {
                        case 0b000u: {//sub
                            //cout << "sub" << endl;
                            x[rd] = x[rs1] - x[rs2];
                            break;
                        }
                        case 0b101u: {//sra
                            //cout << "sra" << endl;
                            x[rd] = (int(x[rs1]) >> int(x[rs2]));
                            break;
                        }
                        default: {
                            //cout << "error";
                            exit(0);
                        }
                    }
                    break;
                }
                default: {
                    //cout << "error";
                    exit(0);
                }
            }

            break;
        }
        default: {
            //cout << "error";
            exit(0);
        }
    }
}


