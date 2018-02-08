# :stars: hoshizora: Fast graph analysis engine
Tutorial is [here](https://github.com/hoshizora-project/tutorial).
You can try *hoshizora* and [amanogawa](https://github.com/hoshizora-project/amanogawa) on Jupyter on Docker

(**:warning: Currently alpha version. Inner structure and APIs might be change a lot**)


## :sparkles: Features
* Easy to use
  * You can use *hoshizora* as a Python library, C++ library and CLI tool
* Extremely fast
  * Full native speed
  * Efficient parallel processing (Can scale to over one hundred cores)

Experimental optimizations are [here](https://github.com/amaya382/hoshizora/tree/experimental).
Note that experimental branch is unstable.


## :soon: Install
Supporting Linux and macOS

### Python library via pip
```sh
pip install hoshizora
```

### From source
Prerequisites
* Make
* CMake 3.0+ 
* Clang++ 3.4+
* Python 3
* \[OPT\] libnuma

#### CLI
```sh
git submodule init && git submodule update
make release
```

#### Python library
```sh
python3 setup.py install
```


## :bulb: Example
### Task: PageRank
#### Python
```python
import hoshizora as hz
result = hz.pagerank(graph_file, num_iters)
```

#### CLI
```sh
./hoshizora-cli ${graph_file} ${num_iters} > result
```


## :persevere: WIP
* [ ] Querying API
* [ ] Support dynamic graph
* [ ] APIs for Graph compaction
* [ ] Out-of-Core processing
* [ ] Tests
* [ ] Many many applications