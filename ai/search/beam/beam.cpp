#include "beam.h"

namespace beam
{

// Expands node
void expand(
    const cell::Pair& pair,
    node::Data& node,
    const eval::Weight& w,
    std::function<void(node::Data&, const move::Placement&, const chain::Score&)> callback
)
{
    // Generates moves
    auto locks = move::generate(node.field, pair.first == pair.second);

    for (auto i = 0; i < locks.get_size(); ++i) {
        // Creates child node
        auto child = node;
        
        // Drops pair
        child.field.drop_pair(locks[i].x, locks[i].r, pair);

        // Pops child field
        auto pop = child.field.pop();

        // Checks for death
        if (child.field.get_height(2) > 11) {
            continue;
        }

        // Evaluates action
        i32 tear = node.field.get_drop_pair_frame(locks[i].x, locks[i].r) - 1;
        i32 waste = pop.get_size();

        eval::action(child, tear, waste, w);

        // Callback
        callback(child, locks[i], chain::get_score(pop));
    }
};

// Does 1 iteration of beam search from the parents layer to the children layer
void think(
    const cell::Pair& pair,
    std::vector<Candidate>& candidates,
    Layer& parents,
    Layer& children,
    const eval::Weight& w
)
{
    // Sorts the parents layer
    parents.sort();

    // Expands each parent to the next layer
    for (auto& node : parents.data) {
        beam::expand(pair, node, w, [&] (node::Data& child, const move::Placement& placement, const chain::Score& chain) {
            // Updates max chain score found
            candidates[child.index].score = std::max(candidates[child.index].score, size_t(chain.score));

            // Prunes children that triggered big chains
            if (chain.score >= beam::PRUNE) {
                return;
            }

            // Get the node's hash
            auto hash = node::get_hash(child);

            // Probes the transposition table
            auto [found, entry] = children.table.get(hash);

            // If the node's position had already been reached before
            if (found) {
                // Transposition table cut
                // If the new node's action cost is higher, we don't push or update the tranposition table
                if (child.score.action < entry->action) {
                    return;
                }

                // Gets entry's evaluation value
                child.score.eval = entry->eval;
            }
            // Gets node static evaluation if failed
            else {
                eval::evaluate(child, w);
            }

            // Stores the entry in the transposition table
            children.table.set(entry, hash, child.score.action, child.score.eval);

            // Adds children to the next layer
            children.add(child);
        });
    }

    // Clears the parents layer
    parents.clear();
};

// Beam search
Result search(
    Field field,
    cell::Queue queue,
    eval::Weight w,
    Configs configs
)
{
    auto result = Result();

    // Checks queue
    if (queue.size() < 2) {
        return result;
    }

    // Creates root
    auto root = node::Data {
        .field = field,
        .score = { 0, 0 },
        .index = -1
    };

    // Creates stack
    std::array<Layer, 2> layers = {
        Layer(configs.width),
        Layer(configs.width)
    };

    // Initializes candidates
    beam::expand(
        queue[0],
        root,
        w,
        [&] (node::Data& child, const move::Placement& placement, const chain::Score& chain) {
            // Creates candidate
            auto candidate = beam::Candidate();

            candidate.placement = placement;
            candidate.score = chain.score;

            // Updates child
            child.index = i32(result.candidates.size());
            eval::evaluate(child, w);

            // Pushes
            result.candidates.push_back(candidate);
            layers[0].data.push_back(child);
        }
    );

    // If there aren't any candidates, stops searching
    if (result.candidates.empty()) {
        return result;
    }

    // Searches
    for (size_t i = 0; i < queue.size() - 1; ++i) {
        beam::think(
            queue[i + 1],
            result.candidates,
            layers[i & 1],
            layers[(i + 1) & 1],
            w
        );

        bool enough = false;

        for (auto& c : result.candidates) {
            if (c.score >= configs.trigger) {
                enough = true;
                break;
            }
        }

        if (enough) {
            break;
        }
    }

    return result;
};

// Beam search but with multiple threads and queues
// According to takapt's paper (https://www.slideshare.net/slideshow/ai-52214222/52214222) but with modifications and upgrades
//
// 1. Searching past the given queue
// Their puyo AI got stronger by using multiple random queues
// The more queues the better the search results
// However, testing shows that you can use these 6 specific queues to achieve the same (or even better) results as using 50 different random queues
// - R, Y, G, B, ...loop
// - R, G, Y, B, ...loop
// - R, B, Y, G, ...loop
// - Y, G, R, B, ...loop
// - Y, B, R, G, ...loop
// - G, B, R, Y, ...loop
//
// One can notice that except when building at the very top (the 14th row), we don't really care about the colors order of a puyo pair
// For example, at low heights, move generation for a RED-BLUE pair is the same as a BLUE-RED pair
// The above 6 queues contain all types of puyo pairs without caring about the pairs' colors order or same-color pairs
// We've tested queues with same-color pairs but the results were worse
// We theorize that same-color pairs may make the AI overestimate its chains probability
//
// 2. Transposition table
// Instead of using std::unordered_map, we use a special transposition table
//
// An entry of the table is 8 bytes:
// - hash key (2 bytes)
// - age (2 bytes)
// - action value (2 bytes)
// - evaluation value (2 bytes)
// Each bucket contains 4 entries and aligns on 32 bytes
// Each layer has a 256 KB table
// 
// Our replacement scheme is value prefered with aging, taking inspiration from how chess engine does it
// Every time we store an entry into the transposition table, we can store the table's age into the entry
// We increase the table's age every iteration of beam search
// This way, we don't have to clear the table every iteration, which is very performance costly
// We can check the age of the entry found with the table's current age to see if this entry is valid or not
// The older an entry is, the more likely it will be replaced
//
// Check `table.h` and `table.cpp` for more info
//
// 3. Selection policy
// In takapt's original algorithm, for each queue they only return the biggest chain found
// We improve upon this by returning all the candidates with their respective biggest chains found in each queue:
//
// Ex:
// - Thread #1 result:
//    +---------------+---------------+---------------+     +---------------+
//    |  placement 1  |  placement 2  |  placement 3  | ... |  placement n  |
//    +---------------+---------------+---------------+     +---------------+
//    |   max chain   |   max chain   |   max chain   |     |   max chain   |
//    |     80000     |     1000      |    100000     |     |      500      |
//    +---------------+---------------+---------------+     +---------------+
// - Thread #2 result:
//    +---------------+---------------+---------------+     +---------------+
//    |  placement 1  |  placement 2  |  placement 3  | ... |  placement n  |
//    +---------------+---------------+---------------+     +---------------+
//    |   max chain   |   max chain   |   max chain   |     |   max chain   |
//    |     95000     |     70000     |     70000     |     |     5000      |
//    +---------------+---------------+---------------+     +---------------+
// - Thread #3 result:
//    +---------------+---------------+---------------+     +---------------+
//    |  placement 1  |  placement 2  |  placement 3  | ... |  placement n  |
//    +---------------+---------------+---------------+     +---------------+
//    |   max chain   |   max chain   |   max chain   |     |   max chain   |
//    |     90000     |     20000     |     90000     |     |     10000     |
//    +---------------+---------------+---------------+     +---------------+
//
// - Accumulated result:
//    +---------------+---------------+---------------+     +---------------+
//    |  placement 1  |  placement 2  |  placement 3  | ... |  placement n  |
//    +---------------+---------------+---------------+     +---------------+
//    |  total chain  |  total chain  |  total chain  |     |  total chain  |
//    |    265000     |     91000     |    260000     |     |     15500     |
//    +---------------+---------------+---------------+     +---------------+
//          ^^^^^
//     The best result
// 
// For each placement, we accumulate it's max chains found from every threads and choose the placement with the highest total chain score
// This is the same as choosing the placement with the highest expected chain score
// We can get the expected chain score by dividing the total chain by the thread count
//
// 4. Search constants
// Instead of searching until we fill the field, we only search to depth 16 with the beam width of 250
// Even searching only to depth 12 yield great results, so I don't see the appeal of searching deeper
// Experimenting with bigger beam width will probably improve the AI, but a width of 250 seems to be good enough for now
Result search_multi(
    Field field,
    cell::Queue queue,
    eval::Weight w,
    Configs configs
)
{
    auto result = Result();

    // Creates future queues
    std::vector<cell::Queue> queues;

    for (auto i = 0; i < beam::BRANCH; ++i) {
        auto q = queue;
        auto qrng = beam::get_queue_random(i, configs.depth - queue.size());

        q.insert(q.end(), qrng.begin(), qrng.end());

        queues.push_back(q);
    }

    // Searching multiple queues at the same time
    std::vector<std::thread> threads;
    std::mutex mtx;
    
    for (auto i = 0; i < beam::BRANCH; ++i) {
        threads.emplace_back([&] (i32 id) {
            // Beam search for 1 queue
            auto b = beam::search(field, queues[id], w, configs);

            if (b.candidates.empty()) {
                return;
            }

            std::lock_guard<std::mutex> lk(mtx);

            // If this is the first finished search
            if (result.candidates.empty()) {
                result = b;
                return;
            }

            // Accumulates the biggest chain scores of each candidate
            for (auto& c1 : result.candidates) {
                for (auto& c2 : b.candidates) {
                    if (c1.placement == c2.placement) {
                        c1.score += c2.score;
                        break;
                    }
                }
            }
        }, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Sorts candidates by their total accumulated scores
    if (!result.candidates.empty()) {
        std::sort(
            result.candidates.begin(),
            result.candidates.end(),
            [&] (const beam::Candidate& a, const beam::Candidate& b) {
                if (configs.stretch) {
                    return a.score > b.score;
                }

                bool a_enough = a.score / beam::BRANCH >= configs.trigger;
                bool b_enough = b.score / beam::BRANCH >= configs.trigger;

                if (a_enough && b_enough) {
                    return a.score < b.score;
                }

                return a.score > b.score;
            }
        );
    }

    return result;
};

// Gets future queues randomly (real 100% no clickbait)
cell::Queue get_queue_random(i32 id, size_t count)
{
    cell::Queue result;

    u8 bag[6][4] = {
        { 0, 1, 2, 3 },
        { 0, 2, 1, 3 },
        { 0, 3, 1, 2 },
        { 1, 2, 0, 3 },
        { 1, 3, 0, 2 },
        { 2, 3, 0, 1 }
    };

    while (result.size() < count)
    {
        result.push_back(cell::Pair {
            cell::Type(bag[id][0]),
            cell::Type(bag[id][1])
        });

        result.push_back(cell::Pair {
            cell::Type(bag[id][2]),
            cell::Type(bag[id][3])
        });

        if (result.size() >= count) {
            break;
        }
    }

    return result;
};

};