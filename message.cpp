#include <cstring>
#include <chrono>

using namespace std;

class message
{
    public:
        int command;
        int src;
        int dest;

        message() {};

        message(int command, int src, int dest)
        {
            this->command = command;
            this->src = src;
            this->dest = dest;
        };

        void setContent(int command, int src, int dest)
        {
            this->command = command;
            this->src = src;
            this->dest = dest;
        };
};
