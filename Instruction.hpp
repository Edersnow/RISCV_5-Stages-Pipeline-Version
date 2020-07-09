#ifndef __Instruction
#define __Instruction

#include "Global.hpp"

struct instruction_decoder{
    typeT _format;
    uint _immediate;
    uint _opcode;
    uint _funct3;
    uint _funct7;
    uint _rs1;
    uint _rs2;
    uint _rd;

    instruction_decoder &operator=(uint ins){
        _opcode=ins&127;
        switch (_opcode){

            //TYPE R
            case 0b0110011:
                _immediate=0;
                _funct3=(ins>>12)&7;
                _funct7=(ins>>25)&127;
                _rs1=(ins>>15)&31;
                _rs2=(ins>>20)&31;
                _rd=(ins>>7)&31;

                switch (_funct3){
                    case 0b000:
                        if(_funct7)  _format=SUB;
                        else  _format=ADD;
                        break;

                    case 0b001:
                        _format=SLL;
                        break;

                    case 0b010:
                        _format=SLT;
                        break;

                    case 0b011:
                        _format=SLTU;
                        break;

                    case 0b100:
                        _format=XOR;
                        break;

                    case 0b101:
                        if(_funct7)  _format=SRA;
                        else  _format=SRL;
                        break;

                    case 0b110:
                        _format=OR;
                        break;

                    case 0b111:
                        _format=AND;
                        break;
                }
                break;

            //TYPE I
            case 0b1100111:  case 0b0000011:  case 0b0010011:
                _immediate=(ins>>20)&4095;
                if(ins>>31)  _immediate|=0xfffff000;
                _funct3=(ins>>12)&7;
                _funct7=0;
                _rs1=(ins>>15)&31;
                _rs2=0;
                _rd=(ins>>7)&31;

                switch (_opcode){
                    case 0b1100111:
                        _format=JALR;
                        break;

                    case 0b0000011:
                        switch (_funct3){
                            case 0b000:
                                _format=LB;
                                break;

                            case 0b001:
                                _format=LH;
                                break;

                            case 0b010:
                                _format=LW;
                                break;

                            case 0b100:
                                _format=LBU;
                                break;

                            case 0b101:
                                _format=LHU;
                                break;
                        }
                        break;

                    case 0b0010011:
                        switch (_funct3){
                            case 0b000:
                                _format=ADDI;
                                break;

                            case 0b001:
                                _funct7=_immediate>>5;
                                _format=SLLI;
                                break;

                            case 0b010:
                                _format=SLTI;
                                break;

                            case 0b011:
                                _format=SLTIU;
                                break;

                            case 0b100:
                                _format=XORI;
                                break;

                            case 0b101:
                                _funct7=_immediate>>5;
                                if(_funct7)  _format=SRAI;
                                else  _format=SRLI;
                                break;

                            case 0b110:
                                _format=ORI;
                                break;

                            case 0b111:
                                _format=ANDI;
                                break;
                        }
                        break;
                }
                break;

            //TYPE S
            case 0b0100011:
                _immediate=((ins>>7)&31)|(((ins>>25)&127)<<5);
                if(ins>>31)  _immediate|=0xfffff000;
                _funct3=(ins>>12)&7;
                _funct7=0;
                _rs1=(ins>>15)&31;
                _rs2=(ins>>20)&31;
                _rd=0;

                switch (_funct3){
                    case 0b000:
                        _format=SB;
                        break;

                    case 0b001:
                        _format=SH;
                        break;

                    case 0b010:
                        _format=SW;
                        break;
                }
                break;

            //TYPE SB
            case 0b1100011:
                _immediate=(((ins>>8)&15)<<1)|(((ins>>25)&63)<<5)|(((ins>>7)&1)<<11);
                if(ins>>31)  _immediate|=0xfffff000;
                _funct3=(ins>>12)&7;
                _funct7=0;
                _rs1=(ins>>15)&31;
                _rs2=(ins>>20)&31;
                _rd=0;

                switch (_funct3){
                    case 0b000:
                        _format=BEQ;
                        break;

                    case 0b001:
                        _format=BNE;
                        break;

                    case 0b100:
                        _format=BLT;
                        break;

                    case 0b101:
                        _format=BGE;
                        break;

                    case 0b110:
                        _format=BLTU;
                        break;

                    case 0b111:
                        _format=BGEU;
                        break;
                }
                break;

            //TYPE U
            case 0b0110111:
                _format=LUI;
                _immediate=ins&0xfffff000;
                _funct3=0;
                _funct7=0;
                _rs1=0;
                _rs2=0;
                _rd=(ins>>7)&31;
                break;
            case 0b0010111:
                _format=AUIPC;
                _immediate=ins&0xfffff000;
                _funct3=0;
                _funct7=0;
                _rs1=0;
                _rs2=0;
                _rd=(ins>>7)&31;
                break;

            //TYPE J
            case 0b1101111:
                _format=JAL;
                _immediate=(((ins>>21)&1023)<<1)|(((ins>>20)&1)<<11)|(((ins>>12)&255)<<12);
                if(ins>>31)  _immediate|=0xfff00000;
                _funct3=0;
                _funct7=0;
                _rs1=0;
                _rs2=0;
                _rd=(ins>>7)&31;
                break;
        }
        return *this;
    }
};

#endif
