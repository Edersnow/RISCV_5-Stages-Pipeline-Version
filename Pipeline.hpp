#ifndef __Pipeline
#define __Pipeline

#include "Instruction.hpp"
struct IF_register;
struct ID_register;
struct EX_register;
struct MEM_register;

/***                                                               ***/
/***                            IF PART                            ***/
/***                                                               ***/

struct IF_register{
    bool is_empty;
    uint cur_instruction;
    uint cur_pc;

    IF_register();
    void operate_IF();
};



/***                                                               ***/
/***                            ID PART                            ***/
/***                                                               ***/

struct ID_register{
    bool is_empty;
    bool is_banched;
    instruction_decoder cur_dins;
    uint _val1;
    uint _val2;
    uint cur_pc;

    ID_register();
    void operate_ID(IF_register &cur_IF, EX_register &cur_EX, MEM_register &cur_MEM);
};



/***                                                               ***/
/***                            EX PART                            ***/
/***                                                               ***/

struct EX_register{
    bool is_empty;
    typeT cur_type;
    uint cur_imm;
    uint _val1;
    uint _val2;
    uint _rd;
    uint _vrd;
    uint cur_pc;

    EX_register();
    void operate_EX(ID_register &cur_ID, IF_register &cur_IF);
};



/***                                                               ***/
/***                            MEM PART                           ***/
/***                                                               ***/

struct MEM_register{

    bool is_empty;
    typeT cur_type;
    uint cur_imm;
    uint _val1;
    uint _val2;
    uint _rd;
    uint _vrd;
    uint cur_pc;
    uint cur_period;

    MEM_register();
    void operate_MEM(EX_register &cur_EX);
    void operate_WB();
};










/***                                                               ***/
/***                            IF PART                            ***/
/***                                                               ***/

IF_register::IF_register() {is_empty=true;}

void IF_register::operate_IF(){
    is_empty=false;
    cur_pc=_pc;
    _pc+=4;
    memcpy(&cur_instruction, _memory + cur_pc, sizeof(uint));


    //for debug
    //printf("IF:ins %08x, pc %4x\n", cur_instruction, cur_pc);
}



/***                                                               ***/
/***                            ID PART                            ***/
/***                                                               ***/

ID_register::ID_register() {is_empty=true;}

void ID_register::operate_ID(IF_register &cur_IF, EX_register &cur_EX, MEM_register &cur_MEM){
    if(cur_IF.cur_instruction == 0x0ff00513){
        is_end=true;
        cur_IF.is_empty=true;
        return;
    }
    cur_dins=cur_IF.cur_instruction;



    //forwarding part
    switch (cur_dins._format){
        //TYPE U/UJ
        case LUI:  case AUIPC:  case JAL:
            if(cur_dins._rd)  ++target[cur_dins._rd];
            break;

        //TYPE I
        case JALR:  case LB:  case LH:  case LW:  case LBU:
        case LHU:  case ADDI:  case SLTI:  case SLTIU:  case XORI:
        case ORI:  case ANDI:  case SLLI:  case SRLI:  case SRAI:
            if(target[cur_dins._rs1]){
                if(cur_EX.cur_type <= 15 && cur_EX.cur_type >= 11 && !cur_EX.is_empty && cur_EX._rd == cur_dins._rs1)  return;
                if(!cur_EX.is_empty && cur_EX._rd == cur_dins._rs1)  _val1=cur_EX._vrd;
                else  _val1=cur_MEM._vrd;
            }
            else  _val1=_register[cur_dins._rs1];
            if(cur_dins._rd)  ++target[cur_dins._rd];
            break;

        //TYPE R & S & SB
        case ADD:  case SUB:  case SLL:  case SLT:  case SLTU:
        case XOR:  case SRL:  case SRA:  case OR:  case AND:
        case SB:  case SH:  case SW:  case BEQ:  case BNE:
        case BLT:  case BGE:  case BLTU:  case BGEU:
            if(target[cur_dins._rs1]){
                if(cur_EX.cur_type <= 15 && cur_EX.cur_type >= 11 && !cur_EX.is_empty && cur_EX._rd == cur_dins._rs1)  return;
                if(!cur_EX.is_empty && cur_EX._rd == cur_dins._rs1)  _val1=cur_EX._vrd;
                else  _val1=cur_MEM._vrd;

                if(target[cur_dins._rs2]){
                    if(cur_EX.cur_type <= 15 && cur_EX.cur_type >= 11 && !cur_EX.is_empty && cur_EX._rd == cur_dins._rs2)  return;
                    if(!cur_EX.is_empty && cur_EX._rd == cur_dins._rs2)  _val2=cur_EX._vrd;
                    else  _val2=cur_MEM._vrd;
                }
                else  _val2=_register[cur_dins._rs2];
            }
            else{
                _val1=_register[cur_dins._rs1];

                if(target[cur_dins._rs2]){
                    if(cur_EX.cur_type <= 15 && cur_EX.cur_type >= 11 && !cur_EX.is_empty && cur_EX._rd == cur_dins._rs2)  return;
                    if(!cur_EX.is_empty && cur_EX._rd == cur_dins._rs2)  _val2=cur_EX._vrd;
                    else  _val2=cur_MEM._vrd;
                }
                else  _val2=_register[cur_dins._rs2];
            }

            switch (cur_dins._format){
                case ADD:  case SUB:  case SLL:  case SLT:  case SLTU:
                case XOR:  case SRL:  case SRA:  case OR:  case AND:
                    if(cur_dins._rd)  ++target[cur_dins._rd];
            }
            break;
    }

    is_empty=false;
    cur_IF.is_empty=true;
    if(cur_dins._rs1==0)  _val1=0;
    if(cur_dins._rs2==0)  _val2=0;
    cur_pc=cur_IF.cur_pc;
    is_banched=false;



    //banche predict(2-bit)
    if(cur_dins._format>=28 && cur_dins._format<=33){
        if(_predictor[cur_dins._format-28][((cur_dins._immediate)>>1)&15]>1)
            _pc=cur_pc + cur_dins._immediate, is_banched=true;
        ++total_prediction;
    }

    //exception(no need to predict)
    if(cur_dins._format == JAL)
        _pc = cur_pc + cur_dins._immediate;
    if(cur_dins._format == JALR)
        _pc = (cur_dins._immediate+_val1)&(-2);

    //for debug
    //printf("ID:ins %s\n", TYPES[cur_dins._format]);
    //printf("   rs1 %u, rs2 %u\n", cur_dins._rs1, cur_dins._rs2);
    //printf("   val1 %u, val2 %u, rd %u, imm %u\n", _val1, _val2, cur_dins._rd, cur_dins._immediate);
}



/***                                                               ***/
/***                            EX PART                            ***/
/***                                                               ***/

EX_register::EX_register() {is_empty=true;}

void EX_register::operate_EX(ID_register &cur_ID, IF_register &cur_IF){
    cur_type=cur_ID.cur_dins._format;
    cur_imm=cur_ID.cur_dins._immediate;
    _val1=cur_ID._val1;
    _val2=cur_ID._val2;
    _rd=cur_ID.cur_dins._rd;
    cur_pc=cur_ID.cur_pc;
    cur_ID.is_empty=true;
    is_empty=false;

    switch (cur_type){
        //PART R
        case ADD:  _vrd = _val1 + _val2; break;
        case SUB:  _vrd = _val1 - _val2; break;
        case SLL:  _vrd = _val1 << (_val2&31); break;
        case SLT:  _vrd = int(_val1) < int(_val2); break;
        case SLTU:  _vrd = _val1 < _val2; break;
        case XOR:  _vrd = _val1 ^ _val2; break;
        case SRL:  _vrd = _val1 >> (_val2&31); break;
        case SRA:  _vrd = int(_val1) >> (_val2&31); break;
        case OR:  _vrd = _val1 | _val2; break;
        case AND:  _vrd = _val1 & _val2; break;

        //PART I
        case JALR:  _vrd = cur_pc + 4; break;
        case ADDI:  _vrd = _val1 + cur_imm; break;
        case SLTI:  _vrd = int(_val1) < int(cur_imm); break;
        case SLTIU:  _vrd = _val1 < cur_imm; break;
        case XORI:  _vrd = _val1 ^ cur_imm; break;
        case ORI:  _vrd = _val1 | cur_imm; break;
        case ANDI:  _vrd = _val1 & cur_imm; break;
        case SLLI:  _vrd = _val1 << (cur_imm&31); break;
        case SRLI:  _vrd = _val1 >> (cur_imm&31); break;
        case SRAI:  _vrd = int(_val1) >> (cur_imm&31); break;

        //PART SB
        case BEQ:
            if(_val1 == _val2){
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::min(_predictor[cur_type-28][((cur_imm)>>1)&15]+1, 3u);
                if(cur_ID.is_banched)  ++correct_prediction;
                else{
                    cur_IF.is_empty=true;
                    _pc=cur_pc+cur_imm;
                }
            }
            else{
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::max(_predictor[cur_type-28][((cur_imm)>>1)&15]-1, 0u);
                if(cur_ID.is_banched){
                    cur_IF.is_empty=true;
                    _pc=cur_pc+4;
                }
            }
            break;

        case BNE:
            if(_val1 != _val2){
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::min(_predictor[cur_type-28][((cur_imm)>>1)&15]+1, 3u);
                if(cur_ID.is_banched)  ++correct_prediction;
                else{
                    cur_IF.is_empty=true;
                    _pc=cur_pc+cur_imm;
                }
            }
            else{
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::max(_predictor[cur_type-28][((cur_imm)>>1)&15]-1, 0u);
                if(cur_ID.is_banched){
                    cur_IF.is_empty=true;
                    _pc=cur_pc+4;
                }
            }
            break;

        case BLT:
            if(int(_val1) < int(_val2)){
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::min(_predictor[cur_type-28][((cur_imm)>>1)&15]+1, 3u);
                if(cur_ID.is_banched)  ++correct_prediction;
                else{
                    cur_IF.is_empty=true;
                    _pc=cur_pc+cur_imm;
                }
            }
            else{
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::max(_predictor[cur_type-28][((cur_imm)>>1)&15]-1, 0u);
                if(cur_ID.is_banched){
                    cur_IF.is_empty=true;
                    _pc=cur_pc+4;
                }
            }
            break;

        case BGE:
            if(int(_val1) >= int(_val2)){
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::min(_predictor[cur_type-28][((cur_imm)>>1)&15]+1, 3u);
                if(cur_ID.is_banched)  ++correct_prediction;
                else{
                    cur_IF.is_empty=true;
                    _pc=cur_pc+cur_imm;
                }
            }
            else{
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::max(_predictor[cur_type-28][((cur_imm)>>1)&15]-1, 0u);
                if(cur_ID.is_banched){
                    cur_IF.is_empty=true;
                    _pc=cur_pc+4;
                }
            }
            break;

        case BLTU:
            if(_val1 < _val2){
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::min(_predictor[cur_type-28][((cur_imm)>>1)&15]+1, 3u);
                if(cur_ID.is_banched)  ++correct_prediction;
                else{
                    cur_IF.is_empty=true;
                    _pc=cur_pc+cur_imm;
                }
            }
            else{
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::max(_predictor[cur_type-28][((cur_imm)>>1)&15]-1, 0u);
                if(cur_ID.is_banched){
                    cur_IF.is_empty=true;
                    _pc=cur_pc+4;
                }
            }
            break;

        case BGEU:
            if(_val1 >= _val2){
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::min(_predictor[cur_type-28][((cur_imm)>>1)&15]+1, 3u);
                if(cur_ID.is_banched)  ++correct_prediction;
                else{
                    cur_IF.is_empty=true;
                    _pc=cur_pc+cur_imm;
                }
            }
            else{
                _predictor[cur_type-28][((cur_imm)>>1)&15]=std::max(_predictor[cur_type-28][((cur_imm)>>1)&15]-1, 0u);
                if(cur_ID.is_banched){
                    cur_IF.is_empty=true;
                    _pc=cur_pc+4;
                }
            }
            break;

        //PART U
        case LUI:  _vrd = cur_imm; break;
        case AUIPC:  _vrd = cur_pc + cur_imm; break;

        //PART UI
        case JAL:  _vrd = cur_pc + 4; break;
    }



    //for debug
    //printf("EX: cur instruction %u\n", cur_type);
}



/***                                                               ***/
/***                            MEM PART                           ***/
/***                                                               ***/

MEM_register::MEM_register() {is_empty=true; cur_period=0;}

void MEM_register::operate_MEM(EX_register &cur_EX){
    cur_type=cur_EX.cur_type;
    cur_imm=cur_EX.cur_imm;
    _val1=cur_EX._val1;
    _val2=cur_EX._val2;
    _rd=cur_EX._rd;
    _vrd=cur_EX._vrd;
    cur_pc=cur_EX.cur_pc;
    cur_period=1;
    is_empty=false;
    cur_EX.is_empty=true;

    switch (cur_type){
        case LB:
            char tmp1;
            memcpy(&tmp1, _memory + (_val1 + cur_imm), sizeof(char));
            _vrd=uint(tmp1);
            break;

        case LH:
            short tmp2;
            memcpy(&tmp2, _memory + (_val1 + cur_imm), sizeof(short));
            _vrd=uint(tmp2);
            break;

        case LW:
            memcpy(&_vrd, _memory + (_val1 + cur_imm), sizeof(uint));
            break;

        case LBU:
            unsigned char tmp3;
            memcpy(&tmp3, _memory + (_val1 + cur_imm), sizeof(unsigned char));
            _vrd=uint(tmp3);
            break;

        case LHU:
            unsigned short tmp4;
            memcpy(&tmp4, _memory + (_val1 + cur_imm), sizeof(unsigned short));
            _vrd=uint(tmp4);
            break;

        case SB:
            char tmp5;
            tmp5=_val2;
            memcpy(_memory + (_val1 + cur_imm), &tmp5, sizeof(char));
            break;

        case SH:
            short tmp6;
            tmp6=_val2;
            memcpy(_memory + (_val1 + cur_imm), &tmp6, sizeof(short));
            break;

        case SW:
            memcpy(_memory + (_val1 + cur_imm), &_val2, sizeof(uint));
            break;
    }


    //for debug
    //printf("MEM: cur instruction %u\n", cur_type);
}

void MEM_register::operate_WB(){
    switch (cur_type){
        case LB:  case LH:  case LW:  case LBU:  case LHU:
        case SB:  case SH:  case SW:
            if(cur_period!=3){
                ++cur_period;
                return;
            }
    }
    is_empty=true;
    switch (cur_type){
        case SB:  case SH:  case SW:  case BEQ:  case BNE:
        case BLT:  case BGE:  case BLTU:  case BGEU:
            break;

        default:
            if(_rd){
                --target[_rd];
                _register[_rd]=_vrd;
            }
            break;
    }


    //for debug
    //printf("WB: cur instruction %u\n", cur_type);
}

#endif