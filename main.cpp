#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>

#include "json_reader.h"

using namespace std;
using namespace transport_catalogue;
using namespace transport_catalogue::json_reader;

int main() {
    ifstream in("input.json");
    ofstream out("output.json");
    TransportCatalogue catalogue;
    JsonReader reader(in);
    reader.SendRequests(catalogue);
    reader.GetResponses(out);
}