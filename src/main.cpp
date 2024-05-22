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
    CERR("fileremove TrainsState.db %d\n", std::remove("TrainsState.db"));
    CERR("fileremove TrainUnitMap.db %d\n", std::remove("TrainUnitMap.db"));
    CERR("fileremove SeatsData.dat %d\n", std::remove("SeatsData.dat"));
    CERR("fileremove StationMap.db %d\n", std::remove("StationMap.db"));
    CERR("fileremove OrdersData.dat %d\n", std::remove("OrdersData.dat"));
    CERR("fileremove TrainIDArray.vec %d\n", std::remove("TrainIDArray.vec"));
    CERR("fileremove UserOrders.db %d\n", std::remove("UserOrders.db"));
    CERR("fileremove Users.map %d\n", std::remove("Users.map"));
}


int main() {
    sjtu::Timer main_timer("main");
    main_timer.start();
    while (Run()) ClearFile();
    main_timer.stop();
    return 0;
}