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
This is ama's new beam search implementation with comments.
I just want to publish this to share the results of my research over the past year.
Currently, ama can build a 100,000-point chain or bigger 78% of the time.
Hopefully, this will be a useful learning resource for future Puyo AI developers.

これはamaの新しいビームサーチの実装とコメントである。
現在、amaは78％の確率で10万点本線を構築できる。
将来のぷよAI開発者にとって有益な学習リソースになることを願っています。

## How to build
This projects can only be compiled using `g++` that supports `c++ 20`. Make sure that your cpu support `sse4` and `pext`.
- Clone and `cd` to the repository.
- Run `make PEXT=true puyop` to build the puyop client.
- Get the binary in `bin`.
- Run the `puyop` client to see ama build chains on [puyop](https://www.puyop.com).

## Files
- [README](README.md) the file you are currently reading.
- [ai](ai) the main AI implementation directory.
    - [search](ai/search) collection of ama's search algorithms.
        - [beam](ai/search/beam) the main beam search implementation.
        - [dfs](ai/search/dfs) ama's old search implementation, you can ignore this.
    - [ai.h](ai/ai.h) just a header file, nothing much here.
    - [path.h](ai/path.h) ama's pathfinder implementation, check this if you want to see how ama find paths with movement cancel in Puyo Puyo Champions.
- [core](core) puyo puyo core game implementation, including fast `bitfield` implementation using `simd` etc.
- [lib](lib) external 3rd library used.
- [puyop](puyop) implementation of `puyop` client, check this if you want to see how to use ama's beam search in your project.
- [test](test) ama's chains tester.
- [tuner](tuner) ama's rough tuner implementation that is really good at improving bad weights fast but very bad at fine-tuning good weights.
- [config.json](config.json) ama's search weights.

## Acknowledgement
- Thanks takapt for their [implementation of beam search](https://www.slideshare.net/slideshow/ai-52214222/52214222), the new ama's search was heavily inspired by this.
- Thanks [puyoai](https://github.com/puyoai/puyoai) for the fast implementation of bitfield and the inspiration for the evaluation function.
- Thanks [nlohmann](https://github.com/nlohmann/json) for the c++ json library.
- Thanks [nicoshev](https://github.com/Nicoshev/rapidhash) for their `rapidhash` hash function.

## License
This project is licensed under [MIT LICENSE](LICENSE).
