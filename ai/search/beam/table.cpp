#include "table.h"

Table::Table()
{
    this->buckets = nullptr;
    this->count = 0;
    this->age = 0;
};

// Resizes or initialize the transposition table
// Allocates memory with alignment for fast fetching
void Table::resize(u64 kb)
{
    const u64 KB = 1ULL << 10;

    // Free memory if there is any
    if (this->buckets != nullptr) {
        free_aligned(this->buckets);
    }
    
    // Allocates memory
    this->buckets = static_cast<Bucket*>(malloc_aligned(KB, kb * KB));

    // Sets count
    this->count = kb * KB / sizeof(Bucket);

    // Clears
    this->clear();
};

// Clears the table
void Table::clear()
{
    memset(this->buckets, 0, this->count * sizeof(Bucket));
};

// Updates the table age after each iteration of search
void Table::update()
{
    this->age += 1;
};

// Gets the bucket index of a position
u64 Table::get_index(u64 hash)
{
    return static_cast<u64>((static_cast<u128>(hash) * static_cast<u128>(this->count)) >> 64);
};

// Probes the table and gets the entry if possible
std::pair<bool, Entry*> Table::get(u64 hash)
{
    auto entries = this->buckets[this->get_index(hash)].entries;

    // Finds matching entry
    for (usize i = 0; i < 4; ++i) {
        if (entries[i].hash == static_cast<u16>(hash)) {
            return { true, &entries[i] };
        }
    }

    // Finds replacement
    auto entry = &entries[0];

    for (usize i = 1; i < 4; ++i) {
        if ((entries[i].age == entry->age && i32(entries[i].action) + i32(entries[i].eval) < i32(entry->action) + i32(entry->eval)) ||
            (entries[i].age < entry->age)) {
            entry = &entries[i];
        }
    }

    return { false, entry };
};

// Stores the new entry values
void Table::set(Entry* entry, u64 hash, i32 action, i32 eval)
{
    if (entry->hash != static_cast<u16>(hash) ||
        entry->age != this->age ||
        i32(entry->action) + i32(entry->eval) < action + eval) {
        entry->hash = static_cast<u16>(hash);
        entry->age = this->age;
        entry->action = action;
        entry->eval = std::clamp(eval, INT16_MIN, INT16_MAX);
    }
};