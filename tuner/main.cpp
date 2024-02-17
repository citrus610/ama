#include "tuner.h"
#include "score.h"

int main()
{
    srand(time(NULL));

    i32 user;

    printf("Choose an action:\n");
    printf("[0] - Train default weights\n");
    printf("[1] - Train custom weights\n");
    // printf("[2] - Print value\n");

    std::cin >> user;

    if (user == 0) {
        Tuner::run(Eval::DEFAULT);
    }
    else if (user == 1) {
        auto w = Eval::Weight();
        std::ifstream f("config.json");
        if (!f.good()) {
            printf("Can't find \"config.json\"!\n");
            return -1;
        };
        json js;
        f >> js;
        f.close();
        from_json(js, w);

        Tuner::run(w);
    }
    // else if (user == 2) {
    //     i32 idx = 0;

    //     while (true)
    //     {
    //         std::ifstream f("config.json");
    //         if (f.good()) {
    //             printf("Can't find \"config.json\"!\n");
    //             return -1;
    //         };
    //     }
    // }

    return 0;
};