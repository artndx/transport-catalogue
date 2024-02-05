#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>

#include "json_reader.h"

using namespace std;
using namespace transport_catalogue;
using namespace transport_catalogue::json_reader;

int main() {
    TransportCatalogue catalogue;
    // ifstream input("build/Debug/input.json");
    // ofstream output("build/Debug/output.json");
    JsonReader reader(cin, cout);
    reader.SendRequests(catalogue);
    reader.GetResponses();
}