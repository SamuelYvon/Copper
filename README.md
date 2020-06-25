# Copper

Copper is a (somewhat) fast implementation of a Cop Number computational algorithm. 


## Compilation and Usage

If you don't want to fiddle with `clang`, the Clion tool should pick everything up and get going immediately.
Otherwise, you can compile from sources using:

```bash
> cmake -DCMAKE_BUILD_TYPE=Release CmakeLists.txt
> make Copper
```

And you will have a `Copper` executable.

### Usage

> ./Copper -h 

... will tell you all you need to know.

## Acknowledgements

The core algorithm is based off _algorithm 2_ presented in

```
@article{bonato2010cops,
  title={Cops and robbers from a distance},
  author={Bonato, Anthony and Chiniforooshan, Ehsan and Pra{\l}at, Pawe{\l}},
  journal={Theoretical Computer Science},
  volume={411},
  number={43},
  pages={3834--3844},
  year={2010},
  publisher={Elsevier}
}
```

The implementation of the .g6 file parser is based off the g6 file format, found at the [the author's website](https://users.cecs.anu.edu.au/~bdm/data/formats.txt).

Furthermore, since the format is somewhat strange, the implementation of the g6 reader has been greatly influenced by the 
implementation of the reader from the `networkX` packaged, found [here](https://github.com/networkx/networkx/blob/master/networkx/readwrite/graph6.py).
