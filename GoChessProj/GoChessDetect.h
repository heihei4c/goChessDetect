#include <opencv2/core/core.hpp>
#include "GoChessObjectClass.cpp"

class GoChessDetect
    {
    public:
        int goChessDetect(std::string data, std::vector<GoChessObjectClass> &goChessList);
    };

