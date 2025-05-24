// server.cpp
#include "httplib.h"
#include <iostream>

using namespace std;

int main()
{
    httplib::Server server;

    server.Get("/status", [](const httplib::Request&, httplib::Response& res){
        res.set_content("Server is running!", "text/plain");
    });

    server.Post("/echo", [](const httplib::Request& request, httplib::Response& res){
        string body = request.body;
        cout << "Received POST body: " << body << endl;
        res.set_content("Echo: " + body, "text/plain");
    });
    
    // Accept connection
    cout << "API SERVER running at http://0.0.0.0:8080\n";
    server.listen("0.0.0.0", 8080); 
}
