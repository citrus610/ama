#include "tuner.h"
#include "score.h"

int main()
{
    srand(time(NULL));

    i32 user;

    printf("Choose an action:\n");
    printf("[0] - Train default weights\n");
    printf("[1] - Train custom weights\n");
    printf("[2] - Print value\n");

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
    else if (user == 2) {
        i32 idx = 0;

        std::string out_str;
        std::string out_id_str;

        while (true)
        {
            std::string id = std::to_string(idx);
            std::string fname = std::string("data/") + id + std::string(".json");

            std::ifstream f(fname);
            if (!f.good()) {
                break;
            };

            Tuner::SaveData s;
            Tuner::load(id, s);

            for (i32 i = 0; i <= s.unchange; ++i) {
                out_str += std::to_string(s.count) + "\n";
                out_id_str += id + "\n";
            }

            idx += 1;
        }

        std::ofstream o("out.txt");
        o << out_str << std::endl;
        o << out_id_str << std::endl;
        o.close();
    }

    return 0;
};