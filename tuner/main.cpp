#include "tuner.h"

int main()
{
    srand(time(NULL));

    printf("Choose an action:\n");
    printf("[0] - Train\n");
    printf("[1] - Print\n");

    i32 user;
    std::cin >> user;

    if (user == 0) {
        auto w = beam::eval::Weight();

        std::ifstream f("config.json");

        if (!f.good()) {
            printf("Can't find \"config.json\"!\n");
            return -1;
        };

        json js;
        f >> js;
        f.close();

        from_json(js, w);

        tuner::run(w);
    }
    else if (user == 1) {
        i32 idx = 0;

        std::string str_s0;
        std::string str_s1;
        std::string str_s2;
        std::string str_id;

        while (true)
        {
            std::string file = std::string("data/") + std::to_string(idx) + std::string(".json");

            std::ifstream f(file);

            if (!f.good()) {
                break;
            };

            auto save = tuner::load(idx);

            str_s0 += std::to_string(double(save.score[0].chain.score) / double(save.score[0].frame)) + "\n";
            str_s1 += std::to_string(save.score[0].chain.score) + "\n";
            str_s2 += std::to_string(save.score[0].frame) + "\n";
            str_id += std::to_string(idx) + "\n";

            idx += 1;
        }

        std::ofstream o("out.txt");

        o << str_s0 << "_" << std::endl;
        o << str_s1 << "_" << std::endl;
        o << str_s2 << "_" << std::endl;
        o << str_id << "_" << std::endl;

        o.close();
    }

    return 0;
};