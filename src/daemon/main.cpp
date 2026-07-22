#include "app/app.h"
#include "server/server.h"

using namespace mdk::app;
using namespace mdk::server;

int main()
{
    try
    {
        Server server;
        server.Run();

        App::GetInstance().Run();
    }
    catch (std::exception e)
    {
        LOG_ERROR("Exception occured {}", e.what());
    }
}