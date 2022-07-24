
#include "star/rpc/route_strategy.h"

#include "star/log.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();
std::vector<int> list{1, 2, 3, 4, 5};

void test_random() {
    star::rpc::RouteStrategy<int>::ptr strategy =
            star::rpc::RouteEngine<int>::queryStrategy(star::rpc::Strategy::Random);

    STAR_LOG_DEBUG(g_logger) << "random";
    for ([[maybe_unused]] auto i: list) {
        auto a = strategy->select(list);
        STAR_LOG_DEBUG(g_logger) << a;
    }
}

void test_poll() {
    star::rpc::RouteStrategy<int>::ptr strategy =
            star::rpc::RouteEngine<int>::queryStrategy(star::rpc::Strategy::Polling);

    STAR_LOG_DEBUG(g_logger) << "Poll";
    for ([[maybe_unused]] auto i: list) {
        auto a = strategy->select(list);
        STAR_LOG_DEBUG(g_logger) << a;
    }
}
void test_hash() {
    star::rpc::RouteStrategy<int>::ptr strategy =
            star::rpc::RouteEngine<int>::queryStrategy(star::rpc::Strategy::HashIP);

    STAR_LOG_DEBUG(g_logger) << "Hash";
    for ([[maybe_unused]] auto i: list) {
        auto a = strategy->select(list);
        STAR_LOG_DEBUG(g_logger) << a;
    }
}
int main() {
    test_random();
    test_poll();
    test_hash();
}