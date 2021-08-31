#pragma once

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

class IBlockOutputter {
public:
    virtual ~IBlockOutputter(){};
    virtual void Output(std::queue<CommandBlock>& toOutput, bool& InputStringFinished, std::mutex& mut) = 0;
};

class CommandBlockConsoleOutputter : public IBlockOutputter {
private:
public:
    CommandBlockConsoleOutputter() = default;

    void Output(std::queue<CommandBlock>& toOutput, bool& InputStringFinished, std::mutex& mut)
    {
        while ((!InputStringFinished) || (!toOutput.empty())){
            std::lock_guard<std::mutex> guard{mut};
            if (toOutput.empty()) continue;
            vector<string> commands = toOutput.front().mCommands;
            toOutput.pop();
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

class CommandBlockFileOutputter : public IBlockOutputter {
private:
    int mId;
public:
    CommandBlockFileOutputter(int id) : mId(id)
    {}

    void Output(std::queue<CommandBlock>& toOutput, bool& InputStringFinished, std::mutex& mut)
    {
        while ((!InputStringFinished) || (!toOutput.empty())) {
            std::lock_guard<std::mutex> guard{mut};
            if (toOutput.empty()) continue;
            vector<string> commands = toOutput.front().mCommands;
            size_t timestamp = toOutput.front().mTimestamp;
            toOutput.pop();

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

class CommandInterpreter
{
private:
    CommandBlock mCurBlock;
    size_t mCurCommandIdx; //number of command that is being processed (does not contain brackets)
    size_t mN;
    int mEmbedBracers = 0;
    bool mAmDynBlock = false;

public:
    CommandInterpreter(size_t N) : mN(N)
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
    template<typename T>
    void InterpretInputs(T& in, CommandInterpreter& M, queue<CommandBlock>& FileLoggerQueue, queue<CommandBlock>& ConsoleLoggerQueue)
    {
        std::cin.rdbuf(in.rdbuf());
        string command;

        for(std::string command; std::getline(in, command);)
        {
            if (command == "") continue;
            bool is_complete = M.Interpret(command);
            if (is_complete)
            {
                PushBlockToOutputQueues(M.GetCurrentBlock(), FileLoggerQueue, ConsoleLoggerQueue);
                M.Refresh(); // empty buffers
            }
            std::this_thread::sleep_for(0.1s);
        }

        return;
    }

    void PushBlockToOutputQueues(CommandBlock& B, queue<CommandBlock>& FileLoggerQueue, queue<CommandBlock>& ConsoleLoggerQueue) {
        FileLoggerQueue.push(B);
        ConsoleLoggerQueue.push(B);
    }
};

class MultithreadCommandParser
{
private:
    InputCommandParser mICP;
    CommandBlockConsoleOutputter mCO;
    CommandInterpreter mCI;
    CommandBlockFileOutputter mFO1, mFO2;

    queue<CommandBlock> mFileLoggerQueue;
    queue<CommandBlock> mConsoleLoggerQueue;
    bool mInputFinished;
    std::mutex mut_console, mut_file;
    std::thread CO_thread, FO1_thread, FO2_thread;


public:
    MultithreadCommandParser(size_t N) : mCI(N), mFO1(1), mFO2(2), mInputFinished(false),
    CO_thread {&CommandBlockConsoleOutputter::Output, &mCO, std::ref(mConsoleLoggerQueue), std::ref(mInputFinished), std::ref(mut_console)},
    FO1_thread { &CommandBlockFileOutputter::Output, &mFO1,  std::ref(mFileLoggerQueue), std::ref(mInputFinished), std::ref(mut_file)},
    FO2_thread { &CommandBlockFileOutputter::Output, &mFO2,  std::ref(mFileLoggerQueue), std::ref(mInputFinished), std::ref(mut_file)}
    {
    }

    ~MultithreadCommandParser()
    {
        if (!mCI.IsDynBlock()) mICP.PushBlockToOutputQueues(mCI.GetCurrentBlock(), mFileLoggerQueue, mConsoleLoggerQueue);
        mInputFinished = true;

        CO_thread.join();
        FO1_thread.join();
        FO2_thread.join();
    }

    void ReceiveInput(const std::string& str)
    {
        std::istringstream in(str);
        mICP.InterpretInputs(in, mCI, mFileLoggerQueue, mConsoleLoggerQueue);
        return;
    }

};