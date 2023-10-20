#include "tuner.h"
#include "score.h"

int main()
{
    srand(time(NULL));

    Tuner::run(Eval::DEFAULT);

    // auto w1 = Eval::DEFAULT;

    // auto w2 = Eval::DEFAULT;
    // w2.chain = 511;
    // w2.chain_y = 10;
    // w2.distribution = -5;
    // w2.extensibility = 26;
    // w2.leftover = 0;
    // w2.link_h = 8;
    // w2.link_v = -3;
    // w2.shape = -45;
    // w2.tear = -28;
    // w2.waste = -130;

    // std::atomic<i32> count1;
    // std::atomic<i32> count2;

    // std::atomic<i32> score1;
    // std::atomic<i32> score2;

    // std::vector<Cell::Pair> queues[100];
    // for (i32 i = 0; i < 100; ++i) {
    //     queues[i] = create_queue();
    // }

    // std::vector<std::thread> threads;

    // for (i32 t = 0; t < 4; ++t) {
    //     threads.emplace_back([&] (i32 tid) {
    //         for (i32 i = 0; i < 25; ++i) {
    //             auto queue = queues[tid * 25 + i];

    //             auto sim1 = simulate(w1, queue);
    //             auto sim2 = simulate(w2, queue);

    //             if (sim1 >= 78000) {
    //                 count1 += 1;
    //                 score1 += sim1;
    //             }

    //             if (sim2 >= 78000) {
    //                 count2 += 1;
    //                 score2 += sim2;
    //             }
    //         }
    //     }, t);
    // }

    // for (auto& thread : threads) {
    //     thread.join();
    // }

    // Tuner::print_w(w1);
    // printf("count %d\n", count1.load());
    // printf("score %d\n", score1.load());
    // printf("arvg  %d\n\n", score1.load() / count1.load());

    // Tuner::print_w(w2);
    // printf("count %d\n", count2.load());
    // printf("score %d\n", score2.load());
    // printf("arvg  %d\n\n", score2.load() / count2.load());

    return 0;
};