#include "crow.h"
#include "crow/middlewares/cors.h"
#include <cpp-base64/base64.h>
#include "GoChessDetect.h"


int main()
{
    crow::App<crow::CORSHandler> app;
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .origin("*")
        .headers("origin, x-requested-with, accept, access-control-allow-origin, authorization, content-type")
        .methods("POST"_method, "GET"_method, "PUT"_method, "DELETE"_method, "PATCH"_method, "OPTIONS"_method);


    // Define the route for the API endpoint
    CROW_ROUTE(app, "/goChessDetectAPI")
        .methods("POST"_method)
        ([](const crow::request& req) {

        crow::response res;
        // Get the input data from the request body
        std::string input_data = req.body;

        // Decode the Base64-encoded string
        std::string decoded = base64_decode(input_data);

        // Process the input data and generate the output

        std::vector<GoChessObjectClass> goChessList;
        GoChessDetect detectObj;

        int result = detectObj.goChessDetect(decoded, goChessList);

        if (result == 0) {
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
            res = crow::response(ss.str());
            res.set_header("Content-Type", "application/json");
        }
        else {
            res = crow::response(500);
        }
        return res;
            });

    // Start the server on https
    app.port(443).ssl_file("server.crt", "server.key").multithreaded().run();
    //app.port(8080).run();

    return 0;
}