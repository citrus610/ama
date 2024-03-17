# Ama
<a href="https://ko-fi.com/citrus610">
    <img
        src="https://img.shields.io/badge/Ko--fi-Support%20me%20on%20Ko--fi-FF5E5B?logo=kofi&logoColor=white"
        alt="ko-fi"
        height="24em"
    >
</a>

A strong Puyo Puyo AI.

## Overview
Ama is an AI created to play Puyo Puyo Tsu 1P and PVP. This project aims to reach the playing strength of professional Puyo Puyo players. Currently, Ama is rated 3648 on Puyo Puyo Champions Steam version.
<p align="center">
    <a href="https://www.youtube.com/watch?v=LQiWRFNRknk">
        <img src="https://img.youtube.com/vi/LQiWRFNRknk/0.jpg">
        <br />
        YouTube - ama AI vs のらすけ [ぷよぷよAI]
    </a>
</p>

## Features
- Efficient bitfield implementation inspired by [puyoai](https://github.com/puyoai/puyoai)
- Uses human-like forms as the default chain forms, such as GTR, Meri, etc. To disable human-like chains, set the `form` parameter in `config.json` to 0.
- Implements the ability to gaze the opponent's field:
  - Can harass the opponent at the right time.
  - Can return the attacks from the opponent.
- Implements high recovery mode that can build chain very fast.
- Decent chain building ablility.

## How to build
For now, this projects can only be compiled using `g++` that supports `c++ 20`. Make sure that your cpu support `sse4` and `pext`.
- Clone and `cd` to the repository.
- Run `make PEXT=true puyop` to build the puyop client.
- Get the binary in `bin`.

## Acknowledgement
- Thanks K. Ikeda, D. Tomizawa, S. Viennot and Y. Tanaka for their paper "Playing PuyoPuyo: Two search algorithms for constructing chain and tactical heuristics." Ama's search algorithm was heavily influenced by their work.
- Thanks [puyoai](https://github.com/puyoai/puyoai) for the fast implementation of bitfield and the inspiration for the evaluation function.
- Thanks [nlohmann](https://github.com/nlohmann/json) for the c++ json library.

## License
This project is licensed under [MIT LICENSE](LICENSE).
