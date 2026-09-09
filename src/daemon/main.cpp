#include "app/app.h"

#include <exception>

using namespace mdk::app;

int main()
{
    try
    {
        App::GetInstance().Run();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception occurred {}", e.what());
        return 1;
    }

    return 0;
}
