#include "demon.hpp"

int main(int argc, char* argv[]) {
    Demon& demon = Demon::get_instance();
    demon.start();
    return 0;
}
