# ModelCheckingAlgorithms
My implementation of a basic model checking library in C++17 using the algorithms and concepts from the Model Checking book. Particularly Chapter 7 of the book.

Algorithms implemented:
 + Buchi intersection of two Buchi automata
 + Double DFS to find accepting run of a Buchi
 + Fair Kripke to Buchi algorithm
 + LTL normalization algorithm (converts to negation normal form and replaces future and global subformulas to their equivalent untile and release subformulas)
 + LTL to Buchi algorithm
 + A model checking algorithm that combines all of the above to determine if there exists a run on a Kripke structure that satisfies an LTL formula.

Currently there are some runtime issues with the model checking algorithm but the subset of the library which involves taking the intersection of two Buchi's and find an accepting run is working. I wrote a driver that can be ran that will parse a file that specifies two buchi automata and will then take their intersection and try to find an accepting run. If one is found, it will print the lasso it forms.

To build the driver simply type `make` under the `src/` folder. A file called `driver` will be generated which can be run with a filename as input like so: `./driver test1.buchi`. The filename should be of a file that holds the specification of two Buchi automaton. 

The specification of a Buchi automaton looks as follows:
```
{
  <init1, init2, ..., initN>
  [acc1, acc2, ..., accM]
  (a1, l1, b1)
  (a2, l2, b2)
  ...
  (aK, lK, bK)
}
```
The dots `...` are not part of the syntax. They are just there to demonstrate a variable number of things.
The states and labels of the Buchi automata specified are strings. Two states are the same if they are the same string (ignoring trailing and leading whitespace). The same goes for labels.
The angle brackets `<init1, init2, ..., initN>` holds the list of starting states.
The square brackets `[acc1, acc2, ..., accM]` holds the list of accepting states.
A triple `(ai, li, bi)` represents a labeled transition from node `ai` to node `bi` with the transition labeled by the string `li`. The states of the system are implicitly defined by the appearance of a state in some transition, or in the list of starting states or the list of accepting states.

For the driver to accept a buchi specification file, it must have exactly two buchi structures as written above (whitespacing differences are ignored).

To help get started playing with this tool, there is an example file you can run the driver on called `test1.buchi`.
