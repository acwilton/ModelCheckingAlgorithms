# ModelCheckingAlgorithms
My implementation of a basic model checking library in C++ using the algorithms and concepts from the Model Checking book. Particularly Chapter 7 of the book.

Algorithms implemented:
 + Buchi intersection of two Buchi automata
 + Double DFS to find accepting run of a Buchi
 + Kripke to Buchi algorithm
 + LTL normalization algorithm (converts to negation normal form and replaces future and global subformulas to their equivalent untile and release subformulas)
 + LTL to Buchi algorithm
 + A model checking algorithm that combines all of the above to determine if there exists a run on a Kripke structure that satisfies an LTL formula.
