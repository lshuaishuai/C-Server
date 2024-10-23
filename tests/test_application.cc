#include "server/application.h"

int main(int argc, char** argv)
{
    shuai::Application app;
    if(app.init(argc, argv)) {
        return app.run();
    }
    return 0;
}