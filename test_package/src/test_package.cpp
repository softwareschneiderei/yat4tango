#include <iostream>
#include <yat4tango/MonitoredDevice.h>

int main(int argc, char* argv[]) {
    const std::string tt_dev_name("sys/tg_test/1");
    yat4tango::MonitoredDevice md(tt_dev_name);
    std::cout << "instanciated MonitoredDevice" << std::endl; 
    return 0;
}
