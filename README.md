# Ama
Ama is an AI that was created to play Puyo Puyo Tsu 1P and PVP. This project aims to reach the playing strength of professional Puyo Puyo players. Ama is currently ranked 3400 ~ 3500 on Puyo Puyo Champions Steam edition.

## Overview
- Efficient bitfield implementation inspired by [puyoai](https://github.com/puyoai/puyoai)
- Uses GTR as the default chain form. To disable GTR, change the `form` parameter in `config.json` to 0.
- Implements the ability to gaze the opponent's field:
  - Can harass the opponent at the right time.
  - Can or try to return the attacks from the opponent.
- Implements high recovery mode that can build chain very fast.
- Decent chain building ablility.

## How to build
For now, this projects can only be compiled using `g++` that supports `c++ 20`. Make sure that your cpu support `sse4` and `pext`.
- Clone and `cd` to the repository.
- Run `make PEXT=true puyop` to build the puyop client.
- Get the binary in `bin`.

## Acknowledgement
- Thanks [puyoai](https://github.com/puyoai/puyoai) for the fast implementation of bitfield and the inspiration for the evaluation function.
- Thanks [nlohmann](https://github.com/nlohmann/json) for the c++ json library.

## License
This project is licensed under [MIT LICENSE](LICENSE).
