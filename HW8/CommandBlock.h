#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <thread>
#include <queue>
#include <mutex>

using namespace std;

struct CommandBlock{
    vector<string> mCommands;
    size_t mTimestamp;
};

std::queue<CommandBlock> FileLoggerQueue;
std::queue<CommandBlock> ConsoleLoggerQueue;
bool InputFinished = false;
std::mutex mut;

class IOutputter {
public:
    virtual ~IOutputter(){};
    virtual void Output() = 0;
};

class CommandBlockConsoleOutputter : public IOutputter {
private:
public:
    CommandBlockConsoleOutputter() = default;

    void Output()
    {
        while ((!InputFinished) || (!ConsoleLoggerQueue.empty())){
            if (ConsoleLoggerQueue.empty()) continue;
            vector<string> commands = ConsoleLoggerQueue.front().mCommands;
            ConsoleLoggerQueue.pop();
            cout << "bulk:";
            char c = ' ';
            for (const auto &el:commands) {
                if (el == "{" || el == "}")
                    continue;
                cout << c << el;
                c = ',';
            }
            cout << endl;
        }
    }
};

class CommandBlockFileOutputter : public IOutputter {
private:
    int mId;
public:
    CommandBlockFileOutputter(int id) : mId(id)
    {}

    void Output()
    {
        while ((!InputFinished) || (!FileLoggerQueue.empty())) {
            std::lock_guard<std::mutex> guard{mut};
            if (FileLoggerQueue.empty()) continue;
            vector<string> commands = FileLoggerQueue.front().mCommands;
            size_t timestamp = FileLoggerQueue.front().mTimestamp;
            FileLoggerQueue.pop();

            ofstream f;
            string s = "bulk" + std::to_string(timestamp) + "_" + to_string(mId) + ".log";
            f.open(s);

            f << "bulk:";
            char c = ' ';
            for (const auto &el:commands) {
                if (el == "{" || el == "}")
                    continue;
                f << c << el;
                c = ',';
            }
            f << endl;
            f.close();
        }
    }
};

class CommandMemoryManager
{
private:
    CommandBlock mCurBlock;
    size_t mCurCommandIdx; //number of command that is being processed (does not contain brackets)
    size_t mN;
    int mEmbedBracers = 0;
    bool mAmDynBlock = false;

public:
    CommandMemoryManager(size_t N) : mN(N)
    {
        Refresh();
    }

    bool Interpret(string command)
    {
        if (mCurCommandIdx == 0) {
            const auto p1 = std::chrono::system_clock::now();
            mCurBlock.mTimestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
        }

        if (command != "}" && command != "{") mCurCommandIdx++;
        mCurBlock.mCommands.push_back(command); // does contain brackets

        if (command == "}") {
            if (--mEmbedBracers == 0) // that is a dyn block finish
                return true;
            return false;
        }

        if (command == "{") { // this is an emergency exit for the block
            mAmDynBlock = true;
            if ((++mEmbedBracers == 1)  && (mCurCommandIdx > 0))
                return true;
            return false;
        }

        if  ((mCurCommandIdx == mN) && (mEmbedBracers == 0)) // block is finished
        {
            return true;
        }
        return false;
    }

    void Refresh()
    {
        mCurCommandIdx = 0;
        mEmbedBracers = 0;
        if (mCurBlock.mCommands.size() > 0 && mCurBlock.mCommands[mCurBlock.mCommands.size() - 1] == "{") {
            mEmbedBracers++;
            mAmDynBlock = true;
        }
        mCurBlock.mCommands.clear();
        return;
    }

    size_t GetBlockStartTimestamp()
    {
        return mCurBlock.mTimestamp;
    }

    CommandBlock& GetCurrentBlock()
    {
        return mCurBlock;
    }

    bool IsDynBlock()
    {
        return mAmDynBlock;
    }

};

class InputCommandParser
{
public:
    InputCommandParser() = default;
    void InterpretInputs(std::ifstream& in, CommandMemoryManager& M)
    {
        std::cin.rdbuf(in.rdbuf());
        string command;

        for(std::string command; std::getline(in, command);)
        {
            bool is_complete = M.Interpret(command);
            if (is_complete)
            {
                PushBlockToOutputQueues(M.GetCurrentBlock());
                M.Refresh(); // empty buffers
            }
            std::this_thread::sleep_for(1s);
        }

        if (!M.IsDynBlock()) PushBlockToOutputQueues(M.GetCurrentBlock());
        InputFinished = true;
        return;
    }

    void PushBlockToOutputQueues(CommandBlock& B) {
        FileLoggerQueue.push(B);
        ConsoleLoggerQueue.push(B);
    }
};

