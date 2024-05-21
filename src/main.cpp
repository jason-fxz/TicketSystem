#include "src/TicketSystem.hpp"

bool Run() {
    sjtu::TicketSystem luowen;
    int r = 0;
    while ((r = luowen.NextCMD()) == 0) ;
    return r == 2; // for clean
}

void ClearFile() {
    CERR("Clearing files...\n");
    CERR("fileremove TrainsData.dat %d\n", std::remove("TrainsData.dat"));
    CERR("fileremove Users.db %d\n", std::remove("Users.db"));
    CERR("fileremove TrainsState.db %d\n", std::remove("TrainsState.db"));
    CERR("fileremove SeatsData.dat %d\n", std::remove("SeatsData.dat"));
    CERR("fileremove StationMap.db %d\n", std::remove("StationMap.db"));
    CERR("fileremove OrdersData.dat %d\n", std::remove("OrdersData.dat"));
    CERR("fileremove TrainUnitMap.db %d\n", std::remove("TrainUnitMap.db"));
    CERR("fileremove UserOrders.db %d\n", std::remove("UserOrders.db"));
}


int main() {
    while (Run()) ClearFile();
    
    return 0;
}