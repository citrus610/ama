<div align="center">

  <h1>Ama</h1>
  <br>
  Strongest Puyo Puyo Tsu AI
  <br>
  <br>
    <a href="https://ko-fi.com/citrus610">
      <img
        src="https://img.shields.io/badge/Ko--fi-Support%20me%20on%20Ko--fi-FF5E5B?logo=kofi&logoColor=white"
        alt="ko-fi"
        height="24em"
      >
    </a>
  <br>

</div>

## Overview
Ama is an AI created to play Puyo Puyo Tsu 1P and PVP. This project aims to become the strongest Puyo Puyo entity. Currently, Ama can run on Puyo Puyo Champions Steam version.

## Features
- Field representation
  - Bitfield
  - SIMD for fast chain simulation
- Search
  - Best first search
  - Beam search
    - Parallelized search
    - Monte Carlo inspired sampling method with predetermined queues
    - Highest expected chain score selection policy
  - Quiescence search
  - Transposition table
    - Value-preferred with aging replacement scheme
- Evaluation
  - Pattern matching
    - GTR
    - Sullen GTR
    - Fron
  - Chain detection
  - Chain extension
  - Trigger height
  - Field shape
  - Avoid tearing
  - Avoid wasting resources
- Enemy reading
  - State machine
  - Build action
    - Big chain (78% of chains >= 100,000)
    - Fast second chain
    - All Clear battle
    - Imbalance resources
    - Countering
    - Harassment
  - Attack action
    - Negamax inspired search that can look 3 moves ahead
    - Crush
    - Combo
    - Kill
  - Defense action
    - Negamax inspired search that can look 2 moves ahead
    - Correspondence return
    - Synchronized attack
    - Accepting/Countering
    - Desparate return

## How to build
For now, this projects can only be compiled using `g++` that supports `c++ 20`. Make sure that your cpu support `sse4` and `pext`.
- Clone and `cd` to the repository.
- Run `make PEXT=true puyop` to build the puyop client.
- Get the binary in `bin`.

NOTE: The source code for the `Puyo Puyo Champions Steam` isn't available to prevent cheating

## Acknowledgement
- Thanks K. Ikeda, D. Tomizawa, S. Viennot and Y. Tanaka for their paper `Playing PuyoPuyo: Two search algorithms for constructing chain and tactical heuristics`. Ama's early search algorithm was heavily influenced by their work.
- Thanks [puyoai](https://github.com/puyoai/puyoai) for the fast implementation of bitfield and the inspiration for the evaluation function.
- Thanks [takapt](https://www.slideshare.net/slideshow/ai-52214222/52214222) for their beam search idea, Ama's new improved beam search was based on their implemntation.
- Thanks [nlohmann](https://github.com/nlohmann/json) for the c++ json library.
- Thanks [nicoshev](https://github.com/Nicoshev/rapidhash) for their `rapidhash` hash function.

## License
This project is licensed under [MIT LICENSE](LICENSE).
