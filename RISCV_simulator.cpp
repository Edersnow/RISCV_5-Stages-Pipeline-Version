#include "Pipeline.hpp"

int main()
{
    IF_register cur_IF;
    ID_register cur_ID;
    EX_register cur_EX;
    MEM_register cur_MEM;
    uint cnt=1;
    uint memory_start;
    char _input[20];

    for(int i=0;i<64;++i)
        for(int j=0;j<256;++j)
            PHT[i][j]=1;
    for(int i=0;i<65536;++i)
        PHT_for_BHR[i]=1;

    /*
    if(!freopen("testcases/array_test1.data", "r", stdin)){
        printf("Error in reading!\n");
        return 0;
    }
    printf("Freopen successfully!\n");
    */
    

    while(scanf("%s", _input)!=EOF){
        if(_input[0]=='@')  sscanf(_input+1, "%x", &memory_start);
        else{
            sscanf(_input, "%x", &_memory[memory_start++]);
            scanf("%x", &_memory[memory_start++]);
            scanf("%x", &_memory[memory_start++]);
            scanf("%x", &_memory[memory_start++]);
            //printf("Read data: %02x%02x%02x%02x\n", _memory[memory_start-1],
            //_memory[memory_start-2], _memory[memory_start-3], _memory[memory_start-4]);
        }
    }
    //printf("Read data successfully!\n");



    //simulate
    do{
        //printf("Cur period: %u\n", cnt++);
        if(!cur_MEM.is_empty)  cur_MEM.execute_WB();
        if(cur_MEM.is_empty && !cur_EX.is_empty)  cur_MEM.execute_MEM(cur_EX);
        if(cur_EX.is_empty && !cur_ID.is_empty)  cur_EX.execute_EX(cur_ID, cur_IF);
        if(cur_ID.is_empty && !cur_IF.is_empty)  cur_ID.execute_ID(cur_IF, cur_EX, cur_MEM);
        if(cur_IF.is_empty && !is_end)  cur_IF.execute_IF();
        //printf("-----------------------------------------\n");
    }while (!cur_IF.is_empty || !cur_ID.is_empty || !cur_EX.is_empty || !cur_MEM.is_empty);

    //printf("Total prediction: %u\n", total_prediction);
    //printf("Correct prediction: %u\n", correct_prediction);
    //printf("Prediction accuracy: %.2f%\n", (double(correct_prediction)/total_prediction)*100);
    //printf("Result: %u\n", _register[10]&255);
    printf("%u\n", _register[10]&255);
    return 0;
}