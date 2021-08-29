#include "CommandBlock.h"
#include <thread>
int main(int argc, char** argv)
{
    //std::cout << std::thread::hardware_concurrency() << std::endl;

    std::ifstream in("../input/2.txt");
    size_t N = (argc > 1) ? atoi(argv[1]) : 3;
    CommandMemoryManager M(N);
    InputCommandParser I;
    CommandBlockConsoleOutputter CO;
    CommandBlockFileOutputter FO1(1), FO2(2);

    std::thread CO_thread { &CommandBlockConsoleOutputter::Output, &CO };
    std::thread FO1_thread { &CommandBlockFileOutputter::Output, &FO1 };
    std::thread FO2_thread { &CommandBlockFileOutputter::Output, &FO2 };

    I.InterpretInputs(in, M);
    CO_thread.join();
    FO1_thread.join();
    FO2_thread.join();

    return 0;
}