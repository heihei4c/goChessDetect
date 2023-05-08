#include "crow.h"
#include <cpp-base64/base64.h>
#include "GoChessDetect.h"


int main()
{
    crow::SimpleApp app;

    // Define the route for the API endpoint
    CROW_ROUTE(app, "/goChessDetectAPI")
        .methods("POST"_method)
        ([](const crow::request& req) {
        // Get the input data from the request body
        std::string input_data = req.body;

        // Decode the Base64-encoded string
        std::string decoded = base64_decode(input_data);

        // Process the input data and generate the output

        std::vector<GoChessObjectClass> goChessList;
        GoChessDetect detectObj;

        detectObj.goChessDetect(decoded, goChessList);

        int listSize = goChessList.size();

        std::ostringstream ss;
        ss << "[";
        for (int i = 0; i < listSize; i++) {
            ss << "{";
            ss << "\"isBlack\":" << (goChessList[i].getIsBlack() ? "true" : "false") << ",";
            ss << "\"xPos\":" << goChessList[i].getXPos() << ",";
            ss << "\"yPos\":" << goChessList[i].getYPos();
            ss << "}";

            if (i < listSize - 1) {
                ss << ",";
            }
        }
        ss << "]";

        // Return the output as a response
        crow::response res(ss.str());
        res.set_header("Content-Type", "application/json");
        return res;
            });

    // Start the server on port 8080
    app.port(8080).run();

    return 0;
}